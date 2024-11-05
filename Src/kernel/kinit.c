/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module      : System Initialisation
 *  Depends on  : All
 *  Provides to : All
 *  Public API  : No
 *
 * 	In this unit:
 * 		o Misc generic utils
 *
 ******************************************************************************/
#include "kglobals.h"
/******************************************************************************
* KERNEL INITIALISATION
*******************************************************************************/
static VOID kMesgBuffPoolInit_(VOID)
{
    kBlockPoolInit(&mesgBuffMem, mesgBuffPool, MSGBUFF_SIZE, K_DEF_N_MESGBUFF);
	kSemaInit(&semaMesgCntr, K_DEF_N_MESGBUFF);
}
static void kTimerPoolInit_(VOID)
{

	kSemaInit(&timerSemaCnt, K_DEF_N_TIMERS);
	kBlockPoolInit(&timerMem, (BYTE*)timerPool, TIMER_SIZE, K_DEF_N_TIMERS);
}
static void kInitRunTime_(void)
{
	runTime.globalTick = 0;
	runTime.nWraps = 0;
}
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

volatile UINT32 version;
void kInit(void)
{

	version = kGetVersion();
	if (version != K_VALID_VERSION)
		kErrHandler(FAULT_KERNEL_VERSION);
	kInitQueues_();
	kInitRunTime_();
	kMesgBuffPoolInit_();
	kTimerPoolInit_();
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
	K_START_APPLICATION;
}

