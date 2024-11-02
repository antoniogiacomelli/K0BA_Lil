/*******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 *******************************************************************************
 *******************************************************************************
 * 	Module        : Nucleus
 * 	Sub-Module    : Tick Management
 * 	Provides to   : Scheduler, Application Timers
 * 	In this unit:
 *
 * 		o System Tick Management
 *
 *******************************************************************************/

#define K_CODE
#include "ksys.h"

TICK kTickGet(void)
{
	return runTime.globalTick;
}

static inline K_ERR kDecTimeSlice_(void);

static K_ERR kDecTimeSlice_(void)
{

	if ((runPtr->status == RUNNING) && (runPtr->runToCompl == FALSE))
	{
		if (runPtr->busyWaitTime > 0)
		{
			runPtr->busyWaitTime -= 1U;
		}

		if (runPtr->timeLeft > 0)
		{
			runPtr->timeLeft -= 1U;
		}
		if (runPtr->timeLeft == 0)
		{
			if (kTCBQEnq(&readyQueue[runPtr->priority], runPtr) == K_SUCCESS)
			{
				runPtr->timeLeft = runPtr->timeSlice;
				runPtr->status = READY;
				return K_TASK_TSLICE_DUE;
			}
		}
	}
	return K_SUCCESS;
}
BOOL kTickHandler(void)
{
	BOOL runToCompl = FALSE;
	BOOL deferRet = FALSE;
	BOOL timeSliceDueRet = FALSE;

	runTime.globalTick += 1U;
	if (runPtr->busyWaitTime > 0)
	{
		runPtr->busyWaitTime -= 1U;
	}
	if (runTime.globalTick == K_TICK_TYPE_MAX)
	{
		runTime.globalTick = 0U;
		runTime.nWraps += 1U;
	}
	K_ERR retTimeSlice = kDecTimeSlice_();

	/* a blocking task is running. it takes precedence all over others */
	if ((runPtr->status == RUNNING) && (runPtr->runToCompl == TRUE))
	{
		return FALSE;
	}
	if (retTimeSlice == K_TASK_TSLICE_DUE)
	{
		timeSliceDueRet = TRUE;
	}

	if (dTimOneShotList || dTimReloadList)
	{
		kSignal(TIMHANDLER_TID);
		deferRet = TRUE;

	}
	BOOL ret = (timeSliceDueRet | runToCompl | deferRet);
	return (ret);
}
