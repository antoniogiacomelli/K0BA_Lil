/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module    : Nucleus
 * 	Sub-module: Application Timers
 *
 * 	In this unit:
 * 			o Timer Pool Management
 *			o Timer Delta List
 *			o Timer Handler
 *			o Sleep delay
 *			o Busy-wait delay
 *
 *****************************************************************************/


#define K_CODE

#include "ksys.h"

K_BLOCKPOOL	 timerMem;
K_TIMER* 	 dTimReloadList=NULL; 		/**< periodic timers */
K_TIMER*	 dTimOneShotList=NULL;		/**< reload	  timers */
K_TIMER 	 timerPool[K_DEF_N_TIMERS];
K_SEMA  	 timerSemaCnt;


static K_ERR kTimerListAdd_(K_TIMER** dTimList, STRING timerName,\
		TICK tickCount,  CALLBACK funPtr, ADDR argsPtr, BOOL reload);



K_TIMER* kTimerGet(VOID)
{

	K_TIMER* retValPtr;
	kMutexLock(&(timerMem.poolMutex));
	retValPtr = (K_TIMER*)kBlockPoolAlloc(&timerMem);
	kMutexUnlock(&(timerMem.poolMutex));
	return retValPtr;
}

K_ERR kTimerPut(K_TIMER* const self)
{

	kMutexLock(&(timerMem.poolMutex));
	if (kBlockPoolFree(&timerMem, (ADDR)self) == 0)
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
		return K_ERROR;
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
		return K_SUCCESS;
	}
	K_TIMER* currListPtr = *selfPtr;
	K_TIMER* prevListPtr = NULL;

	/* traverse the delta list to find the correct position based on relative
    time*/
	while (currListPtr != NULL && currListPtr->dTicks
			< newTimerPtr->dTicks)
	{
		newTimerPtr->dTicks -= currListPtr->dTicks;
		prevListPtr = currListPtr;
		currListPtr = currListPtr->nextPtr;
	}
	/* insert new timer */
 	newTimerPtr->nextPtr = currListPtr;

 	/* adjust delta */
 	if (currListPtr != NULL)
 	{
		currListPtr->dTicks -= newTimerPtr->dTicks;
	}
	/* im the head, here */
	if (prevListPtr == NULL)
	{
		*selfPtr = newTimerPtr;
	}
	else
	{
		prevListPtr->nextPtr = newTimerPtr;
	}
	return K_SUCCESS;
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
			/* ... as long there is that tick, tick...*/
			expTimerPtr = dTimOneShotList;
			/* ... followed by that bump: */
			dTimOneShotList->funPtr(dTimOneShotList->argsPtr);
			kTimerPut(expTimerPtr);
			dTimOneShotList=dTimOneShotList->nextPtr;
		}
	}
	K_TIMER* putRelTimerPtr=NULL;
	while (dTimReloadList->dTicks == 0 && dTimReloadList)
	{

		putRelTimerPtr=dTimReloadList;
		kMemCpy(&timReloadCpy, dTimReloadList, TIMER_SIZE);
		kTimerPut(putRelTimerPtr);
		dTimReloadList->funPtr(dTimReloadList->argsPtr);
		dTimReloadList=dTimReloadList->nextPtr;
		K_TIMER* insTimPtr = &timReloadCpy;
		kTimerInit(insTimPtr->timerName, insTimPtr->ticks, insTimPtr->funPtr, \
				   insTimPtr->argsPtr, insTimPtr->reload);
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
void kSleepDelay(TICK ticks)
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

VOID kBusyDelay(TICK delay)
{
	if (runPtr->busyWaitTime == 0)
		runPtr->busyWaitTime = delay;

}
