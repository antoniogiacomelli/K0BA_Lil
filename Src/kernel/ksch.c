/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Scheduler routines
 *
 *****************************************************************************/

#define K_CODE
#include "ksys.h"

BOOL kSchNeedReschedule(K_TCB *newPtr)
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
	/*TODO: improve this bubble sort*/
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

	kSchFindTask_();
	K_TCB *nextRunPtr = NULL;
	kTCBQDeq(&readyQueue[nextTaskPrio], &nextRunPtr);
	if (nextRunPtr == NULL)
	{

		kErrHandler(FAULT_TCB_NULL);
	}
	nextTaskPrio = K_PRIO_TYPE_MAX; /*reset*/
	runPtr = nextRunPtr;
}

