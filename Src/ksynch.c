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
 * 					o Pend/Suspend/Signal/Resume
 *					o Sleep/Wake-Up (on Events)
 *					o Semaphores
 *					o Mutexes
 *
 *
 *****************************************************************************/

#define K_CODE

#include "kexecutive.h"

/*******************************************************************************
 * DIRECT TASK SIGNAL
 *******************************************************************************/
K_ERR kTaskPend( TICK timeout)
{
	K_ERR err;
	if (kIsISR())
		KFAULT( FAULT_ISR_INVALID_PRIMITVE);
	K_CR_AREA

	K_ENTER_CR
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
		K_EXIT_CR
		/* resuming here, if time is out, return error */
		K_ENTER_CR
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			err = K_ERR_TIMEOUT;
		}
		else
		{
			if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
				kRemoveTimeoutNode( &runPtr->taskHandlePtr->timeoutNode);
		}
	}
	K_EXIT_CR
	return (err);
}

K_ERR kTaskSignal( K_TASK *const taskHandlePtr)
{

	K_ERR err = -1;
	if ((taskHandlePtr == NULL) || (taskHandlePtr->tcbPtr->pid == runPtr->pid)
			|| (taskHandlePtr->tcbPtr->pid == TIMHANDLER_ID)
			|| (taskHandlePtr->tcbPtr->pid == IDLETASK_ID))
		return (err);
	K_CR_AREA
	K_ENTER_CR
	PID pid = taskHandlePtr->tcbPtr->pid;
	if (tcbs[pid].status == PENDING)
	{
		err = kReadyCtxtSwtch( &tcbs[pid]);
		kassert( !err);
	}
	else
	{
		taskHandlePtr->tcbPtr->signalled = TRUE;
		err = (K_SUCCESS);
	}
	K_EXIT_CR
	return (err);
}

/******************************************************************************
 * SLEEP/WAKE ON EVENTS
 ******************************************************************************/
#if (K_DEF_SLEEPWAKE==ON)
K_ERR kEventInit( K_EVENT *const kobj)
{

	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	K_CR_AREA
	K_ENTER_CR
	kassert( !kTCBQInit( &(kobj->waitingQueue), "eventQ"));
	kobj->init = TRUE;
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = EVENT;
	K_EXIT_CR
	return (K_SUCCESS);
}
K_ERR kEventSleep( K_EVENT *kobj, TICK timeout)
{
	K_CR_AREA
	K_ENTER_CR
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
	K_EXIT_CR
	/* resuming here, if time is out, return error */
	K_ENTER_CR
	if (runPtr->timeOut)
	{
		runPtr->timeOut = FALSE;
		K_EXIT_CR
		return (K_ERR_TIMEOUT);
	}
	else
	{
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);

	}
	K_EXIT_CR
	return (K_SUCCESS);
}

VOID kEventWake( K_EVENT *kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->waitingQueue.size == 0)
		return;
	K_CR_AREA
	K_ENTER_CR
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
	K_EXIT_CR
	return;
}

VOID kEventSignal( K_EVENT *kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	if (kobj->waitingQueue.size == 0)
		return;
	K_CR_AREA
	K_ENTER_CR
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	K_TCB *nextTCBPtr;
	kTCBQDeq( &kobj->waitingQueue, &nextTCBPtr);
	kassert( !kReadyCtxtSwtch( nextTCBPtr));
	K_EXIT_CR
	return;
}

UINT kEventQuery( K_EVENT *const kobj)
{
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
	}
	return (kobj->waitingQueue.size);
}

#if ((K_DEF_MUTEX==ON))
inline K_ERR kCondVarWait( K_EVENT *eventPtr, K_MUTEX *mutexPtr, TICK timeout)
{
	K_ERR err;
	K_CR_AREA
	K_ENTER_CR
	kMutexUnlock( mutexPtr);
	err = kEventSleep( eventPtr, timeout);
	K_EXIT_CR
	return (err);
}
#endif


#endif

#if (K_DEF_SEMA == ON)
/******************************************************************************
 * SEMAPHORES
 ******************************************************************************/
