/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Scheduler routines
 * 					o Critical Regions Enter/Exit
 *
 *****************************************************************************/
#include <kapi.h>
BOOL kSchNeedReschedule(K_TCB* newPtr)
{

	if (newPtr->priority < runPtr->priority)
	{
		return TRUE;
	}
	return FALSE;
}

static inline void kSchFindTask_(void)
{
	PRIO prio = K_PRIO_TYPE_MAX;
	for (prio = highestPrio; prio < NPRIO; prio++)
	{

		if (readyQueue[prio].size > 0)
		{
			nextTaskPrio = prio;
			break;
		}
	}

	return;
}
void kSchSwtch(void)
{
	if (runPtr->status==RUNNING)
	{
		if (runPtr->pid != PID_IDLETASK)
		{
			if (!kTCBQEnq(&readyQueue[runPtr->priority], runPtr))
			{
				runPtr->status=READY;
			}
		}
		else
		{
			runPtr->status=READY;
		}
	}
	kSchFindTask_();
	K_TCB* nextRunPtr = NULL;
	kTCBQDeq(&readyQueue[nextTaskPrio], &nextRunPtr);
	if (nextRunPtr == NULL)
	{

		kErrHandler(FAULT_TCB_NULL);
	}
	//assert(nextRunPtr->status==READY);
	nextTaskPrio=K_PRIO_TYPE_MAX; /*reset*/
	runPtr = nextRunPtr;
}



