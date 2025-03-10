/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module           :  High-Level Scheduler / Kernel Initialisation
 * 	Provides to      :  All services
 *  Depends  on      :  Low-Level Scheduler
 *  Public API 		 :  Yes
 *
 * 	In this unit:
 * 					o Scheduler routines
 * 					o Tick Management
 * 		            o Task Queues Management
 *			 		o Task Control Block Management
 *
 ******************************************************************************/

#define K_CODE

#include "kexecutive.h"

/*****************************************************************************/

/* scheduler globals */
K_TCBQ readyQueue[K_DEF_MIN_PRIO + 2];
K_TCB *runPtr;
K_TCB tcbs[NTHREADS];
K_TASK_HANDLE timTaskHandle;
K_TASK_HANDLE idleTaskHandle;
struct kRunTime runTime;
/* local globals  */
static PRIO highestPrio = 0;
static PRIO const lowestPrio = K_DEF_MIN_PRIO;
static PRIO nextTaskPrio = 0;
static PRIO idleTaskPrio = K_DEF_MIN_PRIO + 1;
static volatile ULONG readyQBitMask;
static volatile ULONG readyQRightMask;
static volatile ULONG version;
/* fwded private helpers */
static inline VOID kReadyRunningTask_( VOID);
static inline PRIO kCalcNextTaskPrio_();
#if (K_DEF_SCH_TSLICE == ON)
static inline BOOL kDecTimeSlice_( VOID);
#endif

/*******************************************************************************
 * YIELD
 *******************************************************************************/

VOID kYield( VOID) /*  <- USE THIS =) */
{
    K_CR_AREA
    K_CR_ENTER
    kReadyRunningTask_();
    K_PEND_CTXTSWTCH
    K_CR_EXIT

}
/*******************************************************************************
 TASK QUEUE MANAGEMENT
 *******************************************************************************/

K_ERR kTCBQInit( K_TCBQ *const kobj, STRING listName)
{
    if (IS_NULL_PTR( kobj))
    {
        kErrHandler( FAULT_OBJ_NULL);
        return (K_ERR_OBJ_NULL);
    }
    return (kListInit( kobj, listName));
}

