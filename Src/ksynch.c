/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications | [VERSION: 0.3.1]]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module              : Inter-task Synchronisation
 *  Depends on          : Scheduler, Timer
 *  Provides to         : Application
 *  Public API  	    : Yes
 *
 * 	In this unit:
 *					o Events (Condition Variables and Event Flags)
 *					o Semaphores (Counter, Binary and Mutexes)
 *
 *****************************************************************************/

#define K_CODE

#include "kexecutive.h"

/*******************************************************************************
 * DIRECT TASK SIGNAL
 *******************************************************************************/
/*
 this function pends on a task private binary semaphore
 if already signalled task proceeds, otherwise switches
 to PENDING status until signalled, returning SUCCESS
 if timeout, task get READY again  returning ERR_TIMEOUT
 */
K_ERR kTaskPend( TICK timeout)
{
	K_ERR err;
	if (kIsISR())
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	K_CR_AREA
	K_CR_ENTER
	if (runPtr->signalled == TRUE)
	{
		runPtr->signalled = FALSE;
		err = (K_SUCCESS);
	}
	else
	{
		runPtr->status = PENDING;
		err = K_SUCCESS;
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &runPtr->taskHandlePtr->timeoutNode, timeout);
		}
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		/* resuming here, if time is out, return error */
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			err = K_ERR_TIMEOUT;
		}
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kRemoveTimeoutNode( &runPtr->taskHandlePtr->timeoutNode);

	}
	K_CR_EXIT
	return (err);
}

/* Signal a task directly on its private bin semaphore
 If pending task is readied. If not, task bin semaphore
 remains or switches to TRUE
 */
K_ERR kTaskSignal( K_TASK *const taskHandlePtr)
{

	K_ERR err = -1;
	/*
	 a taskhandler cannot be null
	 a task cannot signal itself, signal the timer handler or signal itself
	 */
	if ((taskHandlePtr == NULL) || (taskHandlePtr->tcbPtr->pid == runPtr->pid)
			|| (taskHandlePtr->tcbPtr->pid == TIMHANDLER_ID)
			|| (taskHandlePtr->tcbPtr->pid == IDLETASK_ID))
		return (err);
	K_CR_AREA
	K_CR_ENTER
	PID pid = taskHandlePtr->tcbPtr->pid;
	if (tcbs[pid].status == PENDING)
	{
		kReadyCtxtSwtch( &tcbs[pid]);
	}
	else
	{
		taskHandlePtr->tcbPtr->signalled = TRUE;
		err = (K_SUCCESS);
	}
	K_CR_EXIT
	return (err);
}

/******************************************************************************
 * TASK FLAGS
 ******************************************************************************/
#if (K_DEF_TASK_FLAGS==ON)
/* running task flags */
#define RUN_FLAGS (runPtr->currFlags)
#define RUN_FLAGS_OPTION (runPtr->flagsOptions)
/* updates target task flags
 * if it is pending, make it ready */
ULONG kTaskFlagsPost( K_TASK *const taskHandlerPtr, ULONG flagMask)
{
	K_CR_AREA
	K_CR_ENTER

	/* update the task's flags */
	taskHandlerPtr->tcbPtr->currFlags |= flagMask;
	ULONG updatedFlags = taskHandlerPtr->tcbPtr->currFlags;

	/* check if task is pending and should wake up */
	if (taskHandlerPtr->tcbPtr->status == PENDING_FLAGS)
	{
		BOOL all = (taskHandlerPtr->tcbPtr->flagsOptions & K_ALL)
				|| (taskHandlerPtr->tcbPtr->flagsOptions & K_ALL_CLEAR);
		BOOL clear = (taskHandlerPtr->tcbPtr->flagsOptions & K_ANY_CLEAR)
				|| (taskHandlerPtr->tcbPtr->flagsOptions & K_ALL_CLEAR);

		/* Wake only if condition is met */
		if ((all && ((taskHandlerPtr->tcbPtr->currFlags & flagMask) == flagMask))
				|| (!all && (taskHandlerPtr->tcbPtr->currFlags & flagMask)))
		{
			/* move task to READY state */
			taskHandlerPtr->tcbPtr->status = READY;
			kReadyCtxtSwtch( taskHandlerPtr->tcbPtr);
			/* Clear flags if necessary */
			if (clear)
			{
				taskHandlerPtr->tcbPtr->currFlags &= ~flagMask;
			}
		}
	}

	K_CR_EXIT
	return updatedFlags;
}

