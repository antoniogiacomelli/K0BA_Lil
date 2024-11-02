/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module      : Nucleus
 * 	Sub-Module  : N/A
 * 	Provides to : All services
 *
 * 	In this unit:
 * 		o Kernel singleton objects: queues, tables, run-time record, scheduler
 * 		                            flags, etc.
 * 		o Kernel initialisation routines: queues, pools, systick and scheduler
 * 										  start-up (via SVC #0)
 * 		o Error Handling
 *
 *
 *****************************************************************************/
#define K_CODE
#include "ksys.h"
/*******************************************************************************
* 					 			SYSTEM SINGLETONS
*******************************************************************************/

#ifndef K_SINGLETONS
#define K_SINGLETONS
K_TCBQ readyQueue[NPRIO];
K_TCBQ sleepingQueue;
K_TCB *runPtr;
K_TCB tcbs[NTHREADS];
PID tidTbl[NTHREADS];
volatile K_FAULT faultID = 0;
PRIO highestPrio = 0;
PRIO const lowestPrio = NPRIO -1;
PRIO nextTaskPrio = 0;
volatile struct kRunTime runTime;
#endif //K_SINGLETONS

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
		kErrHandler(FAULT_INVALID_KERNEL_VERSION);
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

/******************************************************************************
* ERROR HANDLING
*******************************************************************************/

#define ERR_HANDLER 1
void kErrHandler(K_FAULT fault) /* generic error handler */
{
#if (ERR_HANDLER==ON)
	faultID=fault;
	__disable_irq();
	while (1);
#else
	return;
#endif /*err handler*/

}

void kErrCheckPrioInversion(void)
{
	K_CR_AREA;
	K_ENTER_CR
	;
	K_TCB *runPtr_ = runPtr;
	assert(runPtr_->status == RUNNING);
	PRIO prioRun = runPtr_->priority;
	if (prioRun == 0)
	{
		K_EXIT_CR
		;
		return;
	}
	for (UINT32 i = 0; i < NTHREADS; i++)
	{
		if (tcbs[i].status == READY)
		{
			if (tcbs[i].priority < prioRun)
			{
#if (K_DEF_PRIOINV_FAULT == ON)
				kErrHandler(FAULT_PRIO_INV);
#endif
			}
		}
	}
	K_EXIT_CR
	;

	return;
}

