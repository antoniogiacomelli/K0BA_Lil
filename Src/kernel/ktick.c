/*
 * ktick.c
 *
 *  Created on: Oct 21, 2024
 *      Author: anton
 */

#include <kapi.h>

TICK kTickGet(void)
{
	return runTime.globalTick;
}


static inline K_ERR kDecTimeSlice_(void);

static K_ERR kDecTimeSlice_(void)
{

	if (runPtr->status == RUNNING)
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
				runPtr->status=READY;
				return K_TASK_TSLICE_DUE;
			}
		}
	}
	return K_SUCCESS;
}
BOOL kTickHandler(void)
{
	BOOL  runToCompl = FALSE;
	BOOL  deferRet = FALSE;
	BOOL  timeSliceDueRet = FALSE;

	runTime.globalTick += 1U;
	if (runPtr->busyWaitTime > 0)
	{
		runPtr->busyWaitTime -= 1U;
	}
	if (runTime.globalTick == K_TICK_TYPE_MAX)
	{
		runTime.globalTick=0U;
		runTime.nWraps += 1U;
	}
	K_ERR retTimeSlice = kDecTimeSlice_();
	if (retTimeSlice == K_TASK_TSLICE_DUE)
	{
		timeSliceDueRet = TRUE;
	}
	if ((runPtr->runToCompl == TRUE) && (runPtr->status == RUNNING))
	{
		if (kTCBQEnq(&readyQueue[runPtr->priority], runPtr) == K_SUCCESS)
		{
			runPtr->status=READY;
		}
		runToCompl = TRUE;
	}

	if (dTimOneShotList || dTimReloadList)
	{
		kSignal(PID_TIMHANDLER);
		deferRet = TRUE;

	}
	BOOL ret = (timeSliceDueRet | runToCompl | deferRet);
	return (ret);
}
