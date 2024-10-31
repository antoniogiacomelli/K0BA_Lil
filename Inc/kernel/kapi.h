 /******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************/

/**
 * \file     kapi.h
 * \brief    Kernel API
 * \version  1.1.0
 * \author   Antonio Giacomelli
 *
 *
 * \verbatim
 *  ________________________________________
 * |                                        |--
 * | [TASK1][TASK2]  ....         [TASKN]   | application.c/application.h
 * |----------------------------------------|--
 * |                 API                    | this file
 * |----------------------------------------|--
 * |                                        |
 * |             K0BA KERNEL                | k*.h k*.c
 * |________________________________________|--
 * |                                        |
 * |        BOARD SUPPORT PACKAGE           |
 * |________________________________________|
 * |                                        |
 * |                CMIS HAL                |
 * |________________________________________|--
 *
 * This is the kernel API to be included within any application
 * development. It provides methods to access the kernel services.
 *
 * Every kernel primitive is on UPPERCASE. If preceded by a K_ it is a
 * kernel data structure (e.g. K_SEMA is a semaphore) or an enumarated
 * typedef.
 *
 * If not, it is an alias for standard C data or data pointer (e.g., BYTE,
 * INT32, ADDR, STRING: respectively, unsigned char, int, void* and const char*)
 * or represents system data that is not a structure itself:
 * (e.g., PRIO for priority, TID for task ID.)
 *
 * A typical kernel service often receives a pointer to a kernel
 * object (typically 'K_TYPE* const self'. (1) If it does not,
 * it either acts onsingleton object (2) and/or on the (3) caller itself.
 *
 *  E.g., (1) kSemaWait(SEMA const* self): decreases a counter semaphore.
 *        (2) kSleepDelay(): there is a single list of timers dedicated for
 *                           keeping track of sleeping tasks.
 *            kSignal(TID id): direct signals a task. Every task is assigned an
 *        	  unique ID.
 *        (3) kYield(): caller task is SLEEPING
 *
 *\endverbatim
 *
 ******************************************************************************
 */
#ifndef K_API_H
#define K_API_H
/*--------------------------------*/
/*Include ARM GCC, ARM CMSIS-CORE */
/*dependencies here				  */
/*--------------------------------*/
#include "kmacros.h"
#include "kconfig.h"
#include "ktypes.h"
#include "kobjs.h"
#include "kglobals.h"
#include "klist.h"
#include "kerr.h"
#include "ksch.h"
#include "kcheck.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/**
 * \brief Create a new task with time-slice scheduling
 * \param taskFuncPtr Pointer to the task function
 * \param taskName Task name
 * \param id user-defined Task ID - valid range: 1-254
 * \param stackAddrPtr Pointer to the task stack
 * \param stackSize Size of the task stack (in WORDS!)
 * \param timeSlice Time-slice for the task (only effective if greater than 1)
 * \param priority Task priority
 * \param runToCompl Cooperative only function, to use in deferred handlers,
 * 				  	   servers - once dispatched it is never preempted unless
 * 				  	   it blocks or yields.
 * \return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kCreateTask(const TASKENTRY taskFuncPtr, STRING taskName,
		const TID id,	UINT32* const stackAddrPtr, const UINT32 stackSize,
		const TICK timeSlice, const PRIO priority, const BOOL runToCompl);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/**
 * \brief Yields the current task to allow a context switch
 */
VOID kYield(VOID);
/****************************************************************************/
/*																			*/
/* SEMAPHORE																*/
/*																			*/
/****************************************************************************/


/**
 *\brief Initialise a semaphore
 *\param self Semaphore address
 *\param value Initial value
 *\return None
 */
K_ERR kSemaInit(K_SEMA* const self, INT32 value);

/**
 *\brief Wait on a semaphore
 *\param self Semaphore address
 *\return K_SUCCESS / K_ERROR
 */
K_ERR kSemaWait(K_SEMA* const self);

/**
 *\brief Signal a semaphore
 *\param self Semaphore address
 *\return None
 */
K_ERR kSemaSignal(K_SEMA* const self);

/*****************************************************************************
 *
 * MUTEX
 *
 ****************************************************************************/


/**
 *\brief Init a mutex
 *\param self mutex address
 *\return K_SUCCESS / K_ERROR
 */

K_ERR kMutexInit(K_MUTEX* const self);

/**
 *\brief Lock a mutex
 *\param self mutex address
 *\return K_SUCCESS or a specific error
 */
