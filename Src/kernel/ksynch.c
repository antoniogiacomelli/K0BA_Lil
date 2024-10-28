/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Direct Task Pend/Signal
 *					o Generic Events
 *					o Sleep/Wake-Up (on Events)
 *					o Semaphores
 *					o Mutexes with priority inheritance
 *					o Condition Variables
 *
 *****************************************************************************/

#include <kapi.h>

/******************************************************************************
 * DIRECT TASK PENDING/SIGNAL
 *****************************************************************************/


void kPend(void)
{
	K_CR_AREA;
	K_ENTER_CR;
	K_ERR ret = kTCBQEnq(&sleepingQueue, runPtr);
	if (!ret)
	{
		runPtr->status=SUSPENDED;
		K_PEND_CTXTSWTCH;
		K_EXIT_CR;
	}
	else
	{
		assert(0);

	}

}

void kSignal(PID const taskID)
{

	K_CR_AREA;
	K_ENTER_CR;
	PID pid = kGetTaskPID(taskID);
	if (tcbs[pid].status == SUSPENDED)
	{
		K_TCB* tcbGotPtr=&tcbs[pid];
		kTCBQRem(&sleepingQueue, &tcbGotPtr);
		kTCBQEnq(&readyQueue[tcbGotPtr->priority], tcbGotPtr);
		if (READY_HIGHER_PRIO(tcbGotPtr))
		{
					K_PEND_CTXTSWTCH;
		}
	}
	K_EXIT_CR;
	return;
}


/**
 *******************************************************************************
 * SLEEP/WAKE ON EVENTS
 ******************************************************************************/
static K_ERR kEventInit_(K_EVENT* const self)
{

	if(IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERROR;
	}
	self->eventID=(UINT32)self;
	assert(!kTCBQInit(&(self->queue), "eventQ"));
	self->init=TRUE;
	return K_SUCCESS;
}

K_ERR kSleep(K_EVENT* self)
{
	K_CR_AREA;
	K_ENTER_CR;

	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERROR;
	}
	if (self->init==FALSE)
	{
		kEventInit_(self);
	}
	if(self->init == TRUE)
	{
		if (runPtr->status==RUNNING)
		{

			if (!kTCBQEnq(&(self->queue), runPtr))
			{
				runPtr->status=SLEEPING;
				runPtr->pendingObj = (K_EVENT*)self;
				K_TRAP_PENDSV;
				K_EXIT_CR;
				return K_SUCCESS;
			}
		}
	}
	K_EXIT_CR;
	return K_ERROR;
}

K_ERR kWake(K_EVENT* self)
{
	if(IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERR_NULL_OBJ;
	}
	K_CR_AREA;
	K_ENTER_CR;

	BOOL preempt=FALSE;
	SIZE sleepThreads=self->queue.size;
	if (sleepThreads>0)
	{
		for(SIZE i=0;i<sleepThreads;++i)
		{
			K_TCB* nextTCBPtr;
			kTCBQDeq(&self->queue, &nextTCBPtr);
			if(!kTCBQEnq(&readyQueue[nextTCBPtr->priority], nextTCBPtr))
			{

				if (!preempt)
				{
					if (runPtr->priority > nextTCBPtr->priority)
					{
						preempt=TRUE;
					}
				}
			}
		}
		if(preempt)
			K_PEND_CTXTSWTCH;
		K_EXIT_CR;
		return K_SUCCESS;
	}
	return K_ERROR;
}


/**
 ******************************************************************************
 * SEMAPHORES
 ******************************************************************************/

K_ERR kSemaInit(K_SEMA* const self, INT32 value)
{
	K_CR_AREA;
	K_ENTER_CR;

	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERR_NULL_OBJ;
	}
	self->value = value;
	if (kTCBQInit(&(self->queue), "semaQ") != K_SUCCESS)
	{
		kErrHandler(FAULT_LIST);
		K_EXIT_CR;
		return K_ERROR;
	}
	K_EXIT_CR;
	return K_SUCCESS;
}

K_ERR kSemaWait(K_SEMA* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERR_NULL_OBJ;
	}
	(self->value) = (self->value) - 1;
	if ((self->value) < 0)
	{
		if (!kTCBQEnq(&(self->queue), runPtr))
		{
			runPtr->status = BLOCKED;
			runPtr->pendingObj = (K_SEMA*)self;
			K_EXIT_CR;
			K_PEND_CTXTSWTCH;
			return K_SUCCESS;
		}
		kErrHandler(FAULT_LIST);
		K_EXIT_CR;
		return K_ERROR;
	}
	K_EXIT_CR;
	return K_SUCCESS;
}


K_ERR kSemaSignal(K_SEMA* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERROR;
	}
	K_TCB* nextTCBPtr=NULL;
	(self->value) = (self->value) + 1;
	if ((self->value) <= 0)
	{
		kTCBQDeq(&(self->queue), &nextTCBPtr);
		if (IS_NULL_PTR(nextTCBPtr))
		{
			kErrHandler(FAULT_NULL_OBJ);
			K_EXIT_CR;
			return K_ERROR;
		}
		nextTCBPtr->pendingObj=NULL;
		if (!kReadyQEnq(nextTCBPtr))
		{
			K_EXIT_CR;
			return K_SUCCESS;
		}
		K_EXIT_CR;
		kErrHandler(FAULT_READY_QUEUE);
		return K_ERROR;
	}
	K_EXIT_CR;
	return K_SUCCESS;
}
/******************************************************************************
 * MUTEX
 *****************************************************************************/
