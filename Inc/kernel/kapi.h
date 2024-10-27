 /******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************/

/**
 * \file     kapi.h
 * \brief    Kernel API
 * \version  0.1.0
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
 * |             K0BA KERNEL                | kobjs.h k*.c
 * |________________________________________|--
 * |                                        |
 * |        BOARD SUPPORT PACKAGE           |
 * |________________________________________| stm32f4*, cmsis*
 * |                                        |
 * |                CMIS HAL                |
 * |________________________________________|--
 *
 * This is the kernel API to be included within any application
 * development. It provides methods to access the kernel services.
 *
 * Every kernel primitive is on UPPERCASE. If preceded by a K_ it is
 * kernel data structure (e.g. K_SEMA is a semaphore).
 * If not, it is an alias for standard C data or data pointer (e.g., BYTE,
 * INT32, ADDR, STRING: respectively, unsigned char, int, void* and const char*)
 * or rresents system data that is not a structure itself:
 * (e.g., PRIO for priority, TID for task ID.
 *
 * A typical kernel service often receives a pointer to a kernel
 * object or a kernel parameter -  If it does not, it either acts on a
 * (2) singleton object (3) and/or on the (4) caller itself.
 *
 *  E.g., (1) kSemaWait(SEMA const* self): decreases a counter semaphore.
 *        (2) kSleepDelay(): there is a single list of timers dedicated for
 *                           keeping track of sleeping tasks.
 *        (3) kSignal(TID id): direct signals  task. Every task is assigned an
 *        	  unique ID.
 *        (4) kYield(): caller task is suspended
 *
 *\endverbatim
 *
 ******************************************************************************

 */


#ifndef INC_K_API_H_
#define INC_K_API_H_

#include <stm32f4xx_hal.h>
#include <stm32f401xe.h>
#include <cmsis_gcc.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <kmacros.h>
#include <kconfig.h>
#include <ktypes.h>
#include <kobjs.h>
#include <klist.h>
#include <kerr.h>
#include <kglobals.h>
#include <ksch.h>

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/**
 * \brief Create a new task with time-slice scheduling
 * \param taskFuncPtr Pointer to the task function
 * \param taskName Task name
 * \param id Task ID
 * \param stackAddrPtr Pointer to the task stack
 * \param stackSize Size of the task stack
 * \param timeSlice Time-slice for the task (>1 to be set, 0 or 1 considers the systick period)
 * \param priority Task priority
 * \param runToComplet Cooperative only function, to use in deferred handlers,
 * 				  	   servers - once dispatched it is never preempted unless
 * 				  	   it blocks or yields.
 * \return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kCreateTask(const TASKENTRY taskFuncPtr, STRING taskName,
		const PID id,	UINT32* const stackAddrPtr, const UINT32 stackSize,
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
K_ERR kSemaInit(K_SEMA* self, INT32 value);

/**
 *\brief Wait on a semaphore
 *\param self Semaphore address
 *\return K_SUCCESS / K_ERROR
 */
K_ERR kSemaWait(K_SEMA* self);

/**
 *\brief Signal a semaphore
 *\param self Semaphore address
 *\return None
 */
K_ERR kSemaSignal(K_SEMA* self);

/*****************************************************************************
 *
 * MUTEX
 *
 ****************************************************************************/


/**
 *\brief Init a mutex
 *\param self mutex address
 *\return SUCCESS/FAIL
 */

K_ERR kMutexInit(K_MUTEX* const self);

/**
 *\brief Lock a mutex
 *\param self mutex address
 *\return K_SUCCESS / K_ERROR
 */
K_ERR kMutexLock(K_MUTEX* const self);

/**
 *\brief Unlock a mutex
 *\param self mutex address
 *\return SUCCESS/FAIL
 */
K_ERR kMutexUnlock(K_MUTEX* const self);


/******************************************************************************/
/*																			  */
/* MESSAGE QUEUE															  */
/*																			  */
/******************************************************************************/


/**
 *\brief Initialise the mesg buffer memory pool
 *\return K_SUCCESS/K_ERROR
 */
K_ERR kMesgBuffPoolInit(VOID);
/**
 *\brief Get a mesg buffer from the mesg buffer memory pool
 *\return Mesg Buffer address if successful, NULL if fails
 */
K_MESGBUFF* kMesgBuffGet(VOID);

/**
 *\brief Returns a mesg buffer to the mesg buffer memory pool
 *\param self Mesg Buffer address
 *\return K_SUCCESS/K_ERROR
 */
K_ERR kMesgBuffPut(K_MESGBUFF* const self);



