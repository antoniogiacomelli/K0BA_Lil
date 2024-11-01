/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o System Globals (singletons)
 *
 *****************************************************************************/
#define K_CODE

#include "ksys.h"
/*******************************************************************************
 *
 * 					 			SYSTEM GLOBALS
 *
 ******************************************************************************/

#ifndef K_SINGLETONS
#define K_SINGLETONS

struct kversion KVERSION =
{ 1, 1, 0 };
K_TCBQ readyQueue[NPRIO];
K_TCBQ sleepingQueue;
K_TCB *runPtr;
K_TCB tcbs[NTHREADS];
PID tidTbl[NTHREADS];

volatile K_FAULT faultID = 0;
PRIO highestPrio = 0;
const PRIO lowestPrio = NPRIO - 1;
PRIO nextTaskPrio = 0;
volatile struct kRunTime runTime;

#endif //K_SINGLETONS