K_ERR kMutexLock(K_MUTEX* const self);

/**
 *\brief Unlock a mutex
 *\param self mutex address
 *\return K_SUCCESS or a specific error
 */
K_ERR kMutexUnlock(K_MUTEX* const self);


/******************************************************************************/
/*																			  */
/* MESSAGE QUEUE															  */
/*																			  */
/******************************************************************************/


/**
 *\brief Send to a message queue
 *\param self Message queue address
 *\param mesgPtr Message address
 *\param mesgSize Message size
 *\return SUCCESS/FAIL
 */

K_ERR kMesgQPut(K_MESGQ* const self, ADDR mesgPtr, BYTE mesgSize);
/**
 * \brief Receive from a message queue
 * \param self Message Queue address
 * \param rcvdMesgPtr Pointer to address which will store the message
 * \return Sender PID
 */
PID kMesgQGet(K_MESGQ* const self, ADDR rcvdMesgPtr);
/**
 *\brief Initialises a Message Queue
 *\param self Messsage Queue address
 *\param mesgPoolPtr Address of the message pool
 *\param queueSize Number of items in the queue
 *\param mesgSize Size of each item in bytes. sizeof() is recommended.
 *\return none
 */
VOID kMesgQInit(K_MESGQ* const self, ADDR mesgPoolPtr, BYTE queueSize, \
				   BYTE mesgSize);


/******************************************************************************/
/*																			  */
/* MAILBOX																	  */
/*																			  */
/******************************************************************************/

/**
 *\brief Initialise a mailbox
 *\param self Pointer to a mailbox
 */
K_ERR kMailboxInit(K_MAILBOX* const self);

/**
 *\brief Post a message on a mailbox w
 *\param self Address of a mailbox structure
 *\param mesgPtr Message address
 *\param mesgSize Message size
 *\retval 0 if success. -1 if fails.
 */
K_ERR kMailboxPost(K_MAILBOX* const self, const ADDR mesgPtr, SIZE mesgSize);

/**
 *\brief Retrieves a message from a mailbox
 *\param self Address of a mailbox structure
 *\param mailPPtr Pointer-to-pointer to variable that will store the received
 *\				  mail
 *\param sizePtr  Address of the variable to store the mail size. You can
 *				  pass 'NULL' if you sure wha is the size. (Default size is
 *				  4 bytes)
 *\retval Sender's TID. -1 if fails.
 */
TID kMailboxPend(K_MAILBOX* const self, ADDR* mailPPtr, SIZE* sizePtr);

#if (K_DEF_COND==ON)
/*****************************************************************************
 *
 * CONDITION VARIABLE
 *
 ******************************************************************************/

/**
 * \brief Initialises a condition variable.
 * \param self Condition variable address
 * \retval K_SUCCESS/K_ERROR
 */
K_ERR kCondInit(K_COND* const self);

/**
 * \brief Wait on a condition variable.
 * \param self Condition variable address.
 * \retval none
 */
VOID kCondWait(K_COND* const self);

/**
 * \brief Signals a condition variable,
 * to release one queued task, if any.
 * \param self Condition variable address.
 * \retval none
 */
VOID kCondSignal(K_COND* const self);

/**
 * \brief Broad cast signal a condition variable
 * releasing all queued tasks.
 * \param self Condition variable address.
 * \return none
 */
VOID kCondWake(K_COND* const self);

#endif /* K_DEF_COND */


#if (K_DEF_PIPE==ON)
/******************************************************************************/
/*																			  */
/* PIPES																	  */
/*																			  */
/*******************************************************************************/

/**
*\brief Initialise a pipe
*\param self Pointer to the pipe
*/
VOID kPipeInit(K_PIPE* const self);

/**
*\brief Read a stream of bytes from a pipe
*\param self Pointer to a pipe
*\param destPtr Address to store the read data
*\param nBytes Number of bytes to be read
*\retval Number of read bytes if success. -1 if fails.
*/
INT32 kPipeRead(K_PIPE* const self, BYTE* destPtr, UINT32 nBytes);

/**
*\brief Write a stream of bytes to a pipe
*\param self Pointer to a pipe
*\param srcPtr Address to get data
*\param nBytes Number of bytes to be write
*\retval Number of written bytes if success. -1 if fails.
*/
INT32 kPipeWrite(K_PIPE* const self, const BYTE* srcPtr, UINT32 nBytes);


