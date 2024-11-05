/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 * 	In this header:
 * 					o Kernel types definition
 *
 ******************************************************************************/
/**
 * \file     ktypes.h
 * \brief    Kernel Types
 * \version  1.1.0
 * \author   Antonio Giacomelli
 *
 * \details  Kernel configuration definitions.
 *
 ******************************************************************************/



#ifndef KTYPES_H
#define KTYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
/**
 * \brief C standard library primitives alias
 */

typedef void        	VOID;
typedef char        	CHAR;
typedef unsigned char   BYTE;
typedef uint8_t    		BYTE;
typedef uint8_t    		UINT8;
typedef int8_t     		INT8;
typedef int32_t     	INT32;
typedef uint32_t    	UINT32;
typedef uint64_t    	UINT64;
typedef int64_t     	INT64;
typedef int16_t    		INT16;
typedef uint16_t   	    UINT16;
typedef size_t      	SIZE;

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
/* Yes, Windows-Like. Deal with it. */

typedef VOID*             ADDR;     /**< Generic address type */
typedef CHAR const* STRING; /**< Read-only String alias   */
typedef BYTE              PID;      /**< System defined Task ID type */
typedef BYTE		      TID;	    /**< User defined Task ID */
typedef BYTE	          PRIO;     /**< Task priority type */
typedef UINT32         	  TICK;     /**< Tick count type */
typedef VOID (*TASKENTRY)(VOID);    /**< Task entry function pointer */
typedef VOID (*CALLBACK)(ADDR);	    /**< Event callout (timers, signals, etc.)*/

/**
 *\brief Return values
 */
typedef enum kErr
{
    K_SUCCESS                     =  0, /**< Generic successful operation */
    K_ERROR                       = -1, /**< Generic error */
    K_ERR_NULL_OBJ                = -2, /**< A null object was passed as a parameter */
    K_TASK_TSLICE_DUE             =  1, /**< Used on kTickHandler */
    K_TIMER_EXPIRED               =  2, /**< Indicates a timer expired */
    K_ERR_LIST_ITEM_NOT_FOUND     = -3, /**< Item not found on a K_LIST */
    K_ERR_LIST_EMPTY              = -4, /**< Empty list */
	K_ERR_MUTEX_NOT_LOCKED		  = -5, /**< Tried to lock an unlocked mutex */
	K_ERR_MUTEX_NOT_OWNER		  = -6, /**< Tried to unlock an owned mutex */
	K_ERR_MAIL_SIZE				  = -7, /**< Invalid mail size */
	K_ERR_MEM_INIT				  = -8, /**< Error initialising memory control block */
	K_ERR_MEM_FREE				  = -9, /**< Error when freeing an allocated memory */
	K_ERR_MEM_ALLOC				  = -10, /**< Error allocating memory */
	K_ERR_MEM_INVALID_ADDR		  = -11, /**< Address does not belong to MEM */
	K_ERR_INVALID_TID			  = -12, /**< Invalid user-assigned task IDs are 0 or 255*/
	K_ERR_INVALID_Q_SIZE		  = -13, /**< Maximum message queue size is 255 items */
	K_ERR_INVALID_QMESG_SIZE	  = -14, /**< Maximum message for a message queue is 255 bytes */
	K_ERR_INVALID_BYTEPOOL_SIZE	  = -15 /**< Maximum byte pool size is 255 bytes (254 effective) */
} K_ERR;

/**
 *\brief Fault codes
 */
typedef enum kFault
{
    FAULT_READY_QUEUE       = 1, /**< Fault managin Ready Queue*/
    FAULT_NULL_OBJ          = 2, /**< Tried to handle a NULL object */
    FAULT_LIST              = 3, /**< Fault when operating on a list */
	FAULT_KERNEL_VERSION    = 4  /**< Invalid kernel version */
} K_FAULT;

/**
 * \brief Task status
 */
typedef enum kTaskStatus
{
    INVALID = 0,/**< Problem. */
    READY, 		/**< On the Ready Queue, waiting to be dispatched */
    RUNNING,    /**< Running. CPU is taken. 					  */
	PENDING,	/**< Suspended itself waiting for a direct signal */
	SLEEPING,   /**< Is sleeping waiting for a specific event
					(a timer, or a condition variable, etc.) */
	BLOCKED		/**< Blocked on semaphore or mutex 				  */
} K_TASK_STATUS;

#if (K_DEF_TRACE == ON)
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
typedef struct kMemBlock    K_BLOCKPOOL; /**< Block Pool Control Block */
typedef struct kMemByte		K_BYTEPOOL;  /**< Byte Pool Control Block	*/
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

#endif /* KTYPES_H */
