/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module      : System Globals
 * 	Provides to : All services
 * 	Public API  : No
 *
 * 	In this unit:
 * 		o Kernel singleton objects: queues, tables, run-time record, scheduler
 * 		                            flags, etc.
 *
 *****************************************************************************/
#define K_CODE
#include "kglobals.h"
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