#endif /*K_DEF_PIPES*/
/**
 * \brief Initialise Simple thread-safe FIFO
 * \param self FIFO address
 * \return K_ERROR/SUCCESS
 */
K_ERR kFifoInit(K_FIFO* const self);
/**
 * \brief Put a single byte on a fifo
 * \param self FIFO address
 * \param data One-byte data
 * \return K_ERROR/SUCCESS
 */
K_ERR kFifoPut(K_FIFO* const self, BYTE data);
/**
 * \brief Get a single byte from a fifo
 * \param self FIFO address
 * \return Read byte
 */
BYTE kFifoGet(K_FIFO* const self);

/******************************************************************************
 *
 * DIRECT TASK SIGNAL/PEND
 *
 ******************************************************************************/

/**
 * \brief Put the current task into a wait state
 *		  to be waken from another task/ISR
 * \see kSignal
 */
VOID kPend(VOID);

/**
 * \brief Direct Signal a task
 * \param taskID The ID of the task to signal
 */
VOID kSignal(PID const taskID);

/**
 * \brief Suspends a task waiting for a specific event
 * \param self Pointer to a K_EVENT object
 */
K_ERR kSleep(K_EVENT* const self);



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/*******************************************************************************
 *
 * APPLICATION TIMER AND DELAY
 *
 ******************************************************************************/
/**
 * \brief Initialises an application timer
 * \param timerName a STRING (const char*) label for the timer
 * \param ticks initial tick count
 * \param funPtr The callback when timer expires
 * \param argsPtr Address to callback function arguments
 * \param reload TRUE for reloading after timer-out. FALSE for an one-shot
 * \return K_SUCCESS/K_ERROR
 */
K_ERR kTimerInit(STRING timerName, TICK ticks, CALLBACK funPtr, ADDR argsPtr,
				 BOOL reload);

/**
 * \brief Busy-wait the task for a specified delay in ticks.
 *        Task does not suspend.
 * \param delay The delay time in ticks
 */
VOID kBusyDelay(TICK const delay);

/**
 * \brief Put the current task to sleep for a number of ticks.
 *        Task switches to SLEEPING state.
 * \param ticks Number of ticks to sleep
 */
VOID kSleepDelay(TICK const ticks);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*******************************************************************************
 *
 * BLOCK MEMORY POOL
 *
 ******************************************************************************/


/**
 * \brief Memory Pool Control Block Initialisation
 * \param self Pointer to a pool control block
 * \param memPoolPtr Address of a pool (typically an array) \
 * 		  of objects to be handled
 * \param blkSize Size of each block in bytes
 * \param numBlocks Number of blocks
 * \return K_ERROR/K_SUCCESS
 */
K_ERR kBlockPoolInit(K_BLOCKPOOL* const self, ADDR const memPoolPtr, BYTE blkSize, \
		const BYTE numBlocks);

/**
 * \brief Allocate memory from a block pool
 * \param self Pointer to the block pool
 * \return Pointer to the allocated block, or NULL on failure
 */
ADDR kBlockPoolAlloc(K_BLOCKPOOL* const self);

/**
 * \brief Free a memory block back to the block pool
 * \param self Pointer to the block pool
 * \param blockPtr Pointer to the block to free
 * \return Pointer to the allocated memory. NULL on failure.
 */
K_ERR kBlockPoolFree(K_BLOCKPOOL* const self, ADDR const blockPtr);


/*******************************************************************************
*
* BYTE MEMORY POOL
*
******************************************************************************/

