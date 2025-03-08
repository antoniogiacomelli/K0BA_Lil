/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module           : Application Timers
 * 	Depends on       : Scheduler, Inter-task Synchronisation
 *   Public API 		 : Yes
 * 	In this unit:
 * 			o Timer Pool Management
 *			o Timer Delta List
 *			o Timer Handler
 *			o Sleep delay
 *			o Busy-wait delay
 *			o Time-out for blocking mechanisms
 *
 *****************************************************************************/

#define K_CODE

#include "kexecutive.h"

/******************************************************************************
 * GLOBAL TICK RETURN
 *****************************************************************************/
TICK kTickGet( void)
{
    return (runTime.globalTick);
}

/******************************************************************************
 * BUSY-DELAY
 *****************************************************************************/
VOID kBusyDelay( TICK delay)
{
    if (runPtr->busyWaitTime == 0)
    {
        runPtr->busyWaitTime = delay;
    }
    while (runPtr->busyWaitTime)
        ;/* procrastinating here */
    return;
}

#if (K_DEF_CALLOUT_TIMER==ON)
/******************************************************************************
 * CALLOUT TIMERS
 *****************************************************************************/
K_TIMER *currTimerPtr = NULL;

static inline VOID kTimerListAdd_( K_TIMER *kobj, TICK phase, TICK duration,
        CALLOUT funPtr, ADDR argsPtr, BOOL reload)
{
    kobj->timeoutNode.dtick = duration;
    kobj->timeoutNode.timeout = duration;
    kobj->timeoutNode.objectType = TIMER;
    kobj->funPtr = funPtr;
    kobj->argsPtr = argsPtr;
    kobj->reload = reload;
    kobj->phase = phase;
    kTimeOut( &kobj->timeoutNode, duration);
}

K_ERR kTimerInit( K_TIMER *kobj, TICK phase, TICK duration, CALLOUT funPtr,
        ADDR argsPtr, BOOL reload)
{
    if ((kobj == NULL) || (funPtr == NULL))
    {
        return (K_ERR_OBJ_NULL);
    }
    K_CR_AREA
    K_CR_ENTER
    kTimerListAdd_( kobj, phase, duration, funPtr, argsPtr, reload);
    K_CR_EXIT
    return (K_SUCCESS);
}
VOID kRemoveTimerNode( K_TIMEOUT_NODE *node)
{
    if (node == NULL)
        return;

    if (node->nextPtr != NULL)
    {
        node->nextPtr->dtick += node->dtick;
        node->nextPtr->prevPtr = node->prevPtr;
    }

    if (node->prevPtr != NULL)
    {
        node->prevPtr->nextPtr = node->nextPtr;
    }
    else
    {
        timerListHeadPtr = node->nextPtr;
    }

    node->nextPtr = NULL;
    node->prevPtr = NULL;
}
#endif

/* some marvin gaye, some luther vandross, some lil' anita... */
/*******************************************************************************
 * SLEEP TIMER AND BLOCKING TIME-OUT
 *******************************************************************************/
void kSleep( TICK ticks)
{
    K_CR_AREA
    K_CR_ENTER

    if (runPtr->status != RUNNING)
    {
        kassert( FAULT_TASK_INVALID_STATE);
    }
    kTimeOut( &runPtr->timeoutNode, ticks);
    runPtr->status = SLEEPING;
    K_PEND_CTXTSWTCH
    K_CR_EXIT
}

#if(K_DEF_SCH_TSLICE!=ON)

VOID kSleepUntil( TICK const period)
{
    K_CR_AREA
    K_CR_ENTER
    TICK currentTick = kTickGet();
    TICK nextWakeTime = runPtr->lastWakeTime + period;
    /*  the task missed its deadline, adjust nextWakeTime to catch up */
    if (currentTick > nextWakeTime)
    {
        nextWakeTime = currentTick + period;
    }
    /* delay required */
    TICK delay = nextWakeTime - currentTick;
    /* if any */
    if (delay > 0)
    {
        kTimeOut( &runPtr->timeoutNode, period);

        runPtr->status = SLEEPING;
        K_PEND_CTXTSWTCH

    }
    runPtr->timeOut = FALSE;
    /* Update the last wake time */
    runPtr->lastWakeTime = nextWakeTime;
    K_CR_EXIT
}

