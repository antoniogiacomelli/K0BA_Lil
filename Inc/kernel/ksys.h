/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 		o System Globals (singletons) and main dependencies
 *
 *****************************************************************************/

#ifndef KSYS_H
#define KSYS_H
/*******************************/
/*ENVIRONMENT DEPENDENCIES HERE*/
/*******************************/
#include "kmacros.h"
#include "kapi.h"
#include "kerr.h"
#include "klist.h"
#include "ksch.h"
#include "ksystasks.h"
#include "kversion.h"

extern K_TCB *runPtr; /* Pointer to the running TCB */
extern K_TCB tcbs[NTHREADS]; /* Pool of TCBs */
extern volatile K_FAULT faultID; /* Fault ID */
extern UINT32 idleStack[IDLE_STACKSIZE]; /* Stack for idle task */
extern UINT32 timerHandlerStack[TIMHANDLER_STACKSIZE];
extern K_BLOCKPOOL mesgBuffMem; /* global mesg pool control block */
extern K_MESGBUFF mesgBuffPool[K_DEF_N_MESGBUFF]; /* global mesg pool */
extern K_SEMA semaMesgCntr; /*counter sema for mesg pool */
extern K_BLOCKPOOL timerMem; /*global timer mem control block*/
extern K_TIMER timerPool[K_DEF_N_TIMERS]; /*global timer pool*/
extern K_SEMA timerSemaCnt; /*counter sema for timer pool*/
extern K_TCBQ readyQueue[NPRIO]; /* Table of ready queues */
extern K_TCBQ sleepingQueue; /* Queue for tasks that suspended
 themselves */
extern PID tidTbl[NTHREADS]; /* map pid -> tid */
extern K_TIMER *dTimReloadList; /* periodic timers */
extern K_TIMER *dTimOneShotList; /* reload timers */
extern volatile struct kRunTime runTime; /* record of run time */
extern PRIO highestPrio; /* highest initial prio */
extern const PRIO lowestPrio; /* lowest prio allowed */
extern PRIO nextTaskPrio; /* scheduled task priority  */
#ifndef K_DEF_VERSION
#define K_DEF_VERSION
extern struct kversion KVERSION; /* Kernel version */
#endif

#endif /* KSYS_H */
