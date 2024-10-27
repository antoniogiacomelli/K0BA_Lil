/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o System Globals extern declaration
 *
 *****************************************************************************/

#ifndef INC_KGLOBALS_H_
#define INC_KGLOBALS_H_


extern K_TCB*            runPtr;              /* Pointer to the running TCB */
extern K_TCB             tcbs[NTHREADS];	  /* Pool of TCBs				*/
extern volatile K_FAULT  faultID;        	  /* Fault ID					*/
extern PID             uPid[NTHREADS];   	  /* User task IDs 				*/
extern UINT32           idleStack[STACKSIZE]; /* Stack for idle task 		*/
extern K_TCBQ 			 readyQueue[NPRIO];   /* table of ready queues	 	*/
extern K_TCBQ 			 sleepingQueue;		  /* table of suspended queues  */
extern TID 			 uPidTbl[NTHREADS];	      /* map user task id ->  pid   */
extern K_TIMER* 	 	 dTimReloadList; 	  /* periodic timers		    */
extern K_TIMER*			 dTimOneShotList;	  /* reload	  timers	    	*/
extern volatile struct kRunTime runTime; 	  /* record of run time 	    */
extern PRIO highestPrio;		 	 		  /* highest initial prio	    */
extern const PRIO lowestPrio; 				  /* lowest prio allowed 	    */
extern PRIO nextTaskPrio;		 			  /* scheduled task priority    */
extern struct kversion KVERSION; 			  /* Kernel version			    */

#endif /* INC_KGLOBALS_H_ */
