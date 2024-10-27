/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 		o Kernel initialisation routines: queues, pools, systick and scheduler
 * 										  start-up (via SVC #0)
 *
 *****************************************************************************/

#include <kapi.h>

static void kInitRunTime_(void);
static K_ERR kInitQueues_(void)
{
	K_ERR retVal=0;
	for (PRIO prio=0; prio<NPRIO; prio++)
	{
		retVal |= kTCBQInit(&readyQueue[prio], "ReadyQ");

	}
		retVal |= kTCBQInit(&sleepingQueue, "SleepQ");

	assert(retVal == 0);
	return retVal;
}

static void kInitRunTime_(void)
{
	runTime.globalTick = 0;
	runTime.nWraps = 0;
}

void kInitKernel(void)
{
	kInitQueues_();
	kInitRunTime_();
	kMesgBuffPoolInit();
	kTimerPoolInit();
	highestPrio=tcbs[0].priority;
	for (int i = 0; i<NTHREADS; i++)
	{
		if (tcbs[i].priority < highestPrio)
		{
			highestPrio = tcbs[i].priority;
		}
	}
	if ((lowestPrio-highestPrio) > NPRIO)
	{
		assert(0);
	}
	else
	{
		for(int i = 0; i<NTHREADS; i++)
		{
			kTCBQEnq(&readyQueue[tcbs[i].priority], &tcbs[i]);
		}
	}

	kReadyQDeq(&runPtr, highestPrio);
	__enable_irq();
	K_TRAP_SVC(0);
}

