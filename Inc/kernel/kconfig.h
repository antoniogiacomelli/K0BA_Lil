 /******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************/

/**
 * \file     kconfig.h
 * \brief    Kernel Configuration File
 * \version  0.1.0
 * \author   Antonio Giacomelli
 *
 * \details  In this header kernel services are configured.
 *
 ******************************************************************************/



#ifndef INC_K_CONFIG_H_
#define INC_K_CONFIG_H_

#ifndef __GNUC__
#   error "You need GCC as your compiler!"
#endif

#ifndef __KVERSION
#error "Missing kernel version."
#endif


#define TIM_ON 0

#define PID_IDLETASK    	 0   /**<  ID of the idle task */
#define PID_TIMHANDLER 		 1   /**< ID of the timer handler task */
#define PID_INVALID			 255 /**< Invalid PID */
#define N_USRTASKS    	  4	 /**<  Number of tasks */
#define STACKSIZE        128  /**<  Stack size in words for each task */
#define _N_SYSTASKS       2
#define NTHREADS		(N_USRTASKS + _N_SYSTASKS)
#define _NPRIO           5  /**< Number of priorities */
#define NPRIO            (_NPRIO + 1)
#define TICK_10MS        (SystemCoreClock/1000)  /**<  Tick period of 10ms */
#define TICK_5MS         (SystemCoreClock/2000)  /**< Tick period of 5ms */
#define TICK_1MS         (SystemCoreClock/10000) /**<  Tick period of 1ms */
#define K_CONF_TICK_PERIOD TICK_10MS /**< System tick period */
#define K_CONF_TRACE OFF
#if (K_CONF_TRACE)

	#define TRACEBUFF_SIZE	  512
	#define TRACE_MAX_CHAR 	  16
	#define TRACE_ONLY_EVENT  ON

	#if		(TRACE_ONLY_EVENT==ON)
			#define K_TRACE(event) kTrace(event, NULL)
	#else
			#define K_TRACE(event, info) kTrace(event, info)
	#endif
#endif


/**
*\brief Message Passing configuration
*/
#define K_CONF_MSG_QUEUE 			ON /**< Mesg Queue ON/OFF */
#define K_CONF_MAILBOX	 			ON /**< Mailbox ON/OFF */
#if (K_CONF_MAILBOX==ON)
	#define K_CONF_MAILBOX_ACK		ON /**< Mailbox extended rendez-vous */
#endif
#if ((K_CONF_MSG_QUEUE == ON))
	#define MSGBUFF_SIZE sizeof(K_MESGBUFF) /**< message buffer size 	  */
	#define N_MSGBUFF 10				    /**< number of message buffer */
#endif
#if (K_CONF_MSG_QUEUE == ON)
	#if (N_MSGBUFF <=0 )
	#	error "Number of message buffers must be greater than 0"
	#endif
#endif
/*
 * Condition Variables
 */
#define K_CONF_COND_VAR ON

/*  Pipes Configuration */
#define K_CONF_PIPES 	ON
#if (K_CONF_PIPES==ON)
    #define PIPE_SIZE    	 	 (35) /* pipe size (bytes) */
#endif

/*FIFO */
#define FIFO_SIZE				 (32)
/*
 *  Application Timer Configuration
 */
#define N_TIMERS 	6					   /* number of application timers */
#define TIMER_SIZE sizeof(K_TIMER)		   /* application timer size */

#endif /* K_CONFIG_H */