/*
 * caller checks on a combination of flags
 * if this combination is already set, function returns
 * if it is not set, caller blocks
 */
ULONG kTaskFlagsPend( ULONG flagMask, ULONG option, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	BOOL clear = 0;
	BOOL all = 0;
	ULONG currFlags = RUN_FLAGS;
	switch (option)
	{
	case (K_ANY):
		clear = 0;
		all = 0;
		break;
	case (K_ALL):
		clear = 0;
		all = 1;
		break;
	case (K_ANY_CLEAR):
		clear = 1;
		all = 0;
		break;
	case (K_ALL_CLEAR):
		clear = 1;
		all = 1;
		break;
	default:
		/* not valid option, return with no updates */
		goto EXIT;
	}
	/*  given option parameters, do not pend if
	 *  current flags meet flagMask  */
	if ((all && ((currFlags & flagMask) == flagMask))
			|| (!all && (currFlags & flagMask)))
	{
		/* if CLEAR option, zero flags for AND
		 * clear met flags for OR */
		if (clear)
		{
			{ /* Only clear the met flags */
				currFlags &= ~flagMask;
			}
		}
	}
	else /* if not, pend */
	{
		runPtr->status = PENDING_FLAGS;
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &runPtr->taskHandlePtr->timeoutNode, timeout);
		}
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		/* resuming here */
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			goto EXIT;
			/* timeout; return without updating flags */
		}
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kRemoveTimeoutNode( &runPtr->taskHandlePtr->timeoutNode);
	}
	/* update flags */
	RUN_FLAGS = currFlags;
	EXIT:
	K_CR_EXIT
	return (RUN_FLAGS);
}
#endif
/******************************************************************************
 * SLEEP/WAKE ON EVENTS
 ******************************************************************************/
#if (K_DEF_EVENT==ON)
K_ERR kEventInit( K_EVENT *const kobj)
{

	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	K_CR_AREA
	K_CR_ENTER
	kassert( !kTCBQInit( &(kobj->waitingQueue), "eventQ"));
	kobj->init = TRUE;
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = EVENT;
#if (K_DEF_EVENT_FLAGS==ON)
	kobj->eventFlags = 0UL;
#endif
	K_CR_EXIT
	return (K_SUCCESS);
}
/*
 Sleep waiting for a signal or wake event
 tasks sleeping for an event are enqueued and
 dequeued by priority
 */
K_ERR kEventSleep( K_EVENT *kobj, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kIsISR())
	{
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	}
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
	runPtr->status = SLEEPING;
	if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
	{
		kTimeOut( &kobj->timeoutNode, timeout);
	}
	K_PEND_CTXTSWTCH
	K_CR_EXIT
	/* resuming here, if time is out, return error */
	K_CR_ENTER
	if (runPtr->timeOut)
	{
		runPtr->timeOut = FALSE;
		K_CR_EXIT
		return (K_ERR_TIMEOUT);
	}
	else
	{
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
/* broadcast signal to an event - all tasks will switch to READY */
VOID kEventWake( K_EVENT *kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->waitingQueue.size == 0)
		return;
	K_CR_AREA
	K_CR_ENTER
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	ULONG sleepThreads = kobj->waitingQueue.size;
	if (sleepThreads > 0)
	{
		for (ULONG i = 0; i < sleepThreads; ++i)
		{
			K_TCB *nextTCBPtr;
			kTCBQDeq( &kobj->waitingQueue, &nextTCBPtr);
			kassert( !kReadyCtxtSwtch( nextTCBPtr));
		}
	}
	K_CR_EXIT
	return;
}
/*
 signal an event - a single task switches to READY, the highest
 priority one enqueued
 */

