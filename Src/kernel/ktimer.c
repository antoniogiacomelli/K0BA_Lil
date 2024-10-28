/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 			o Timer Pool Management
 *			o Timer Delta List
 *			o Timer Handler
 *			o Sleep delay
 *			o Busy-wait delay
 *
 *****************************************************************************/


#include <kapi.h>
#include <ksystasks.h>

K_MEM 		 timerMem;
K_TIMER* 	 dTimReloadList=NULL; 		/**< periodic timers */
K_TIMER*	 dTimOneShotList=NULL;		/**< reload	  timers */
K_TIMER 	 timerPool[N_TIMERS];
K_SEMA  	 timerSemaCnt;


static K_ERR kTimerListAdd_(K_TIMER** dTimList, STRING timerName,\
		TICK tickCount,  CALLBACK funPtr, ADDR argsPtr, BOOL reload);


K_ERR kTimerPoolInit(VOID)
{

	kSemaInit(&timerSemaCnt, N_TIMERS);
	return kMemInit(&timerMem, timerPool, TIMER_SIZE, N_TIMERS);
}
K_TIMER* kTimerGet(VOID)
{

	K_TIMER* retValPtr;
	kMutexLock(&(timerMem.poolMutex));
	retValPtr = (K_TIMER*)kMemAlloc(&timerMem);
	kMutexUnlock(&(timerMem.poolMutex));
	return retValPtr;
}

K_ERR kTimerPut(K_TIMER* const self)
{

	kMutexLock(&(timerMem.poolMutex));
	if (kMemFree(&timerMem, (ADDR)self) == 0)
	{
		kMutexUnlock(&(timerMem.poolMutex));
		return K_SUCCESS;
	}
	kErrHandler(FAULT_POOL_PUT);
	return K_ERROR;
}

K_ERR kTimerInit(STRING timerName, TICK ticks, CALLBACK funPtr, ADDR argsPtr,
				 BOOL reload)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (reload==TRUE)
	{
		assert(kTimerListAdd_(&dTimReloadList, timerName, ticks, funPtr, \
				argsPtr, reload)==K_SUCCESS);
		K_EXIT_CR;
		return K_SUCCESS;
	}
	else
	{
		assert(kTimerListAdd_(&dTimOneShotList, timerName, ticks, funPtr, \
				argsPtr, reload)==K_SUCCESS);
		K_EXIT_CR;
		return K_ERROR;
	}
}

static K_ERR kTimerListAdd_(K_TIMER** selfPtr, STRING timerName, TICK ticks,
				   	   	    CALLBACK funPtr, ADDR argsPtr, BOOL reload)
{

	K_TIMER* newTimerPtr = kTimerGet();
	if (newTimerPtr == NULL)
	{
		return K_ERR_TIMER_GET;
	}
	newTimerPtr->timerName = timerName;
	newTimerPtr->dTicks = ticks;
	newTimerPtr->ticks = ticks;
	newTimerPtr->funPtr = funPtr;
	newTimerPtr->argsPtr = argsPtr;
	newTimerPtr->reload = reload;

	if (*selfPtr == NULL)
	{

		*selfPtr = newTimerPtr;
		return K_TIMER_SUCCESS;
	}
	K_TIMER* currListPtr = *selfPtr;
	K_TIMER* prevListPtr = NULL;

	/* Traverse the delta list to find the correct position based on relative
    time*/
	while (currListPtr != NULL && currListPtr->dTicks
			< newTimerPtr->dTicks)
	{
		// Adjust the relative time
		newTimerPtr->dTicks -= currListPtr->dTicks;
		prevListPtr = currListPtr;
		currListPtr = currListPtr->nextPtr;
	}

	// Insert the new timer at the correct position
	newTimerPtr->nextPtr = currListPtr;

	// Adjust the remaining time of the next timer (if any)
	if (currListPtr != NULL) {
		currListPtr->dTicks -= newTimerPtr->dTicks;
	}

	// If the new timer is at the head, update the head pointer
	if (prevListPtr == NULL)
	{
		*selfPtr = newTimerPtr;
	}
	else
	{
		prevListPtr->nextPtr = newTimerPtr;
	}
	return K_TIMER_SUCCESS;
}
static K_TIMER timReloadCpy={0};
void kTimerHandler(void)
{



	if (dTimOneShotList->dTicks > 0)
		dTimOneShotList->dTicks--;
	if (dTimReloadList->dTicks > 0)
		dTimReloadList->dTicks--;
	if (dTimOneShotList->dTicks == 0)
	{
		K_TIMER* expTimerPtr = dTimOneShotList;
		while (dTimOneShotList != NULL && dTimOneShotList->dTicks == 0)
		{
			K_CR_AREA;
			K_ENTER_CR;
			expTimerPtr = dTimOneShotList;
			dTimOneShotList->funPtr(dTimOneShotList->argsPtr);
			kTimerPut(expTimerPtr);
			dTimOneShotList=dTimOneShotList->nextPtr;
			K_EXIT_CR;
		}
	}
	K_TIMER* putRelTimerPtr=NULL;
	while (dTimReloadList->dTicks == 0 && dTimReloadList)
	{
		K_CR_AREA;
		K_ENTER_CR;
		putRelTimerPtr=dTimReloadList;
		kMemCpy(&timReloadCpy, dTimReloadList, TIMER_SIZE);
		kTimerPut(putRelTimerPtr);
		dTimReloadList->funPtr(dTimReloadList->argsPtr);
		dTimReloadList=dTimReloadList->nextPtr;
		K_TIMER* insTimPtr = &timReloadCpy;
		kTimerInit(insTimPtr->timerName, insTimPtr->ticks, insTimPtr->funPtr, \
				   insTimPtr->argsPtr, insTimPtr->reload);
		K_EXIT_CR;
		if (dTimReloadList==NULL)
		{
				dTimReloadList=&timReloadCpy;
				break;
		}

	}

	return;
}

/*******************************************************************************
 *
 * SLEEP TIMER AND BUSY-WAIT-DELAY
 *
 ******************************************************************************/
static void SleepTimerCbk_(ADDR args)
{
	UNUSED(args);

	K_TCB* tcbToWakePtr=NULL;
	tcbToWakePtr = (K_TCB*)(dTimOneShotList->argsPtr);
	assert(tcbToWakePtr != NULL);
	assert(!kTCBQRem(&sleepingQueue, &tcbToWakePtr));
	assert(tcbToWakePtr != NULL);
	assert(!kTCBQEnq(&readyQueue[tcbToWakePtr->priority], tcbToWakePtr));
	tcbToWakePtr->status=READY;
}
K_TCB* sleepTcbPtr = 0;
void kSleepDelay(TICK const ticks)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (runPtr->status == RUNNING)
	{
		sleepTcbPtr=runPtr;
		if (!kTimerListAdd_(&dTimOneShotList, "SleepTimer", ticks, \
				SleepTimerCbk_, (K_TCB*)runPtr, ONESHOT))
		{

			if (!kTCBQEnq(&sleepingQueue, runPtr))
			{
				runPtr->status = SLEEPING;
				runPtr->pendingObj=(K_TIMER*)(dTimOneShotList);
				K_PEND_CTXTSWTCH;
				K_EXIT_CR;
				return;
			}
		}
	}
	K_EXIT_CR;
	return;
}

VOID kBusyDelay(TICK const delay)
{
	if (runPtr->busyWaitTime == 0)
		runPtr->busyWaitTime = delay;

}
