/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o System Globals
 *
 *****************************************************************************/

#include <kapi.h>

#ifndef __SYSTEMSINGLETONS
#define __SYSTEMSINGLETONS
/*******************************************************************************
 *
 * 					 			SYSTEM GLOBALS
 *
 ******************************************************************************/
#ifdef __KVERSION
struct kversion KVERSION = {0, 1, 0}; /*<* Kernel version */
#endif
K_TCBQ readyQueue[NPRIO]; 	     /*<* table of ready queues 	*/
K_TCBQ sleepingQueue; 	 /*<* table of suspended queues */
K_TCB* runPtr;		         	 /*<* pointer to the running TCB */
K_TCB  tcbs[NTHREADS];		 	 /*<* pool of tcbs */
TID uPidTbl[NTHREADS];	     /*<* map user task id ->  pid  */
volatile K_FAULT faultID=0;	     /*<* fatal error id */
PRIO highestPrio=0;		 	 /*<* highest initial prio */
const PRIO lowestPrio=NPRIO-1; /*<* lowest prio allowed */
PRIO nextTaskPrio = 0;		 /*<* scheduled task priority */
volatile struct kRunTime runTime; /*<* record of run time 	 	*/


#endif //__SYSTEMSINGLETONS
