/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 ******************************************************************************
 * 	In this header:
 * 					o Kernel types definition
 *
 ******************************************************************************/

#ifndef KTYPES_H
#define KTYPES_H

#include "kenv.h"

/**
 * \brief C primitives
 */
/*** these are immutable:                                                   */
typedef void VOID;
typedef char CHAR;
typedef unsigned char BYTE;
typedef signed INT; /* stack type */
typedef unsigned UINT;
typedef unsigned long ULONG;
typedef void *ADDR; /* Generic address type 		*/

/*** if you dont provide a stdbool                                          */
#if !defined(bool)
typedef unsigned BOOL;
#define FALSE (unsigned)0
#define TRUE  (unsigned)1
#define bool
#else
typedef _Bool BOOL;
#define TRUE			true
#define FALSE			false
#endif

typedef char *STRING; /*Pointer to string of chars */

/*** Priority range: 0-31 - tasks can have the same priority    */
typedef unsigned char PID; /* System defined Task ID type */
typedef unsigned char PRIO; /* Task priority type */
typedef unsigned TICK; /* Tick count type */

/*** Func ptrs typedef */
typedef void (*TASKENTRY)(void); /* Task entry function pointer */
typedef void (*CALLOUT)(void*); /* Callout (timers)             */
typedef void (*CBK)(void*); /* Generic Call Back             */

typedef enum
{
	OR, AND
}K_OR_AND;

/**
 *\brief Return values
 */
typedef enum kErr
{

	/* SUCCESSFUL: 0U */
	K_SUCCESS = 0U, /* No Error */

	/* INFO OR UNSUCCESSFUL-NON-FAULTY RETURN VALUES: positive   */
	K_ERR_TIMEOUT = 0x1,
	K_QUERY_MBOX_EMPTY = 0x2,
	K_QUERY_MBOX_FULL = 0x3,
	K_QUERY_MUTEX_LOCKED = 0x4,
	K_QUERY_MUTEX_UNLOCKED = 0x5,
	K_ERR_MBOX_FULL = 0x6,
	K_ERR_MBOX_SIZE = 0x7,
	K_ERR_MBOX_EMPTY = 0x8,
	K_ERR_MBOX_ISR = 0x9,
	K_ERR_MBOX_NO_WAITERS = 0xA,
	K_ERR_STREAM_FULL = 0xB,
	K_ERR_STREAM_EMPTY = 0xC,
	K_ERR_MUTEX_LOCKED = 0xD,
	K_ERR_INVALID_PARAM	= 0xE,
	/* FAULTY RETURN VALUES: negative */
	K_ERROR = (int) 0xFFFFFFFF, /* (0xFFFFFFFF) Generic error placeholder */

	K_ERR_OBJ_NULL = (int) 0xFFFF0000, /* A null object was passed as a parameter */
	K_ERR_OBJ_NOT_INIT = (int) 0xFFFF0001, /* Tried to use an unitialised kernel object */

	K_ERR_LIST_ITEM_NOT_FOUND = (int) 0xFFFF0002, /* Item not found on a K_LIST */
	K_ERR_LIST_EMPTY = (int) 0xFFFF0003, /* Empty list */

	K_ERR_MEM_INIT = (int) 0xFFFF0004, /* Error initialising memory control block */
	K_ERR_MEM_FREE = (int) 0xFFFF0005, /* Error when freeing an allocated memory */
	K_ERR_MEM_ALLOC = (int) 0xFFFF0006, /* Error allocating memory */

	K_ERR_TIMER_POOL_EMPTY = (int) 0xFFFF0007,
	K_ERR_READY_QUEUE	   = (int) 0xFFFF0008,
	K_ERR_INVALID_PRIO = (int) 0xFFFF0009, /* Valid task priority range: 0-31. */

	K_ERR_INVALID_QUEUE_SIZE = (int) 0xFFFF000A, /* Maximum message queue size is 255 items */
	K_ERR_INVALID_MESG_SIZE = (int) 0xFFFF000B, /* Maximum message for a message queue is 255 bytes */

	K_ERR_MESG_CPY = (int) 0xFFFF000C, /* Error when copying a chunk of bytes from one addr to other */

	K_ERR_PDBUF_SIZE = (int) 0xFFFF000D, /* Invalid size of mesg attached to a PD Buffer */

	K_ERR_SEM_INVALID_VAL = (int) 0xFFFF000E, /* Invalid semaphore value */

	K_ERR_INVALID_TSLICE = (int) 0xFFFF0011,
	K_ERR_KERNEL_VERSION = (int) 0xFFFF0012,
	K_ERR_MBOX_INIT_MAIL = (int) 0xFFFF0013,
	K_ERR_MUTEX_REC_LOCK = (int) 0xFFFF0014,
	K_ERR_MUTEX_NOT_OWNER = (int) 0xFFFF0015,
	K_ERR_TASK_INVALID_ST = (int) 0xFFFF0016,
	K_ERR_INVALID_ISR_PRIMITIVE = (int) 0xFFFFF0017
} K_ERR;

/**
 *\brief Fault codes
 */
typedef enum kFault
{
	FAULT = K_ERROR,
	FAULT_READY_QUEUE = K_ERR_READY_QUEUE,
	FAULT_NULL_OBJ =  K_ERR_OBJ_NULL,
	FAULT_KERNEL_VERSION = K_ERR_KERNEL_VERSION,
	FAULT_OBJ_NOT_INIT = K_ERR_OBJ_NOT_INIT,
	FAULT_TASK_INVALID_PRIO = K_ERR_INVALID_PRIO,
	FAULT_UNLOCK_OWNED_MUTEX = K_ERR_MUTEX_NOT_OWNER,
	FAULT_ISR_INVALID_PRIMITVE = K_ERR_INVALID_ISR_PRIMITIVE,
	FAULT_TASK_INVALID_STATE = K_ERR_TASK_INVALID_ST,
	FAULT_TASK_INVALID_TSLICE = K_ERR_INVALID_TSLICE
} K_FAULT;

/**
 * \brief Task status
 */
typedef enum kTaskStatus
{
	INVALID = 0, READY, RUNNING,
	/* WAITING */
	PENDING, SLEEPING, BLOCKED, SUSPENDED, SENDING, RECEIVING

} K_TASK_STATUS;

typedef struct kTimeoutNode K_TIMEOUT_NODE;
typedef struct kTcb K_TCB;
#if (K_DEF_CALLOUT_TIMER==ON)
typedef struct kTimer K_TIMER;
#endif
#if (K_DEF_ALLOC==ON)
typedef struct kMemBlock K_MEM;
#endif
typedef struct kList K_LIST;
typedef struct kListNode K_NODE;
typedef K_LIST K_TCBQ;
typedef struct kTask K_TASK;
#if (K_DEF_SEMA == ON)
typedef struct kSema K_SEMA;
#endif /*sema */
#if (K_DEF_STREAM == ON)
typedef struct kStream K_STREAM;
#endif /*mesgq*/
#if (K_DEF_MBOX == ON)
typedef struct kMailbox K_MBOX;
#endif /* mbox */
#if (K_DEF_QUEUE==ON)
typedef struct kQ K_QUEUE;
#endif
#if (K_DEF_SLEEPWAKE==ON)
typedef struct kEvent K_EVENT;
#endif

#if (K_DEF_PDMESG== ON)

typedef struct kPumpDropBuf K_PDBUF;
typedef struct kPumpDropQueue K_PDMESG;

#endif

#if (K_DEF_MUTEX == ON)

typedef struct kMutex K_MUTEX;

#endif

#endif/*ktypes*/