K_ERR kMutexInit(K_MUTEX* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERROR;
	}
	self->lock = FALSE;
	if (kTCBQInit(&(self->queue), "mutexQ") != K_SUCCESS)
	{
		K_EXIT_CR;
		kErrHandler(FAULT_LIST);
		return K_ERROR;
	}
	K_EXIT_CR;
	return K_SUCCESS;
}
K_ERR kMutexLock(K_MUTEX* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERR_NULL_OBJ;
	}
	if (self->lock == FALSE)
	{
		/* lock mutex and set the owner */
		self->lock = TRUE;
		self->ownerPtr = runPtr;
		K_EXIT_CR;
		return K_SUCCESS;
	}

		if ((self->ownerPtr != runPtr) && (self->ownerPtr != NULL))
		{
			if (self->ownerPtr->priority > runPtr->priority)
			{

				/* mutex owner has lower priority than the tried-to-lock-task
				 * thus, we boost owner priority, to avoid an intermediate
				 * priority task that does not need lock to preempt
				 * this task, causing an unbounded delay */

				self->ownerPtr->priority = runPtr->priority;

			}
			runPtr->status = BLOCKED;
			runPtr->pendingObj = (K_MUTEX*)self;
			kTCBQEnq(&(self->queue), runPtr);
			K_EXIT_CR;
			K_PEND_CTXTSWTCH;
		}

	K_EXIT_CR;
	return K_SUCCESS;
}

K_ERR kMutexUnlock(K_MUTEX* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
	K_TCB* tcbPtr;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERR_NULL_OBJ;
	}
	if ((self->lock == FALSE))
	{
		K_EXIT_CR;
		return K_ERR_MUTEX_NOT_LOCKED;
	}
	if (self->ownerPtr != runPtr)
	{
		K_EXIT_CR;
		return K_ERR_MUTEX_NOT_OWNER;
	}
	/* runPtr is the owner and mutex was locked */
	if (self->queue.size == 0) //no waiters
	{
			self->lock = FALSE;
			self->ownerPtr->priority=self->ownerPtr->realPrio;
			self->ownerPtr->pendingObj = NULL;
			tcbPtr=self->ownerPtr;
			self->ownerPtr = NULL;
	}
	else
	{
		/*there are waiters, unblock a waiter set new mutex owner.
		 * mutex is still locked */
			kTCBQDeq(&(self->queue), &tcbPtr);
			if (IS_NULL_PTR(tcbPtr))
				kErrHandler(FAULT_TCB_NULL);
			self->ownerPtr = tcbPtr;
			tcbPtr->status=READY;
			tcbPtr->pendingObj=NULL;
			kTCBQEnq(&readyQueue[tcbPtr->priority], tcbPtr);
			if (runPtr->priority < runPtr->realPrio)
			{
				runPtr->priority = runPtr->realPrio;
			}
			kTCBQEnq(&readyQueue[runPtr->priority], runPtr);
			runPtr->status=READY;
			K_PEND_CTXTSWTCH;
	}
	K_EXIT_CR;
	return K_SUCCESS;
}
/******************************************************************************
 * CONDITION VARIABLES
 ******************************************************************************/

VOID kCondInit(K_COND* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	kMutexInit(&self->condMutex);  /* Initialize mutex (unlocked/no ownerPtr) */
	kTCBQInit(&self->queue, "cvQ");
}

VOID kCondWait(K_COND* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	K_CR_AREA;
	K_ENTER_CR;
	kMutexUnlock(&self->condMutex);
	runPtr->status = SLEEPING;
	kTCBQEnq(&self->queue, runPtr);
	kYield();
	K_EXIT_CR;
	kMutexLock(&self->condMutex);
}

VOID kCondSignal(K_COND* const self)
{
	if (IS_NULL_PTR(self))
			kErrHandler(FAULT_NULL_OBJ);
	K_TCB* nextTCBPtr=NULL;
	kMutexLock(&self->condMutex);
	K_CR_AREA;
	K_ENTER_CR;
	kTCBQDeq(&self->queue, &nextTCBPtr);
	if (IS_NULL_PTR(nextTCBPtr))
		kErrHandler(FAULT_NULL_OBJ);
	nextTCBPtr->status = READY;
	kTCBQEnq(&readyQueue[nextTCBPtr->priority], nextTCBPtr);
	K_EXIT_CR;
	kMutexUnlock(&self->condMutex);
}

VOID kCondWake(K_COND* const self)
{
	if (IS_NULL_PTR(self))
			kErrHandler(FAULT_NULL_OBJ);
	kMutexLock(&self->condMutex);
	SIZE nThreads=self->queue.size;
	for (SIZE i = 0; i < nThreads; i++)
	{
			kCondSignal(self);
	}
	kMutexUnlock(&self->condMutex);
}


void kYield(void)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (runPtr->status == RUNNING)
	{ /* if yielded, not blocked, make it ready*/
		assert(!kReadyQEnq(runPtr));
	}
	K_EXIT_CR;
	K_PEND_CTXTSWTCH;
}