/**
 *\brief Send to a message queue
 *\param self Message queue address
 *\param mesgPtr Message address
 *\param mesgSize Message size
 *\return SUCCESS/FAIL
 */

K_ERR kMesgQPut(K_MESGQ* self, ADDR mesgPtr, SIZE mesgSize);
/**
 * \brief Receive from a message queue
 * \param self Message Queue address
 * \param rcvdMesgPtr Pointer to address which will store the message
 * \return Sender PID
 */
PID kMesgQGet(K_MESGQ* self, ADDR rcvdMesgPtr);
/**
 *\brief Initialises a Message Queue
 *\param self Messsage Queue address
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
K_ERR kMailboxInit(K_MAILBOX* self);

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
 *\param rcvdMesgPPtr Address that will store the received mesg address
 *\retval Sender's PID. -1 if fails.
 */
PID kMailboxPend(K_MAILBOX* const self, ADDR rcvdMesgPtr);
/*****************************************************************************
 *
 * CONDITION VARIABLE
 *
 ******************************************************************************/

/**
 * \brief Initialises a condition variable.
 * \param self Condition variable address
 * \retval none
 */
VOID kCondInit(K_COND* const self);

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
 * \brief Signal a condition variable,
 * releasing all queued tasks.
 * \param self Condition variable address.
 * \return none
 */
VOID kCondWake(K_COND* const self);

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
SIZE kPipeRead(K_PIPE* const self, BYTE* destPtr, SIZE nBytes);

/**
*\brief Write a stream of bytes to a pipe
*\param self Pointer to a pipe
*\param srcPtr Address to get data
*\param nBytes Number of bytes to be write
*\retval Number of written bytes if success. -1 if fails.
*/
SIZE kPipeWrite(K_PIPE* const self, const BYTE* srcPtr, SIZE nBytes);

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
K_ERR kFifoPut(K_FIFO* self, BYTE data);
/**
 * \brief Get a single byte from a fifo
 * \param self FIFO address
 * \return Read byte
 */
BYTE FifoGet(K_FIFO* self);

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
VOID kSignal(PID taskID);

/**
 * \brief Suspends a task waiting for a specific event
 * \param self Pointer to a K_EVENT object
 */
K_ERR kSleep(K_EVENT* self);

/**
 * \brief Wakes a task waiting for a specific event
 * \param self Pointer to a K_EVENT object
 */
K_ERR kWake(K_EVENT* self);


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
 * \param ticks initial tick count
 * \param funPtr The callback when timer expires
 * \param argsPtr Address to callback function arguments
 * \param reload TRUE for reloading after timer-out. FALSE for an one-shot
 * \return K_SUCCESS/K_ERROR
 */


K_ERR kTimerInit(STRING timerName, TICK ticks, CALLBACK funPtr, ADDR argsPtr,
				 BOOL reload);

/**
 * \brief Initialises the application timer pool
 * \return K_SUCCESS/K_ERROR
 */
K_ERR kTimerPoolInit(VOID);

/**
 * \brief Put a timer back into the timer pool
 * \param self Pointer to the timer
 * \return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kTimerPut(K_TIMER* const self);

/**
 * \brief Get a timer from the timer pool
 * \return K_SUCCESS on success, K_ERROR on failure
 */
K_TIMER* kTimerGet(VOID);

/**
 * \brief Busy-wait the task for a specified delay in ticks
 * \param delay The delay time in ticks
 */
VOID kBusyDelay(TICK delay);

/**
 * \brief Puts the current task to sleep for a number of ticks
 * \param ticks Number of ticks to sleep
 */
VOID kSleepDelay(TICK ticks);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*******************************************************************************
 *
 * MEMORY POOL
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
K_ERR kMemInit(K_MEM* const self, ADDR const memPoolPtr, BYTE blkSize,
		const BYTE numBlocks);

/**
 * \brief Allocate memory from a block pool
 * \param self Pointer to the block pool
 * \return Pointer to the allocated block, or NULL on failure
 */
ADDR kMemAlloc(K_MEM* const self);

/**
 * \brief Free a memory block back to the block pool
 * \param self Pointer to the block pool
 * \param blockPtr Pointer to the block to free
 * \return Pointer to the allocated memory. NULL on failure.
 */
K_ERR kMemFree(K_MEM* const self, ADDR const blockPtr);

/**
 ******************************************************************************
 *
 * TRACER
 *
 ******************************************************************************/
#if (K_CONF_TRACER == ON)
void kTraceInit();
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
TID kGetTaskPID(TID taskID);
/**
 * \brief Gets a task priorirty
 * \param taskID Task user-defined ID
 */
PRIO kGetTaskPrio(TID taskID);
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

#endif /* INC_K_API_H_ */