VOID kEventSignal( K_EVENT *kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->waitingQueue.size == 0)
		return;
	K_CR_AREA
	K_CR_ENTER
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	K_TCB *nextTCBPtr;
	kTCBQDeq( &kobj->waitingQueue, &nextTCBPtr);
	kassert( !kReadyCtxtSwtch( nextTCBPtr));
	K_CR_EXIT
	return;
}
/* returns the number of tasks sleeping for an event */
UINT kEventQuery( K_EVENT *const kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	return (kobj->waitingQueue.size);
}

/***************************************************************************/
/* CONDITION VARIABLES                                                     */
/***************************************************************************/

#if ((K_DEF_MUTEX==ON))
/* this is a helper for condition variables to perform the wait atomically
 unlocking the mutex and going to sleep
 for signal and wake one can use EventSignal and EventWake */
inline K_ERR kCondVarWait( K_EVENT *eventPtr, K_MUTEX *mutexPtr, TICK timeout)
{
	K_ERR err;
	/* atomic */
	K_CR_AREA
	K_CR_ENTER
	kMutexUnlock( mutexPtr);
	err = kEventSleep( eventPtr, timeout);
	K_CR_EXIT
	return (err);
	/* upon returning (after wake) the condition variable must loop lock the
	 * the mutex and loop around the condition */
}
inline VOID kCondVarSignal( K_EVENT *eventPtr)
{
	return (kEventSignal( eventPtr));
}
inline VOID kCondVarBroad( K_EVENT *eventPtr)
{
	return (kEventWake( eventPtr));
}
#endif

/*****************************************************************************/
/* EVENT FLAGS                                                               */
/*****************************************************************************/

/* Event Flags extend EVENT. For each event there is an associated ULONG to
 * represent a bit string. Every time a task posts to an Event Flags object, the
 * kernel sweep the event list to check if there is one ore more  tasks waiting
 * for that combination. If so, it wakes up that task. Note this combination of
 * flags a task waits for is stored in each task control block - on the same
 * ULONG used for private Flags.
 * When a task pend on a combination of flags associated to a EVENT_FLAGS
 * object, first it checks if the current flags meet the asked combination.
 * If so, task proceeds.
 * if not, the task  switches to SLEEPING state.
 */
#if (K_DEF_EVENT_FLAGS==(ON))
K_ERR kEventFlagsInit( K_EVENT *const kobj, ULONG mask)
{
	K_ERR err = -1;
	K_CR_AREA
	K_CR_ENTER
	err = kEventInit( kobj);
	if (err == K_SUCCESS)
	{
		kobj->eventFlags = mask;
	}
	K_CR_EXIT
	return (err);
}
ULONG kEventFlagsQuery(K_EVENT *const kobj)
{
	return (kobj->eventFlags);
}
K_ERR kEventFlagsGet( K_EVENT *const kobj, ULONG requiredFlags,
		 ULONG options, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	K_ERR err = -1;
	BOOL clear = 0;
	BOOL all = 0;
	ULONG currFlags = kobj->eventFlags;
	runPtr->currFlags = requiredFlags;
	runPtr->flagsOptions = options;
	switch (options)
	{
	case K_ANY:
		clear = 0;
		all = 0;
		break;
	case K_ALL:
		clear = 0;
		all = 1;
		break;
	case K_ANY_CLEAR:
		clear = 1;
		all = 0;
		break;
	case K_ALL_CLEAR:
		clear = 1;
		all = 1;
		break;
	default:
		goto EXIT;
	}

	/* Check if event condition is already met */
	if ((all && ((currFlags & requiredFlags) == requiredFlags))
			|| (!all && (currFlags & requiredFlags)))
	{
		err = K_SUCCESS;
		goto UPDATE;
	}
	else
	{
		/* Block task if condition is not met */
		err = kEventSleep( kobj, timeout);
		K_CR_EXIT
		K_CR_ENTER
		if (err == K_SUCCESS)
		{
			goto UPDATE;
		}
		else
		{
			goto EXIT;
		}
	}
	UPDATE:
	/* If CLEAR option is set, clear only the met flags */
	if (clear)
	{
		kobj->eventFlags &= ~requiredFlags;
	}
	EXIT:
	K_CR_EXIT
	return (err);
}

