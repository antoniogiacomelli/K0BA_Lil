/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
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
	kobj->timeoutNode.kobj = (ADDR) kobj;
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
	K_ENTER_CR
	kTimerListAdd_( kobj, phase, duration, funPtr, argsPtr, reload);
	K_EXIT_CR
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
	K_ENTER_CR

	if (runPtr->status != RUNNING)
	{
		kassert( FAULT_TASK_INVALID_STATE);
	}
	kTimeOut( &runPtr->taskHandlePtr->timeoutNode, ticks);
	runPtr->status = SLEEPING;
	K_PEND_CTXTSWTCH
	K_EXIT_CR
}

VOID kSleepUntil( TICK const period)
{
	K_CR_AREA
	K_ENTER_CR
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
		kTimeOut( &runPtr->taskHandlePtr->timeoutNode, period);

		runPtr->status = SLEEPING;
		K_PEND_CTXTSWTCH

	}
	/* Update the last wake time */
	runPtr->lastWakeTime = nextWakeTime;
	K_EXIT_CR
}
/* timeout and sleeping list (delta-list) */
K_ERR kTimeOut( K_TIMEOUT_NODE *timeOutNode, TICK timeout)
{

	if (timeout == 0)
		return (K_ERR_INVALID_PARAM);
	if (timeOutNode == NULL)
		return (K_ERR_OBJ_NULL);

	timeOutNode->timeout = timeout;
	timeOutNode->dtick = timeout;
	timeOutNode->prevPtr = NULL;
	timeOutNode->nextPtr = NULL;
#if (K_DEF_CALLOUT_TIMER==ON)
	if (timeOutNode->objectType == TIMER)
	{

		timeOutNode->timeout = timeout;
		timeOutNode->dtick = timeout;
		K_TIMEOUT_NODE *currPtr = (K_TIMEOUT_NODE*) timerListHeadPtr;
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
		K_TIMEOUT_NODE *currPtr = (K_TIMEOUT_NODE*) timeOutListHeadPtr;
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

VOID kRemoveTaskFromPendingOrSleeping( ADDR kobj)
{
	K_TCB *taskPtr = (K_TCB*) kobj;
	if (taskPtr->status == PENDING)
	{
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
	else if (taskPtr->status == SLEEPING)
	{
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
		else
		{
			kassert( 0);
		}
	}

}

#if (K_DEF_MBOX==ON)
VOID kRemoveTaskFromMbox( ADDR kobj)
{
	K_MBOX *mboxPtr = (K_MBOX*) kobj;
	if (mboxPtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &mboxPtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif

#if (K_DEF_QUEUE==ON)

VOID kRemoveTaskFromMQueue( ADDR kobj)
{
	K_QUEUE *mmboxPtr = (K_QUEUE*) kobj;

	if (mmboxPtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &mmboxPtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif

#if (K_DEF_SEMA==ON)
void kRemoveTaskFromSema( void *kobj)
{
	K_SEMA *semaPtr = (K_SEMA*) kobj;
	if (semaPtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &semaPtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif

#if (K_DEF_MUTEX==ON)
VOID kRemoveTaskFromMutex( ADDR kobj)
{
	K_MUTEX *mutexPtr = (K_MUTEX*) kobj;
	if (mutexPtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &mutexPtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif

#if (K_DEF_STREAM==ON)
VOID kRemoveTaskFromStream( ADDR kobj)
{
	K_STREAM *queuePtr = (K_STREAM*) kobj;
	if (queuePtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &queuePtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif
#if (K_DEF_SLEEPWAKE==ON)
VOID kRemoveTaskFromEvent( ADDR kobj)
{
	K_EVENT *eventPtr = (K_EVENT*) kobj;
	if (eventPtr->waitingQueue.size > 0)
	{
		K_TCB *taskPtr;
		kTCBQDeq( &eventPtr->waitingQueue, &taskPtr);
		taskPtr->timeOut = TRUE;
		if (!kTCBQEnq( &readyQueue[taskPtr->priority], taskPtr))
		{
			taskPtr->status = READY;
		}
	}
}
#endif
/* runs @ systick */
static volatile K_TIMEOUT_NODE *node;
BOOL kHandleTimeoutList( VOID)
{
	BOOL ret = FALSE;

	/* the list is not empty, decrement only the head  */
	if (timeOutListHeadPtr != NULL)
	{
		if (timeOutListHeadPtr->dtick > 0)
		{
			timeOutListHeadPtr->dtick--;
		}

		/*  possible to have a node which offset is already (dtick == 0) */
		while (timeOutListHeadPtr != NULL && timeOutListHeadPtr->dtick == 0)
		{
			node = timeOutListHeadPtr;
			/* Remove the expired node from the list */
			timeOutListHeadPtr = node->nextPtr;
			kRemoveTimeoutNode( (K_TIMEOUT_NODE*) node);
			ret = TRUE;
			switch (node->objectType)
			{
#if (K_DEF_MBOX==ON)
			case MAILBOX:
				kRemoveTaskFromMbox( node->kobj);
				break;
#endif
#if (K_DEF_QUEUE==ON)
			case QUEUE:
				kRemoveTaskFromMQueue( node->kobj);
				break;
#endif
#if (K_DEF_SEMA==ON)
			case SEMAPHORE:
				kRemoveTaskFromSema( node->kobj);
				break;
#endif
#if (K_DEF_MUTEX==ON)
			case MUTEX:
				kRemoveTaskFromMutex( node->kobj);
				break;
#endif
#if (K_DEF_STREAM==ON)
			case STREAM:
				kRemoveTaskFromStream( node->kobj);
				break;
#endif
#if (K_DEF_SLEEPWAKE==ON)
			case EVENT:
				kRemoveTaskFromEvent( node->kobj);
				break;
#endif
			case TASK_HANDLE:
				kRemoveTaskFromPendingOrSleeping( node->kobj);
				break;
			default:
				KFAULT( FAULT);
				break;
			}
		}
	}
	return (ret);
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