K_ERR kTCBQEnq( K_TCBQ *const kobj, K_TCB *const tcbPtr)
{
    K_CR_AREA
    K_CR_ENTER
    if (kobj == NULL || tcbPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    K_ERR err = kListAddTail( kobj, &(tcbPtr->tcbNode));
    if (err == 0)
    {
        if (kobj == &readyQueue[tcbPtr->priority])
            readyQBitMask |= 1 << tcbPtr->priority;
    }
    K_CR_EXIT
    return (err);
}

K_ERR kTCBQDeq( K_TCBQ *const kobj, K_TCB **const tcbPPtr)
{
    if (kobj == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    K_NODE *dequeuedNodePtr = NULL;
    K_ERR err = kListRemoveHead( kobj, &dequeuedNodePtr);

    if (err != K_SUCCESS)
    {
        return (err);
    }
    *tcbPPtr = K_LIST_GET_TCB_NODE( dequeuedNodePtr, K_TCB);

    if (*tcbPPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
        return (K_ERR_OBJ_NULL);
    }
    K_TCB *tcbPtr_ = *tcbPPtr;
    PRIO prio_ = tcbPtr_->priority;
    if ((kobj == &readyQueue[prio_]) && (kobj->size == 0))
        readyQBitMask &= ~(1U << prio_);
    return (K_SUCCESS);
}

K_ERR kTCBQRem( K_TCBQ *const kobj, K_TCB **const tcbPPtr)
{
    if (kobj == NULL || tcbPPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    K_NODE *dequeuedNodePtr = &((*tcbPPtr)->tcbNode);
    K_ERR err = kListRemove( kobj, dequeuedNodePtr);
    if (err != K_SUCCESS)
    {
        return (err);
    }
    *tcbPPtr = K_LIST_GET_TCB_NODE( dequeuedNodePtr, K_TCB);
    if (*tcbPPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    return (K_SUCCESS);
}

K_TCB* kTCBQPeek( K_TCBQ *const kobj)
{
    if (kobj == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    K_NODE *nodePtr = kobj->listDummy.nextPtr;
    return K_GET_CONTAINER_ADDR( nodePtr, K_TCB, tcbNode);
}

K_ERR kTCBQEnqByPrio( K_TCBQ *const kobj, K_TCB *const tcbPtr)
{
    K_ERR err;
    if (kobj == NULL || tcbPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    if (kobj->size == 0)
    {
        /* enq on tail */
        err = kTCBQEnq( kobj, tcbPtr);
        return (err);
    }
    /* start on the tail and traverse with >= cond,    */
    /*  so we use a single insertafter.                */
    K_NODE *currNodePtr = kobj->listDummy.prevPtr;
    K_TCB *currTcbPtr = K_LIST_GET_TCB_NODE( currNodePtr, K_TCB);
    while (currTcbPtr->priority >= tcbPtr->priority)
    {
        currNodePtr = currNodePtr->nextPtr;
        /* list end */
        if (currNodePtr == &(kobj->listDummy))
        {
            break;
        }
        currTcbPtr = K_LIST_GET_TCB_NODE( currNodePtr, K_TCB);
    }
    err = kListInsertAfter( kobj, currNodePtr, &(tcbPtr->tcbNode));
    kassert( err == 0);
    return (err);
}

#define INSTANT_PREEMPT_LOWER_PRIO
/* this function enq ready and pend ctxt swtch if ready > running */
K_ERR kReadyCtxtSwtch( K_TCB *const tcbPtr)
{
    K_CR_AREA
    K_CR_ENTER
    if (IS_NULL_PTR( tcbPtr))
    {
        kErrHandler( FAULT_OBJ_NULL);
        K_CR_EXIT
        return (K_ERR_OBJ_NULL);
    }
    if (kTCBQEnq( &readyQueue[tcbPtr->priority], tcbPtr) == K_SUCCESS)
    {
        tcbPtr->status = READY;
#ifdef INSTANT_PREEMPT_LOWER_PRIO
        if (READY_HIGHER_PRIO( tcbPtr))
        {
            kassert( !kTCBQEnq( &readyQueue[runPtr->priority], runPtr));
            runPtr->status = READY;
            K_PEND_CTXTSWTCH
        }
#endif
    }
    K_CR_EXIT
    return (K_SUCCESS);
}

K_ERR kReadyQDeq( K_TCB **const tcbPPtr, PRIO priority)
{

    K_ERR err = kTCBQDeq( &readyQueue[priority], tcbPPtr);
    kassert( err == 0);
    return (err);
}

/*******************************************************************************
 * Task Control Block Management
 *******************************************************************************/
static PID pPid = 0; /** system pid for each task 	*/

/* fwded private prototypes */
static K_ERR kInitStack_( INT *const stackAddrPtr, UINT const stackSize,
        TASKENTRY const taskFuncPtr); /* init stacks */

static K_ERR kInitTcb_( TASKENTRY const taskFuncPtr, INT *const stackAddrPtr,
        UINT const stackSize);
/**/

static K_ERR kInitStack_( INT *const stackAddrPtr, UINT const stackSize,
        TASKENTRY const taskFuncPtr)
{
    if (stackAddrPtr == NULL || stackSize == 0 || taskFuncPtr == NULL)
    {
        return (K_ERROR);
    }
    stackAddrPtr[stackSize - PSR_OFFSET] = 0x01000000;
    stackAddrPtr[stackSize - PC_OFFSET] = ( INT) taskFuncPtr;
    stackAddrPtr[stackSize - LR_OFFSET] = 0x14141414;
    stackAddrPtr[stackSize - R12_OFFSET] = 0x12121212;
    stackAddrPtr[stackSize - R3_OFFSET] = 0x03030303;
    stackAddrPtr[stackSize - R2_OFFSET] = 0x02020202;
    stackAddrPtr[stackSize - R1_OFFSET] = 0x01010101;
    stackAddrPtr[stackSize - R0_OFFSET] = 0x00000000;
    stackAddrPtr[stackSize - R11_OFFSET] = 0x11111111;
    stackAddrPtr[stackSize - R10_OFFSET] = 0x10101010;
    stackAddrPtr[stackSize - R9_OFFSET] = 0x09090909;
    stackAddrPtr[stackSize - R8_OFFSET] = 0x08080808;
    stackAddrPtr[stackSize - R7_OFFSET] = 0x07070707;
    stackAddrPtr[stackSize - R6_OFFSET] = 0x06060606;
    stackAddrPtr[stackSize - R5_OFFSET] = 0x05050505;
    stackAddrPtr[stackSize - R4_OFFSET] = 0x04040404;
    /*stack painting*/
    for (ULONG j = 17; j < stackSize; j ++)
    {
        stackAddrPtr[stackSize - j] = ( INT) 0xBADC0FFE;
    }
    stackAddrPtr[0] = 0x0BADC0DE;
    return (K_SUCCESS);
}

static K_ERR kInitTcb_( TASKENTRY const taskFuncPtr, INT *const stackAddrPtr,
        UINT const stackSize)
{
    if (kInitStack_( stackAddrPtr, stackSize, taskFuncPtr) == K_SUCCESS)
    {
        tcbs[pPid].stackAddrPtr = stackAddrPtr;
        tcbs[pPid].sp = &stackAddrPtr[stackSize - R4_OFFSET];
        tcbs[pPid].stackSize = stackSize;
        tcbs[pPid].status = READY;
        tcbs[pPid].pid = pPid;

        return (K_SUCCESS);
    }
    return (K_ERROR);
}

K_ERR kCreateTask( K_TASK_HANDLE *taskHandlePtr, TASKENTRY const taskFuncPtr,
        STRING taskName, INT *const stackAddrPtr, UINT const stackSize,
#if(K_DEF_SCH_TSLICE==ON)
        TICK const timeSlice,
#endif
        PRIO const priority, BOOL const runToCompl)
{

    /* if private PID is 0, system tasks hasn't been started yet */
    if (pPid == 0)
    {
        /* initialise IDLE TASK */
        kassert( kInitTcb_(IdleTask, idleStack, IDLE_STACKSIZE) == K_SUCCESS);

        tcbs[pPid].priority = idleTaskPrio;
#if ( (K_DEF_FUNC_DYNAMIC_PRIO==ON) || (K_DEF_MUTEX_PRIO_INH==ON) )
        tcbs[pPid].realPrio = idleTaskPrio;
#endif
        tcbs[pPid].taskName = "IdleTask";
        tcbs[pPid].runToCompl = FALSE;
#if(K_DEF_SCH_TSLICE==ON)

        tcbs[pPid].timeSlice = 0;
#endif
        idleTaskHandle = &tcbs[pPid];
        pPid += 1;

        /* initialise TIMER HANDLER TASK */
        kassert(
                kInitTcb_(TimerHandlerTask, timerHandlerStack, TIMHANDLER_STACKSIZE) == K_SUCCESS);

        tcbs[pPid].priority = 0;
#if ( (K_DEF_FUNC_DYNAMIC_PRIO==ON) || (K_DEF_MUTEX_PRIO_INH==ON) )
        tcbs[pPid].realPrio = 0;
#endif
        tcbs[pPid].taskName = "TimHandlerTask";
        tcbs[pPid].runToCompl = TRUE;
#if(K_DEF_SCH_TSLICE==ON)

        tcbs[pPid].timeSlice = 0;
#endif
        timTaskHandle = &tcbs[pPid];
        pPid += 1;

    }
    /* initialise user tasks */
    if (kInitTcb_( taskFuncPtr, stackAddrPtr, stackSize) == K_SUCCESS)
    {
        if (priority > idleTaskPrio)
        {
            kErrHandler( FAULT_TASK_INVALID_PRIO);
        }
        tcbs[pPid].priority = priority;
#if ( (K_DEF_FUNC_DYNAMIC_PRIO==ON) || (K_DEF_MUTEX_PRIO_INH==ON) )
        tcbs[pPid].realPrio = priority;
#endif
        tcbs[pPid].taskName = taskName;

#if(K_DEF_SCH_TSLICE==ON)
        tcbs[pPid].timeSlice = timeSlice;
        tcbs[pPid].timeSliceCnt = 0UL;
#else
		tcbs[pPid].lastWakeTime = 0;
#endif
        tcbs[pPid].signalled = FALSE;
        tcbs[pPid].runToCompl = runToCompl;

        *taskHandlePtr = &tcbs[pPid];
        (*taskHandlePtr)->timeoutNode.objectType = TASK_HANDLE;
        pPid += 1;
        return (K_SUCCESS);
    }

    return (K_ERROR);
}
/*******************************************************************************
 * CRITICAL REGIONS
 *******************************************************************************/

UINT kEnterCR( VOID)
{
    asm volatile("DSB");

    UINT crState;

    crState = __get_PRIMASK();
    if (crState == 0)
    {
        asm volatile("DSB");
        asm volatile ("CPSID I");
        asm volatile("ISB");

        return (crState);
    }
    asm volatile("DSB");
    return (crState);
}

VOID kExitCR( UINT crState)
{
    asm volatile("DSB");
    __set_PRIMASK( crState);
    asm volatile ("ISB");

}

#if (K_DEF_FUNC_DYNAMIC_PRIO==(ON))
K_ERR kTaskChangePrio( PRIO newPrio)
{
    if (kIsISR())
        return (K_ERR_INVALID_ISR_PRIMITIVE);
    K_CR_AREA
    K_CR_ENTER
    runPtr->priority = newPrio;
    K_CR_EXIT
    return (K_SUCCESS);
}

K_ERR kTaskRestorePrio( VOID)
{
    if (kIsISR())
        return (K_ERR_INVALID_ISR_PRIMITIVE);
    K_CR_AREA
    K_CR_ENTER
    runPtr->priority = runPtr->realPrio;
    K_CR_EXIT
    return (K_SUCCESS);
}
#endif

/******************************************************************************
 * KERNEL INITIALISATION
 *******************************************************************************/

static VOID kInitRunTime_( VOID)
{
    runTime.globalTick = 0;
    runTime.nWraps = 0;
}
static K_ERR kInitQueues_( VOID)
{
    K_ERR err = 0;
    for (PRIO prio = 0; prio < NPRIO + 1; prio ++)
    {
        err |= kTCBQInit( &readyQueue[prio], "ReadyQ");
    }
    kassert( err == 0);
    return (err);
}

VOID kInit( VOID)
{

    version = kGetVersion();
    if (version != K_VALID_VERSION)
        kErrHandler( FAULT_KERNEL_VERSION);
    kInitQueues_();
    kInitRunTime_();
    highestPrio = tcbs[0].priority;
    for (ULONG i = 0; i < NTHREADS; i ++)
    {
        if (tcbs[i].priority < highestPrio)
        {
            highestPrio = tcbs[i].priority;
        }
    }

    for (ULONG i = 0; i < NTHREADS; i ++)
    {
        kTCBQEnq( &readyQueue[tcbs[i].priority], &tcbs[i]);
    }

    kReadyQDeq( &runPtr, highestPrio);
    kassert( runPtr->status == READY);
    kassert( tcbs[IDLETASK_ID].priority == lowestPrio+1);
    kApplicationInit();
    __enable_irq();
    _K_STUP
    while (1)
        ;

    /* calls low-level scheduler for start-up */
}

/*******************************************************************************
 *  TICK MANAGEMENT
 *******************************************************************************/
static inline VOID kReadyRunningTask_( VOID)
{
    if (runPtr->status == RUNNING)
    {
        kassert(
                (kTCBQEnq( &readyQueue[runPtr->priority], runPtr) == (K_SUCCESS)));
        runPtr->status = READY;
    }
}

#if (K_DEF_SCH_TSLICE == ON)
static inline BOOL kDecTimeSlice_( VOID)
{
    if ((runPtr->status == RUNNING) && (runPtr->runToCompl == FALSE ))
    {

        runPtr->timeSliceCnt += 1UL;
        if (runPtr->busyWaitTime > 0)
        {
            runPtr->busyWaitTime -= 1U;
        }
        return (runPtr->timeSliceCnt == runPtr->timeSlice);
    }
    return (FALSE );
}
#endif
volatile K_TIMEOUT_NODE *timeOutListHeadPtr = NULL;
volatile K_TIMEOUT_NODE *timerListHeadPtr = NULL;
volatile K_TIMER *headTimPtr;
volatile K_TIMEOUT_NODE *timerListHeadPtrSaved = NULL;

BOOL kTickHandler( VOID)
{
    /* return is short-circuit to !runToCompl & */
    BOOL runToCompl = FALSE;
    BOOL timeOutTask = FALSE;
    BOOL ret = FALSE;
    runTime.globalTick += 1U;
#if (K_DEF_SCH_TSLICE!=ON)
    if (runPtr->busyWaitTime > 0)
    {
        runPtr->busyWaitTime -= 1U;
    }
#endif
    if (runTime.globalTick == K_TICK_TYPE_MAX)
    {
        runTime.globalTick = 0U;
        runTime.nWraps += 1U;
    }

    if (runPtr->yield == TRUE)
    {
        kReadyRunningTask_();
    }
    /* handle time out and sleeping list */
    /* the list is not empty, decrement only the head  */
    if (timeOutListHeadPtr != NULL)
    {
        timeOutTask = kHandleTimeoutList();
    }
    /* a run-to-completion task is a first-class citizen not prone to tick
     truculence.*/
    if (runPtr->runToCompl && (runPtr->status == RUNNING))
        /* this flag toggles, short-circuiting the */
        /* return value  to FALSE                  */
        runToCompl = TRUE;

    /* if time-slice is enabled, decrease the time-slice. */
#if (K_DEF_SCH_TSLICE==ON)
    BOOL tsliceDue = FALSE;
    tsliceDue = kDecTimeSlice_();
    if (tsliceDue)
    {
        kReadyRunningTask_();
        runPtr->timeSliceCnt = 0UL;
    }
#endif
#if (K_DEF_CALLOUT_TIMER==ON)
    K_TIMER *headTimPtr = K_GET_CONTAINER_ADDR( timerListHeadPtr, K_TIMER,
            timeoutNode);

    if (timerListHeadPtr != NULL)
    {

        if (headTimPtr->phase > 0)
        {
            headTimPtr->phase --;
        }
        else
        {
            if (timerListHeadPtr->dtick > 0)
                (( K_TIMEOUT_NODE*) timerListHeadPtr)->dtick --;
        }
    }
    if (timerListHeadPtr != NULL && timerListHeadPtr->dtick == 0)
    {
        timerListHeadPtrSaved = timerListHeadPtr;
        kTaskSignal( timTaskHandle);
        timeOutTask = TRUE;
    }
#endif
    /* unless there is a run to completion task running, context switch
     * happens whenever running task is ready (probably yielded or tslice is due
     * context switching by preemption for higher priority, happens whenever a
     * task of higher priority than the running task switches to ready
     * and is unrelated to the the tick handler
     */
    ret = ((!runToCompl) & ((runPtr->status == READY) | timeOutTask));

    return (ret);
}

/*******************************************************************************
 TASK SWITCHING LOGIC
 *******************************************************************************/
static inline PRIO kCalcNextTaskPrio_()
{
    if (readyQBitMask == 0U)
    {
        return (idleTaskPrio);
    }
    readyQRightMask = readyQBitMask & -readyQBitMask;
    PRIO prio = ( PRIO) (__getReadyPrio( readyQRightMask));

    return (prio);
    /* return __builtin_ctz(readyQRightMask); */
}

VOID kSchSwtch( VOID)
{
    K_TCB *nextRunPtr = NULL;
    K_TCB *prevRunPtr = runPtr;
    if (runPtr->status == RUNNING)
    {
        kReadyRunningTask_();
    }
    nextTaskPrio = kCalcNextTaskPrio_(); /* get the next task priority */
    kTCBQDeq( &readyQueue[nextTaskPrio], &nextRunPtr);
    if (nextRunPtr == NULL)
    {
        kErrHandler( FAULT_OBJ_NULL);
    }
    runPtr = nextRunPtr;
    if (nextRunPtr->pid != prevRunPtr->pid)
    {
        runPtr->nPreempted += 1U;
        prevRunPtr->preemptedBy = runPtr->pid;
    }
    if (runPtr->yield)
    {
        runPtr->yield = FALSE;
    }
    return;
}

