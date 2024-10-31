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
 ******************************************************************************/



#ifndef K_CONFIG_H
#define K_CONFIG_H


#define ON     1
#define OFF    0

#define K_DEF_PID_IDLETASK  	  	 0      /**<  ID of the idle task */
#define K_DEF_PID_TIMHANDLER 		 1      /**< ID of the timer handler task */
#define K_DEF_PID_INVALID			 255    /**< Invalid PID */
#define K_DEF_N_USRTASKS    	     4	    /**<  Number of tasks */
#define K_DEF_N_PRIO	           	 5      /**< Number of user task different priorities */
#define K_DEF_TICK_PERIOD 	 TICK_10MS  	/**< System tick period */
#define K_DEF_FIFO_SIZE				 32		/**< FIFO size */
#define K_DEF_N_TIMERS				  6 	/**< Number of system timers */
#define K_DEF_ERRHANDLER			 ON 	/**< Kernel will stop on faults */
#define K_DEF_PRIOINV_FAULT			 ON 	/**< Treat priority inversion as a fault */

/**
 * \brief These macros are to turn features ON/OFF and some configurations
 */

#define K_DEF_MEMBLOCK_ALIGN_4 	ON	  /**< Make memory blocks aligned to 4 */


#define K_DEF_BYTEPOOL			ON    /**< K_BYTEPOOL ON/OFF */

/**
*\brief Message Passing configuration
*/
#define K_DEF_MESGQ 			ON     /**< Mesg Queue ON/OFF */
#define K_DEF_MAILBOX	 		ON     /**< Mailbox ON/OFF */


#if (K_DEF_MAILBOX==ON)
	#define K_DEF_MAIL_SIZE			 4	  /**< Max size of a mail in mailbox */
	#define K_DEF_MAILBOX_ACK		ON    /**< Fully synchornous mailbox 	 */
#endif

#if ((K_DEF_MESGQ == ON))
	#define K_DEF_N_MESGBUFF 10			  /**< Global message buffers number */
#endif

#define K_DEF_COND ON				  /**< Condition Variables ON/OFF */

#if (K_DEF_COND == ON)				  /**< you need condition variables for pipes */
	#define K_DEF_PIPE		   	  ON  /**< Pipes ON/OFF */

/*  Pipes Configuration */
	#if (K_DEF_PIPE==ON)
		#define K_DEF_PIPE_SIZE   (35) /**< Pipe size (bytes) */
	#endif
#endif

#define K_DEF_TRACE 		 ON				/**< Trace ON/OFF      */
#if (K_DEF_TRACE==ON)
	#define K_DEF_TRACEBUFF_SIZE	  64   /**< Trace Buffer size*/
	#define K_DEF_TRACE_MAX_CHAR 	  16	/**< Max info bytes  */
	#define K_DEF_TRACE_NO_INFO       ON	/**< No custom info on trace buffer */
#endif


#endif /* K_CONFIG_H */
