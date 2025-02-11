/*******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 *******************************************************************************
 * ##Important Parameters##
 *
 * - **Number of User Tasks:**  (`K_DEF_N_USRTASKS`)
 *
 *    Number of user tasks.
 *
 * - **Lowest effective priority:**      (`K_DEF_N_MINPRIO`)
 *   Priorities range are from `0` to ``K_DEF_N_MINPRIO`.
 *   (0 is highest effective priority)
 *
 * - **Number of timers:**     (`K_DEF_N_TIMERS`)
 *   Minimal: Number of Tasks + 1
 *
 * - **Tick Period:**        (`K_DEF_TICK_PERIOD`)
 *   Pre-defined values are `TICK_1MS`, `TICK_5MS` and `TICK_10MS`.
 *   Users can define it, as they wish, by configuring SysTick.
 *   Recommended value is 5ms.
 *
 * - **Queue Discipline**: blocking mechanisms that can change the queue dis
 *   cipline are either by priority  (`K_DEF_ENQ_PRIO`) or FIFO (`K_DEF_ENQ_FIFO`).
 *   Default/fallback value is by priority.
 *
 ******************************************************************************/

 #ifndef KCONFIG_H
 #define KCONFIG_H
 #include "kinternals.h"
 
 #define ON     (1)
 #define OFF    (0)
 
 /* include headers for HAL and compiler in kenv.h */
 /* and set this macro to 1                        */
 #define CUSTOM_ENV (0)
 
 /**/
 /*** [ System Tasks Stack Size (WORDS)] ***************************************/
 /*
  * This configuration is exposed so the system programmer might adjust
  * the IdleTask stack size to support any hook. The Timer Handler stack size
  * must be adjusted to support Application Timers callouts.
  *
  * */
 #define IDLE_STACKSIZE      	    	(64)
 #define TIMHANDLER_STACKSIZE  			(64)
 
 /**/
 /*** [ Time Quantum ] *********************************************************/
 #define K_DEF_TICK_PERIOD               (TICK_5MS)
 
 /**/
 /*** [ Number of user-defined tasks ] *****************************************/
 #define K_DEF_N_USRTASKS    	        (3)
 
 /**/
 /*** [The lowest effective priority, that is the highest user-defined value]  */
 #define K_DEF_MIN_PRIO	           	    (1)
 
 /**/
 /*** [ Time-Slice Scheduling ]*************************************************/
 #define K_DEF_SCH_TSLICE			    (OFF)
 
 /**/
 /*** [ App Timers ] ***********************************************************/
 #define K_DEF_N_TIMERS                  (K_DEF_N_USRTASKS+1)
 
 /**/
 /*** [ Dynamic priority change ] **********************************************/
 /* Enables the methods kTaskChangePrio() and kTaskRestorePrio() to act on
  the calling task. */
 
 #define K_DEF_FUNC_DYNAMIC_PRIO		    (OFF)
 
 /**/
 /*** [ Direct Task Signal ] ***************************************************/
 /* Act as a private binary semaphore. If so, a late pend() will not fail
  if already signalled. It can be useful to synchronise two tasks directly. */
 #define K_DEF_TASK_SIGNAL_BIN_SEMA 	    (OFF)
 
 /**/
 /*** [ Semaphores ] ***********************************************************/
 #define K_DEF_SEMA                      (OFF)
 
 #if (K_DEF_SEMA==ON)
 #define K_DEF_SEMA_ENQ  		        (K_DEF_ENQ_PRIO)
 #endif
 
 /**/
 /*** [ Mutexes ] **************************************************************/
 #define K_DEF_MUTEX                     (OFF)
 /* Priority Inheritance */
 #define K_DEF_MUTEX_PRIO_INH			(OFF)
 #if (K_DEF_MUTEX==ON)
 /* Queue Discipline:				 */
 #define K_DEF_MUTEX_ENQ				    (K_DEF_ENQ_PRIO)
 #endif
 
 /**/
 /*** [ Sleep/Wake Events ] ****************************************************/
 #define K_DEF_SLEEPWAKE                  (OFF)
 
 /**/
 /*** [ Mailbox ] **************************************************************/
 
 #define K_DEF_MBOX	       	             (OFF)
 
 #if(K_DEF_MBOX==ON)
 
 /* Queue discipline:   				 */
 #define K_DEF_MBOX_ENQ       	    	(K_DEF_ENQ_PRIO)
 
 /* Optional Methdos*/
 #define K_DEF_FUNC_MBOX_POSTPEND    	(OFF)
 #define K_DEF_FUNC_MBOX_ISFULL			  (OFF)
 #define K_DEF_FUNC_MBOX_PEEK			    (OFF)
 #define K_DEF_FUNC_MBOX_POSTOVW			(OFF)
 
 #endif
 
 /**/
 /*** [ Multimailbox ] *********************************************************/
 
 #define K_DEF_MMBOX						(ON)
 
 #if(K_DEF_MMBOX==ON)
 
 /* Queue discipline:   				 */
 #define K_DEF_MMBOX_ENQ       	    	(K_DEF_ENQ_PRIO)
 
 /* Optional Methdos*/
 #define K_DEF_FUNC_MMBOX_ISFULL			(OFF)
 #define K_DEF_FUNC_MMBOX_PEEK			(OFF)
 #define K_DEF_FUNC_MMBOX_MAILCOUNT		(OFF)
 #define K_DEF_FUNC_MMBOX_JAM			(OFF)
 #endif
 
 /**/
 /*** [ Message Queue ] *******************************************************/
 
 #define K_DEF_MESGQ			      	    (OFF)
 
 #if (K_DEF_MESGQ == ON)
 /* Queue Discipline				 */
 #define K_DEF_MESGQ_ENQ				    (K_DEF_ENQ_PRIO)
 
 /* Optional methods */
 #define K_DEF_FUNC_MESGQ_JAM			 (OFF)
 #define K_DEF_FUNC_MESGQ_PEEK			 (OFF)
 #define K_DEF_FUNC_MESGQ_MESGCOUNT		 (OFF)
 #define K_DEF_FUNC_MESGQ_RESET			 (OFF)
 
 #endif /*mesgq*/
 
 /**/
 /*** [ Pump-Drop Queues ] *****************************************************/
 #define K_DEF_PDMESG                       (OFF)
 
 #endif /* KCONFIG_H */
 