ULONG kEventFlagsSet( K_EVENT *const kobj, ULONG flagMask)
{
	K_CR_AREA
	K_CR_ENTER

	/* update the event flags */
	kobj->eventFlags |= flagMask;
	ULONG updatedFlags = kobj->eventFlags;

	/* process waiting tasks */
	if (kobj->waitingQueue.size > 0)
	{
		/* start from the last task in the queue */
		K_NODE *currNodePtr = kobj->waitingQueue.listDummy.prevPtr;
		while (currNodePtr != &kobj->waitingQueue.listDummy)
		{
			/* save previous node before modification */
			K_NODE *prevNodePtr = currNodePtr->prevPtr;
			K_TCB *currTcbPtr = K_LIST_GET_TCB_NODE( currNodePtr, K_TCB);

			BOOL all = (currTcbPtr->flagsOptions & K_ALL)
					|| (currTcbPtr->flagsOptions & K_ALL_CLEAR);

			/* wake only if the task's flag condition is met */
			if ((all
					&& ((kobj->eventFlags & currTcbPtr->currFlags)
							== currTcbPtr->currFlags))
					|| (!all && (kobj->eventFlags & currTcbPtr->currFlags)))
			{

				kListRemove( &kobj->waitingQueue, currNodePtr);
				currTcbPtr->gotFlags = kobj->eventFlags;
				kReadyCtxtSwtch( currTcbPtr);

			}
			currNodePtr = prevNodePtr; /* Move to the previous node */
		}
	}

	K_CR_EXIT
	return (updatedFlags);
}

#endif
#endif

#if (K_DEF_SEMA == ON)
/******************************************************************************
 * COUNTER SEMAPHORES
 ******************************************************************************/
/* counter semaphores cannot initialise with a negative value */
K_ERR kSemaInit( K_SEMA *const kobj, const INT value)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
		K_CR_EXIT
		return (K_ERR_OBJ_NULL);
	}
	if (value < 0)
		KFAULT( FAULT);
	kobj->value = value;
	if (kTCBQInit( &(kobj->waitingQueue), "semaQ") != K_SUCCESS)
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	kobj->init = TRUE;
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = SEMAPHORE;
	K_CR_EXIT
	return (K_SUCCESS);
}

/* Counter Semaphores have their own waiting queue and do not
 * handle priority inversion
 * */
K_ERR kSemaWait( K_SEMA *const kobj, const TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kIsISR())
	{
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	kobj->value--;
	DMB
	if (kobj->value < 0)
	{
#if(K_DEF_SEMA_ENQ==K_DEF_ENQ_FIFO)
        kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = BLOCKED;
		DMB
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kTimeOut( &kobj->timeoutNode, timeout);
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			kobj->value += 1;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

VOID kSemaSignal( K_SEMA *const kobj)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	K_TCB *nextTCBPtr = NULL;
	(kobj->value) = (kobj->value) + 1;
	DMB
	if ((kobj->value) <= 0)
	{
		K_ERR err = kTCBQDeq( &(kobj->waitingQueue), &nextTCBPtr);
		kassert( err == 0);
		kassert( nextTCBPtr != NULL);
		err = kReadyCtxtSwtch( nextTCBPtr);
	}
	K_CR_EXIT
	return;
}
#endif

#if (K_DEF_MUTEX == ON)
/*******************************************************************************
 * MUTEX SEMAPHORE
 *******************************************************************************/
/* mutex handle priority inversion by default */

K_ERR kMutexInit( K_MUTEX *const kobj)
{

	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
		return (K_ERROR);
	}
	kobj->lock = FALSE;
	if (kTCBQInit( &(kobj->waitingQueue), "mutexQ") != K_SUCCESS)
	{
		return (K_ERROR);
	}
	kobj->init = TRUE;
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = MUTEX;
	return (K_SUCCESS);
}