K_ERR kSemaInit( K_SEMA *const kobj, const INT value)
{
	K_CR_AREA
	K_ENTER_CR
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
		K_EXIT_CR
		return (K_ERR_OBJ_NULL);
	}
	if (value < 0)
		KFAULT( FAULT);
	kobj->value = value;
	if (kTCBQInit( &(kobj->waitingQueue), "semaQ") != K_SUCCESS)
	{
		K_EXIT_CR
		return (K_ERROR);
	}
	kobj->init = TRUE;
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = SEMAPHORE;
	K_EXIT_CR
	return (K_SUCCESS);
}

K_ERR kSemaWait( K_SEMA *const kobj, const TICK timeout)
{
	K_CR_AREA
	K_ENTER_CR
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
		K_EXIT_CR
		K_ENTER_CR
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			kobj->value += 1;
			K_EXIT_CR
			return (K_ERR_TIMEOUT);
		}
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	K_EXIT_CR
	return (K_SUCCESS);
}

VOID kSemaSignal( K_SEMA *const kobj)
{
	K_CR_AREA
	K_ENTER_CR
	if (kobj == NULL)
	{
		KFAULT( FAULT_NULL_OBJ);
		K_EXIT_CR
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
	K_EXIT_CR
	return;
}
#endif

#if (K_DEF_MUTEX == ON)
/*******************************************************************************
 * MUTEX
 *******************************************************************************/
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
	K_ENTER_CR
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
		K_EXIT_CR
		return (K_SUCCESS);
	}
	if ((kobj->ownerPtr != runPtr) && (kobj->ownerPtr != NULL))
	{
		if (timeout == 0)
		{
			K_EXIT_CR
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
		if ((timeout == 0))
			if ((timeout > 0) && (timeout < 0xFFFFFFFF))
				kTimeOut( &kobj->timeoutNode, timeout);
		runPtr->status = BLOCKED;
		K_PEND_CTXTSWTCH
		K_EXIT_CR
		K_ENTER_CR
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_EXIT_CR
			return (K_ERR_TIMEOUT);
		}
		else
		{
			if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
				kRemoveTimeoutNode( &kobj->timeoutNode);
		}
	}
	else
	{
		if (kobj->ownerPtr == runPtr)
		{ /* recursive lock ? why ? WHYYYYYYYYYYY*/
			K_EXIT_CR
			return (K_ERR_MUTEX_REC_LOCK);
		}
	}

	K_EXIT_CR
	return (K_SUCCESS);
}

VOID kMutexUnlock( K_MUTEX *const kobj)
{
	K_CR_AREA
	K_ENTER_CR
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
		K_EXIT_CR
		return;
	}
	/* runPtr is the owner and mutex was locked */
	if (kobj->waitingQueue.size == 0)
	{
		kobj->lock = FALSE;
#if (K_DEF_MUTEX_PRIO_INH==(ON))
		kobj->ownerPtr->priority = kobj->ownerPtr->realPrio;
#endif
		tcbPtr = kobj->ownerPtr;
		kobj->ownerPtr = NULL;
	}
	else
	{
		/*there are waiters, unblock a waiter set new mutex owner.
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
			K_EXIT_CR
			return;
		}
		else
		{
			KFAULT( FAULT_READY_QUEUE);
		}
	}
	K_EXIT_CR
	return;
}

K_ERR kMutexQuery( K_MUTEX *const kobj)
{
	K_CR_AREA

	K_ENTER_CR

	if (kobj == NULL)
	{
		K_EXIT_CR
		return (K_ERR_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		K_EXIT_CR
		return (K_ERR_OBJ_NOT_INIT);
	}
	if (kobj->lock == TRUE)
	{
		K_EXIT_CR
		return (K_QUERY_MUTEX_LOCKED);
	}
	if (kobj->lock == FALSE)
	{
		K_EXIT_CR
		return (K_QUERY_MUTEX_UNLOCKED);
	}
	K_EXIT_CR
	return (K_ERROR);
}

#endif /* mutex */