/**
*
* \details
*
* Le heap, c'est chic...
*
* About BYTE POOLS.
*
* It is hard to find a trade-off for byte pools - a random chunk of bytes the
* application can allocate and deallocate.
* A safer version, with meta-data is counter-productive,  given you need
* a record (a struct) to keep track of every itsy bitsy weenie BYTE.
* In very constrained applications, it is also a bit hard to deem a situation
* that cannot be circumvented either by a fixed-size pool or by -
* the safest choice by far - a static memory allocation.
*
* So, it is up to the application programmer to diminish the hazards:
*
*  o Do Not allocate and deallocate randomly. You got no UNIX here, son.
*    It's hardcore emBedded \m/
*
*  o TAKE THE OATH BEFORE THIS KERNEL:
*    - to allocate, use, and free the EXACTLY size for each chunk you
*      requested. In this order.
*
*  o If you write out of the boundaries you have allocated - too sad, too bad
*    end of story.
*
*  o Check for NULL returns. Take a look on the pool. If it is too fragmented
*    so you won't find a contiguous chunk the size you need, you are
*    better off reinitialising the pool.
*
*  o The IDEAL use:
*    - allocate and deallocate *multiples* of the pool size.
*    - do it on a unidirectional manner, just as you would do with
*      synchronisation to avoid a 'deadlock'.
*
* \verbatim
*
* - - DEPICTION - -
*
* You associate a pool (an array) of BYTEs to a BYTE POOL CONTROL BLOCK
* (K_BYTEPOOL)
*
* * - allocated byte
* | free byte
* x sentinel
*
* o INITIALISATION:
*
* [||||||||||||||||||||||||||||||||||||||x]
*  ^freelist
* [------------- poolSize-1--------------)
*
*
* o ALLOCATION:
*
* - allocated byte
* | free byte
* x sentinel
*
* Pool before allocation::
* [-----|||||||||||||||||||||||||||||||||x]
*       ^freelist
*
* Size Requested by application: ||||||
*
* Pool after allocation:
*
* [----------||||||||||||||||||||||||||||x]
*       -----^freeList updated
*       |
*       returned ADDR
*
* o FREE:
* Returns to the pool a chunk of bytes
*
* Fragmented Pool
*   [-------||---||||||||||||||||||||||||||x]
*      ^ Application asks to free: ||||
*
* After Freeing:
*   [--||||-||---||||||||||||||||||||||||||x]
*      ^..·.^·...^
*      freeList
*
* After Merging Adjacent Free Block:
*   [--||||||----||||||||||||||||||||||||||x]
*      ^....·....^
*      freeList
*  \endverbatim
*******************************************************************************/


#if (K_DEF_BYTEPOOL==ON)

/**
 * \brief Initialises a Byte pool control block.
 * \param self Pointer to the byte pool control block
 * \param memPool The memory to be associated to this Control Block
 * \param poolSize Pool size. MAX: 255 bytes
 * \return K_SUCCESS or K_ERR_MEM_INIT
 */
K_ERR kBytePoolInit(K_BYTEPOOL* const self, BYTE* memPool, BYTE const poolSize);
/**
 * \brief Allocates a chunk of bytes from a byte pool
 * \param self Pointer to the Byte Pool Control Block associated
 *             to a byte pool
 * \param size Number of required bytes + 1
 * \return ADDRess of the byte chunk. NULL when failure allocating.
 */
ADDR kBytePoolAlloc(K_BYTEPOOL* const self, BYTE const size);

/**
 * \brief Deallocates a chunk of bytes (give it back to the pool)
 * \param self Pointer to the Byte Pool Control Block associated
 *             to a byte pool
 * \param chunkPtr Address of the chunk
 * \param size     Chunk size. Be sure to free the same number
 *                 of you bytes you allocated.
 * \return K_SUCCESS / K_ERR_MEM_FREE / K_ERR_MEM_INVALID_ADDR
 */
K_ERR kBytePoolFree(K_BYTEPOOL* const self, BYTE* const chunkPtr, \
		BYTE const size);


#endif /*K_DEF_BYTEPOOL*/
/******************************************************************************
 *
 * TRACER
 *
 ******************************************************************************/
#if (K_DEF_TRACER == ON)
VOID kTraceInit(VOID);
K_ERR kTrace(K_TRACEID event, CHAR* info);
#endif

/*******************************************************************************
 *
 * UTILS
 *
 ******************************************************************************/
/**
 *\brief Gets a task system ID
 *\param taskID user-defined ID
 *\return Task system ID
 */
TID kGetTaskPID(TID const taskID);
/**
 * \brief Gets a task priorirty
 * \param taskID user-defined Task ID
 */
PRIO kGetTaskPrio(TID const taskID);
/**
 * \brief Gets the current number of  ticks
 * \return Global system tick value
 */
TICK kTickGet(VOID);

/**
 * \brief Returns the size of a string
 * \param string A pointer to char
 * \return string length - 1 (does not count '\0')
 */
SIZE kStrLen(STRING string);
/**
 * \brief Deep copy data from one address to other
 * \param destPtr Destination address
 * \param srcPtr  Source address
 * \param size    Number of bytes to be copied
 * \return        Destination address
 */
ADDR kMemCpy(ADDR destPtr, const ADDR srcPtr, SIZE size);


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


#endif /* K_API_H */