#endif

/* timeout and sleeping list (delta-list) */
K_ERR kTimeOut( K_TIMEOUT_NODE *timeOutNode, TICK timeout)
{

    if (timeout == 0)
        return (K_ERR_INVALID_PARAM);
    if (timeOutNode == NULL)
        return (K_ERR_OBJ_NULL);

    runPtr->timeOut = FALSE;
    timeOutNode->timeout = timeout;
    timeOutNode->dtick = timeout;
    timeOutNode->prevPtr = NULL;
    timeOutNode->nextPtr = NULL;
#if (K_DEF_CALLOUT_TIMER==ON)
    if (timeOutNode->objectType == TIMER)
    {

        timeOutNode->timeout = timeout;
        timeOutNode->dtick = timeout;
        K_TIMEOUT_NODE *currPtr = ( K_TIMEOUT_NODE*) timerListHeadPtr;
        K_TIMEOUT_NODE *prevPtr = NULL;

        while (currPtr != NULL && currPtr->dtick < timeOutNode->dtick)
        {
            timeOutNode->dtick -= currPtr->dtick;
            prevPtr = currPtr;
            currPtr = currPtr->nextPtr;
        }

        timeOutNode->nextPtr = currPtr;
        if (currPtr != NULL)
        {
            currPtr->dtick -= timeOutNode->dtick;
            timeOutNode->prevPtr = currPtr->prevPtr;
            currPtr->prevPtr = timeOutNode;
        }
        else
        {
            timeOutNode->prevPtr = prevPtr;
        }

        if (prevPtr == NULL)
        {
            timerListHeadPtr = timeOutNode;
        }
        else
        {
            prevPtr->nextPtr = timeOutNode;
        }

    }
    else
    {
#endif
        K_TIMEOUT_NODE *currPtr = ( K_TIMEOUT_NODE*) timeOutListHeadPtr;
        K_TIMEOUT_NODE *prevPtr = NULL;

        while (currPtr != NULL && currPtr->dtick <= timeOutNode->dtick)
        {
            timeOutNode->dtick -= currPtr->dtick;
            prevPtr = currPtr;
            currPtr = currPtr->nextPtr;
        }

        timeOutNode->nextPtr = currPtr;
        if (currPtr != NULL)
        {
            currPtr->dtick -= timeOutNode->dtick;
            timeOutNode->prevPtr = currPtr->prevPtr;
            currPtr->prevPtr = timeOutNode;
        }

        if (prevPtr == NULL)
        {
            timeOutListHeadPtr = timeOutNode;
        }
        else
        {
            prevPtr->nextPtr = timeOutNode;
            timeOutNode->prevPtr = prevPtr;
        }
#if (K_DEF_CALLOUT_TIMER==ON)
    }
#endif
    return (K_SUCCESS);
}

/* Handler traverses the list and process each object accordinly */

K_ERR kRemoveTaskFromPendingOrSleeping( volatile K_TIMEOUT_NODE *node)
{
    K_TCB *taskPtr = K_GET_CONTAINER_ADDR( node, K_TCB, timeoutNode);

    if (taskPtr->status == PENDING)
    {
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
        }
        return (K_SUCCESS);
    }
    else if (taskPtr->status == SLEEPING)
    {
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
        }
        return (K_SUCCESS);
    }
    return (K_ERROR);
}

#if (K_DEF_MBOX==ON)
K_ERR kRemoveTaskFromMbox( volatile K_TIMEOUT_NODE *node)
{

    K_MBOX *mboxPtr = K_GET_CONTAINER_ADDR( node, K_MBOX, timeoutNode);
    if (mboxPtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &mboxPtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
        }
        return (K_SUCCESS);
    }
    return (K_ERROR);
}
#endif

#if (K_DEF_QUEUE==ON)

K_ERR kRemoveTaskFromMQueue( volatile K_TIMEOUT_NODE *node)
{

    K_QUEUE *mmboxPtr = K_GET_CONTAINER_ADDR( node, K_QUEUE, timeoutNode);
    /*mmbox = multi-mail box */
    if (mmboxPtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &mmboxPtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
            return (K_SUCCESS);

        }
    }
    return (K_ERROR);
}
#endif

