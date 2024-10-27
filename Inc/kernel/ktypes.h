/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************/

/**
 * \file     ktypes.h
 * \brief    Kernel Primitive Types and Typedefs
 * \version  0.1.0
 * \author   Antonio Giacomelli
 *
 *
 ******************************************************************************/


#ifndef K_TYPES_H
#define K_TYPES_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * \brief C standard library primitives alias
 */
#if (!defined(_STDDEF_H) && !defined(_STDDEF_H_) && !defined(_ANSI_STDDEF_H) \
     && !defined(__STDDEF_H__))
#error "You need a compiler with stddef.h library."
#else
typedef void        	VOID;
typedef char        	CHAR;
typedef unsigned char   BYTE;
typedef uint8_t    		BYTE;
typedef int8_t     		INT8;
typedef int32_t     	INT32;
typedef uint32_t    	UINT32;
typedef uint64_t    	UINT64;
typedef int64_t     	INT64;
typedef int16_t    		INT16;
typedef uint16_t   	    UINT16;
typedef size_t      	SIZE;
#endif

#if !defined(bool)
typedef unsigned int  BOOL
#define false (unsigned int)0U;
#define true  (unsigned int)1U;
#define bool
#endif
typedef _Bool       	BOOL;
#define TRUE			true
#define FALSE			false
/**
 * \brief Primitve aliases
 */

typedef VOID*             ADDR;     /**< Generic address type */
typedef const CHAR*       STRING;   /**< Read-only String alias   */
typedef BYTE              PID;      /**< System defined Task ID type */
typedef BYTE		      TID;	    /**< User defined Task ID */
typedef BYTE	          PRIO;     /**< Task priority type */
typedef UINT32         	  TICK;     /**< Tick count type */
typedef VOID (*TASKENTRY)(VOID);    /**< Task entry function pointer */
typedef VOID (*CALLBACK)(ADDR);	    /**< Event callout (timers, signals, etc.) */

/**
 *\brief Return values
 */
typedef enum kErr
{
    K_SUCCESS                     =  0,
    K_ERROR                       = -1,
    K_ERR_NULL_OBJ                = -2,
    K_TASK_TSLICE_DUE             =  1, /* Task time-slice due */
    K_TIMER_SUCCESS               =  K_SUCCESS,
    K_TIMER_EXPIRED               =  2,
    K_TIMER_NO_EXPIRED            =  K_SUCCESS,
    K_TIMER_POOL_EMPTY            =  4,
    K_ERR_TIMER_GET               = -3,
    K_LIST_SUCCESS                =  K_SUCCESS,
    K_ERR_LIST_FAIL               = -4,
    K_ERR_LIST_NULL_OBJ           = -5,
    K_ERR_LIST_ITEM_NOT_FOUND     = -6,
    K_ERR_LIST_EMPTY              = -7,
	K_ERR_MUTEX_NOT_LOCKED		  = -8,
	K_ERR_MUTEX_NOT_OWNER		  = -9,
	K_ERR_EVENT_NOT_INIT		  = -10,
} K_ERR;

/**
 *\brief Fault codes
 */
typedef enum kFault
{
    FAULT_TASK              = 1,
    FAULT_NOT_IDLE          = 2,
    FAULT_READY_QUEUE       = 3,
    FAULT_DTIMER_POOL_EMPTY = 4,
    FAULT_TCB_NULL          = 5,
    FAULT_LIST              = 6,
	FAULT_NULL_OBJ			= 7,
	FAULT_MEM_POOL_INIT		= 8,
	FAULT_INVALID_ARG		= 9,
	FAULT_TASK_STATUS		= 10,
	FAULT_POOL_PUT			= 11,
	FAULT_EVENT_NOT_INIT	= 12
} K_FAULT;

/**
 * \brief Task status
 */
typedef enum kTaskStatus
{
    INVALID = 0,
    READY, 		/**< On the Ready Queue, waiting to be dispatched */
    RUNNING,    /**< Running. CPU is taken. 					  */
    SUSPENDED,  /**< Suspended itself waiting for a direct signal */
	SLEEPING,   /**< Is sleeping waiting for a specific event
					(a timer, or a condition variable, etc.       */
	BLOCKED		/**< Blocked on semaphore or mutex 				  */
} K_TASK_STATUS;

#if (K_CONF_TRACE == ON)
typedef enum kTraceID
{
	NONE=0,
	TIMER_EN,
	TIMER_OUT,
	PREEMPTED,
	PRIORITY_BOOST,
	PRIORITY_RESTORED,
	INTERRUPT,
	TSIGNAL_RCVD,
	TSIGNAL_SENT,
	TPEND_SEMA,
	TPEND_MUTEX,
	TPEND_CVAR,
	TPEND_SLEEP,
	TWAKE,
	TPEND,
	MSG_RCVD,
	MSG_SENT,
	LIST_INSERT,
	LIST_REMOVED,
	REMOVE_EMPTY_LIST
}K_TRACEID;
#endif

/* Forward declarations */
typedef struct kTcb         K_TCB;       /**< Task control block */
typedef struct kTimer       K_TIMER;     /**< Application timer control block */
typedef struct kMem    		K_MEM; 	 	 /**< Memory partition control block */
typedef struct kList        K_LIST;      /**< Generic linked list */
typedef struct kListNode    K_LISTNODE;  /**< Linked list node */
typedef struct kSema		K_SEMA;		 /**< Semaphore */
typedef struct kMutex		K_MUTEX; 	 /**< Mutex		*/
typedef struct kMesgBuff	K_MESGBUFF;	 /**< Message Buffer for Mesg Queues  */
typedef struct kMesgQ		K_MESGQ;	 /**< Message Queue */
typedef struct kMailbox		K_MAILBOX; 	 /**< Mailbox */
typedef struct kCond     	K_COND;   	 /**< Condition variable */
typedef struct kFifo		K_FIFO;		 /**< Thread-safe Fifo */
typedef struct kPipe 		K_PIPE;		 /**< Pipe  			 */
typedef struct kEvent		K_EVENT;	 /**< Generic Event	 */
typedef K_LIST              K_TCBQ;      /**< Alias for the TCB Queue */
typedef struct kTrace  	    K_TRACE;	 /**< Tracer/Logger			*/
#endif