K_ERR kMutexLock( K_MUTEX *const kobj, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj->init == FALSE)
	{
		kassert( 0);
	}
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kIsISR())
	{
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	}
	if (kobj->lock == FALSE)
	{
		/* lock mutex and set the owner */
		kobj->lock = TRUE;
		kobj->ownerPtr = runPtr;
		K_CR_EXIT
		return (K_SUCCESS);
	}
	if ((kobj->ownerPtr != runPtr) && (kobj->ownerPtr != NULL))
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MUTEX_LOCKED);
		}
#if(K_DEF_MUTEX_PRIO_INH==(ON))
		if (kobj->ownerPtr->priority > runPtr->priority)
		{
			/* mutex owner has lower priority than the tried-to-lock-task
			 * thus, we boost owner priority, to avoid an intermediate
			 * priority task that does not need lock to preempt
			 * this task, causing an unbounded delay */

			kobj->ownerPtr->priority = runPtr->priority;
		}
#endif
#if(K_DEF_MUTEX_ENQ==K_DEF_ENQ_FIFO)
        kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		if ((timeout > 0) && (timeout < 0xFFFFFFFF))
			kTimeOut( &kobj->timeoutNode, timeout);
		runPtr->status = BLOCKED;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	else
	{
		if (kobj->ownerPtr == runPtr)
		{ /* recursive lock ? why ? WHYYYYYYYYYYY*/
			K_CR_EXIT
			return (K_ERR_MUTEX_REC_LOCK);
		}
	}

	K_CR_EXIT
	return (K_SUCCESS);
}

/* a unlock will fail if not issued by the owner
 * therefore it cannot be within an ISR
 * */
VOID kMutexUnlock( K_MUTEX *const kobj)
{
	K_CR_AREA
	K_CR_ENTER
	K_TCB *tcbPtr;
	if (kIsISR())
	{
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	}
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	if ((kobj->lock == FALSE ))
	{
		return;
	}
	if (kobj->ownerPtr != runPtr)
	{
		KFAULT( FAULT_UNLOCK_OWNED_MUTEX);
		K_CR_EXIT
		return;
	}
	/* runPtr is the owner and mutex was locked */
	if (kobj->waitingQueue.size == 0)
	{
		kobj->lock = FALSE;
#if (K_DEF_MUTEX_PRIO_INH==(ON))
		/* restore owner priority */
		kobj->ownerPtr->priority = kobj->ownerPtr->realPrio;
#endif
		tcbPtr = kobj->ownerPtr;
		kobj->ownerPtr = NULL;
	}
	else
	{
		/* there are waiters, unblock a waiter set new mutex owner.
		 * mutex is still locked */
		kTCBQDeq( &(kobj->waitingQueue), &tcbPtr);
		if (IS_NULL_PTR( tcbPtr))
			KFAULT( FAULT_NULL_OBJ);
#if ((K_DEF_MUTEX_PRIO_INH==ON) )

		/* here only runptr can unlock a mutex*/
		if (runPtr->priority < runPtr->realPrio)
		{
			runPtr->priority = runPtr->realPrio;
		}
#endif
		if (!kReadyCtxtSwtch( tcbPtr))
		{
			kobj->ownerPtr = tcbPtr;
			K_CR_EXIT
			return;
		}
		else
		{
			KFAULT( FAULT_READY_QUEUE);
		}
	}
	K_CR_EXIT
	return;
}
/* return mutex state - it checks for abnormal values */
K_ERR kMutexQuery( K_MUTEX *const kobj)
{
	K_CR_AREA

	K_CR_ENTER

	if (kobj == NULL)
	{
		K_CR_EXIT
		return (K_ERR_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		K_CR_EXIT
		return (K_ERR_OBJ_NOT_INIT);
	}
	if (kobj->lock == TRUE)
	{
		K_CR_EXIT
		return (K_QUERY_MUTEX_LOCKED);
	}
	if (kobj->lock == FALSE)
	{
		K_CR_EXIT
		return (K_QUERY_MUTEX_UNLOCKED);
	}
	K_CR_EXIT
	return (K_ERROR);
}

#endif /* mutex */

