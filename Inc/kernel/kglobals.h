/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o System Globals extern declaration
 *
 *****************************************************************************/

#ifndef K_GLOBALS_H
#define K_GLOBALS_H


extern K_TCB*            runPtr;              /* Pointer to the running TCB */
extern K_TCB             tcbs[NTHREADS];	  /* Pool of TCBs				*/
extern volatile K_FAULT  faultID;        	  /* Fault ID					*/
extern UINT32            idleStack[IDLE_STACKSIZE]; /* Stack for idle task 		*/
extern K_TCBQ 			 readyQueue[NPRIO];   /* table of ready queues	 	*/
extern K_TCBQ 			 sleepingQueue;		  /* table of SLEEPING queues  */
extern PID				 tidTbl[NTHREADS];    /* map pid -> tid */
extern K_TIMER* 	 	 dTimReloadList; 	  /* periodic timers		    */
extern K_TIMER*			 dTimOneShotList;	  /* reload	  timers	    	*/
extern volatile struct kRunTime runTime; 	  /* record of run time 	    */
extern PRIO highestPrio;		 	 		  /* highest initial prio	    */
extern const PRIO lowestPrio; 				  /* lowest prio allowed 	    */
extern PRIO nextTaskPrio;		 			  /* scheduled task priority    */
#ifndef K_DEF_VERSION
#define K_DEF_VERSION
extern struct kversion KVERSION; 			  /* Kernel version			    */
#endif

#endif /* INC_KGLOBALS_H_ */