#if (K_DEF_SEMA==ON)
K_ERR kRemoveTaskFromSema( volatile K_TIMEOUT_NODE *node)
{

    K_SEMA *semaPtr = K_GET_CONTAINER_ADDR( node, K_SEMA, timeoutNode);
    if (semaPtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &semaPtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
            return (K_SUCCESS);
        }
    }
    return (K_ERROR);
}
#endif

#if (K_DEF_MUTEX==ON)
K_ERR kRemoveTaskFromMutex( volatile K_TIMEOUT_NODE *node)
{

    K_MUTEX* mutexPtr = K_GET_CONTAINER_ADDR( node, K_MUTEX, timeoutNode);
    if (mutexPtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &mutexPtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
            return (K_SUCCESS);
        }
    }
    return (K_ERROR);
}
#endif

#if (K_DEF_STREAM==ON)
K_ERR kRemoveTaskFromStream( volatile K_TIMEOUT_NODE *node)
{

    /* queuePtr -> pointer to stream queue */
    K_STREAM *queuePtr = K_GET_CONTAINER_ADDR( node, K_STREAM, timeoutNode);
    if (queuePtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &queuePtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
            return (K_SUCCESS);
        }
    }
    return (K_ERROR);
}
#endif
#if (K_DEF_EVENT==ON)
K_ERR kRemoveTaskFromEvent( volatile K_TIMEOUT_NODE *node)
{

    K_EVENT *eventPtr = K_GET_CONTAINER_ADDR( node, K_EVENT, timeoutNode);
    if (eventPtr->waitingQueue.size > 0)
    {
        K_TCB *taskPtr;
        kTCBQDeq( &eventPtr->waitingQueue, &taskPtr);
        taskPtr->timeOut = TRUE;
        if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
        {
            taskPtr->status = READY;
            return (K_SUCCESS);
        }

    }
    return (K_ERROR);
}
#endif
/* runs @ systick */
static volatile K_TIMEOUT_NODE *node;
BOOL kHandleTimeoutList( VOID)
{
    K_ERR err = K_ERROR;


        if (timeOutListHeadPtr->dtick > 0)
        {
            timeOutListHeadPtr->dtick --;
        }

        /*  possible to have a node which offset is already (dtick == 0) */
        while (timeOutListHeadPtr != NULL && timeOutListHeadPtr->dtick == 0)
        {
            node = timeOutListHeadPtr;
            /* Remove the expired node from the list */
            timeOutListHeadPtr = node->nextPtr;
            kRemoveTimeoutNode( ( K_TIMEOUT_NODE*) node);
            switch (node->objectType)
            {
#if (K_DEF_MBOX==ON)
            case MAILBOX:
              err  = kRemoveTaskFromMbox( node);
                break;
#endif
#if (K_DEF_QUEUE==ON)
            case QUEUE:
                err = kRemoveTaskFromMQueue( node);
                break;
#endif
#if (K_DEF_SEMA==ON)
            case SEMAPHORE:
                err = kRemoveTaskFromSema( node);
                break;
#endif
#if (K_DEF_MUTEX==ON)
            case MUTEX:
                err = kRemoveTaskFromMutex( node);
                break;
#endif
#if (K_DEF_STREAM==ON)
            case STREAM:
                err = kRemoveTaskFromStream( node);
                break;
#endif
#if (K_DEF_EVENT==ON)
            case EVENT:
                err = kRemoveTaskFromEvent( node);
                break;
#endif
            case TASK_HANDLE:

                err = kRemoveTaskFromPendingOrSleeping( node);
                break;
            default:
                err = K_ERROR;
                KFAULT( FAULT);
                break;
            }
        }

    return (err == K_SUCCESS);
}

VOID kRemoveTimeoutNode( K_TIMEOUT_NODE *node)
{
    if (node == NULL)
        return;

    if (node->nextPtr != NULL)
    {
        node->nextPtr->dtick += node->dtick;
        node->nextPtr->prevPtr = node->prevPtr;
    }

    if (node->prevPtr != NULL)
    {
        node->prevPtr->nextPtr = node->nextPtr;
    }
    else
    {
        timeOutListHeadPtr = node->nextPtr;
    }

    node->nextPtr = NULL;
    node->prevPtr = NULL;
}
