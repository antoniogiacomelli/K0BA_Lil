/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************/

/**
 * \file     kconfig.h
 * \brief    Kernel Configuration File
 * \version  1.1.0
 * \author   Antonio Giacomelli
 *
 * \details  Kernel configuration definitions.
 *
 * **Inter-task Synchronisation**
 *
 * - Semaphores and Mutexes are alaways ON.
 * - Condition Variables and Sleep/Wake-Up on Events can be turned OFF, as they
 *   might be redundant depending on the use.
 *
 * **Inter-task Communication:**
 *
 * - Mailboxes and FIFOs are always on.
 * - Message Queues can be turned ON/OFF.
 * - Extended PIPEs can be turned ON/FF.
 * - Note Extended PIPEs depend on Condition Vars.
 *
 * **Memory Management**
 *
 * - Block Pools are always on as they are used by other services.
 * - Byte Pools can be turned ON/OFF.
 *
 *
 * **Important Parameters**
 *
 * - Number of User Tasks:  (K_DEF_N_USRTASKS)
 *                      Number of tasks your application will have.
 *                      Any custom deferred handlers shall be included.
 *                     
 * - Maximum priority:  (K_DEF_N_MINPRIO)
 *                     Priorities range are from 0 to K_DEF_N_MINPRIO.
 *                     Note, the lower the number the highest the effective
 *                     priority. (0 is highest effective priority)
 *                     Besides, the IDLE TASK is a system task with a priority
 *                     number one unit above the maximum user-defined number,
 *                     so it will always be lowest-priority task.
 *
 * - Number of timers: (K_DEF_N_TIMERS)
 *                     Consider using the number of tasks that will rely on timers
 *                     +1. This a safe configuration. Remember kSleepDelay() uses
 *                     timers.
 * 
 *  
 * - Tick Period: Pre-defined values are TICK_1MS, TICK_5MS and TICK_10MS.
 *                Users can define it, as they wish, by configuring SysTick.
 *                Recommended value is 5ms.
 * 
 ******************************************************************************/

#ifndef KCONFIG_H
#define KCONFIG_H

#define ON     1
#define OFF    0

#define K_DEF_N_USRTASKS    	     4 /**<  Number of tasks */
#define K_DEF_MIN_PRIO	           	 5 /**< Number of user task different priorities */
#define K_DEF_TICK_PERIOD 	 		 TICK_5MS  	/**< System tick period */
#define K_DEF_FIFO_SIZE				 32		/**< FIFO size */
#define K_DEF_N_TIMERS				  6 	/**< Number of system timers */
#define K_DEF_ERRHANDLER			 ON 	/**< Kernel will stop on faults */

/**
 * \brief These macros are to turn features ON/OFF and some configurations
 */

#define K_DEF_MEMBLOCK_ALIGN_4 	ON	  /**< Make memory blocks aligned to 4 */

#define K_DEF_BYTEPOOL			OFF    /**< K_BYTEPOOL ON/OFF */

/**
 *\brief Message Passing configuration
 */
#define K_DEF_MESGQ 			ON     /**< Mesg Queue ON/OFF */
#define K_DEF_MAILBOX	 		ON     /**< Mailbox ON/OFF */

#if (K_DEF_MAILBOX==ON)
#define K_DEF_MAIL_SIZE			 4	  /**< Max size of a mail in mailbox */
#endif

#if ((K_DEF_MESGQ == ON))
#define K_DEF_N_MESGBUFF 10		  /**< Global message buffers number */
#endif

#define K_DEF_SLEEPWAKE		  ON   /**<Sleep/Wake-up on Events ON/OFF    */
#define K_DEF_COND 			  ON   /**< Condition Variables ON/OFF */

#if (K_DEF_COND == ON)			  /**< you need condition variables for pipes */
#define K_DEF_PIPE		   	  ON  /**< Pipes ON/OFF */

/*  Pipes Configuration */
#if (K_DEF_PIPE==ON)
#define K_DEF_PIPE_SIZE   (35) /**< Pipe size (bytes) */
#endif
#endif

#define K_DEF_TRACE 		 ON			/**< Trace ON/OFF      */
#if (K_DEF_TRACE==ON)
#define K_DEF_TRACEBUFF_SIZE	  64   /**< Trace Buffer size*/
#define K_DEF_TRACE_MAX_CHAR 	  16	/**< Max info bytes  */
#define K_DEF_TRACE_NO_INFO       ON	/**< No custom info on trace buffer */
#endif

#endif /* KCONFIG_H */
