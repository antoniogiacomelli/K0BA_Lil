/******************************************************************************
*
* [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]
*
******************************************************************************/
/**
 * \file     kapi.h
 * \brief    Kernel API
 * \version  0.4.0
 * \author   Antonio Giacomelli
 *
 * This is the kernel API to be included within any application
 * development. It provides methods to access the kernel services.
 *
 * By default, it is placed in "application.h"
 * For detailed return values \see ktypes.h.
 *
 *
 **/

#ifndef KAPI_H
#define KAPI_H

#include "kexecutive.h"

/******************************************************************************/
/**
 * \brief 			   Create a new task.
 *
 * \param taskHandle   Handle object for the task.
 *
 * \param taskFuncPtr  Pointer to the task entry function.
 *
 * \param taskName     Task name. Keep it as much as 8 Bytes.
 *
 * \param stackAddrPtr Pointer to the task stack (the array variable).
 *
 * \param stackSize    Size of the task stack (in WORDS. 1WORD=4BYTES)
 *
 * \param timeSlice    UNSIGNED integer for time-slice.
 * 					   If time-slice is ON, value 0 is invalid.
 *
 *
 * \param priority     Task priority - valid range: 0-31.
 *
 * \param runToCompl   If this flag is 'TRUE',  the task once dispatched
 *                     although can be interrupted by tick and other hardware
 *                     interrupt lines, won't be preempted by user tasks.
 *                     runToCompl tasks are normally deferred handlers for ISRs.
 *
 * \return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kCreateTask( K_TASK *taskHandlePtr, TASKENTRY const taskFuncPtr,
		STRING taskName, INT *const stackAddrPtr,
		UINT const stackSize,
#if(K_DEF_SCH_TSLICE==ON)
        TICK const timeSlice,
#endif
		PRIO const priority, BOOL const runToCompl);

/**
 * \brief Initialises the kernel. To be called in main()
 *        after hardware initialisation and task creation.
 *
 */
VOID kInit( VOID);

/**
 * \brief Yields the current task.
 *
 */
VOID kYield( VOID);

#if (K_DEF_FUNC_DYNAMIC_PRIO==ON)
/**
 * \brief			A task changes its priority
 * \param newPrio   New priority value
 * \return			K_SUCCESS or specific error
 */
K_ERR kTaskChangePrio( PRIO newPrio);

/**
 * \brief			A task restore its original priority
 * \return			K_SUCCESS or specific error
 */
K_ERR kTaskRestorePrio( VOID);
#endif
/*******************************************************************************
 COUNTER SEMAPHORE
 *******************************************************************************/
#if (K_DEF_SEMA==ON)
/**
 *\brief      		Initialise a semaphore
 *\param kobj  		Semaphore address
 *\param value 		Initial value
 *\return      \K_SUCCESS or specific error
 */

K_ERR kSemaInit( K_SEMA *const kobj, INT const value);

/**
 *\brief 			Wait on a semaphore
 *\param kobj 		Semaphore address
 *\param timeout	Maximum suspension time
 */
K_ERR kSemaWait( K_SEMA *const kobj, TICK const timeout);

/**
 *\brief Signal a semaphore
 *\param kobj Semaphore address
 */
VOID kSemaSignal( K_SEMA *const kobj);

/**
 *\brief Return the counter's value of a semaphore
 *\param kobj Semaphore address
 *\return      Counter's value
 */
INT kSemaQuery( K_SEMA *const kobj);

#endif
/*******************************************************************************
 * MUTEX SEMAPHORE
 *******************************************************************************/
#if (K_DEF_MUTEX==ON)
/**
 *\brief Init a mutex
 *\param kobj mutex address
 *\return K_SUCCESS / K_ERROR
 */

K_ERR kMutexInit( K_MUTEX *const kobj);

/**
 *\brief Lock 		a mutex
 *\param kobj 		mutex address
 *\param timeout	Maximum suspension time
 *\return K_SUCCESS or a specific error \K_SUCCESS or specific error
 */
K_ERR kMutexLock( K_MUTEX *const kobj, TICK timeout);

/**
 *\brief Unlock a mutex
 *\param kobj mutex address
 *\return K_SUCCESS or specific error
 */
K_ERR kMutexUnlock( K_MUTEX *const kobj);

/**
 * \brief Return the state of a mutex (locked/unlocked)
 */
K_ERR kMutexQuery( K_MUTEX *const kobj);

#endif

/*******************************************************************************
 * MAILBOX (SINGLE-ITEM MAILBOX)
 *******************************************************************************/
#if (K_DEF_MBOX == ON)

/**
 * \brief               Initialises an indirect single mailbox.
 * \param kobj          Mailbox address.
 * \param initMail		If initialising full, address of initial mail.
 *  					Otherwise NULL.
 * \return              K_SUCCESS or specific error.
 */

K_ERR kMboxInit( K_MBOX *const kobj, ADDR initMail);
/**
 * \brief               Send to a mailbox. Task blocks when full.
 * \param kobj          Mailbox address.
 * \param sendPtr       Mail address.
 * \param timeout		Suspension time-out
 * \return              K_SUCCESS or specific error.
 */

K_ERR kMboxPost( K_MBOX *const kobj, ADDR const sendPtr, TICK timeout);
/**
 * \brief               Receive from a mailbox. Block if empty.
 *
 * \param kobj          Mailbox address.
 * \param recvPPtr      Address that will store the message address (pointer-to-pointer).
 * \param timeout		Suspension time-out
 * \return				K_SUCCESS or specific error.
 */
K_ERR kMboxPend( K_MBOX *const kobj, ADDR *recvPPtr, TICK timeout);

#if (K_DEF_FUNC_MBOX_POSTOVW==(ON))

/**
 * \brief			Post to a mailbox even if it is full, overwriting the
 *                  the current mail.
 * \param kobj		Mailbox address.
 * \param sendPtr   Mail address.
 * \return          K_SUCCESS or specific error
 */
K_ERR kMboxPostOvw( K_MBOX *const kobj, ADDR const sendPtr);

#endif

#if (K_DEF_FUNC_MBOX_PEEK==ON)

/**
 * \brief 			   Reads the mail without extracting it.
 * \param kobj		   Mailbox address.
 * \param peekPPtr	   Pointer to receive address.
 * \return			   K_SUCCESS or specific error.
 */
K_ERR kMboxPeek( K_MBOX *const kobj, ADDR *peekPPtr);

#endif

#if (K_DEF_FUNC_MBOX_POSTPEND==ON)

/**
 * \brief               Send and receive from the same mailbox.
 * \param kobj          Mailbox address.
 * \param sendPtr		Address of sending message.
 * \param recvPPtr      Address to store the response. (can be &sendPtr)
 * \param timeout		Suspension time-out
 * \return				K_SUCCESS or specific error.
 */
K_ERR kMboxPostPend( K_MBOX *const kobj, ADDR const sendPtr,
		ADDR *const recvPPtr, TICK timeout);
#endif

#if (K_DEF_FUNC_MBOX_ISFULL==ON)
/**
 * \brief   Check if a mailbox is full.
 * \return  TRUE or FALSE.
 */
BOOL kMboxIsFull( K_MBOX *const kobj);

#endif

#endif /* MBOX  */
/*******************************************************************************
 MESSAGE QUEUES (QUEUE AND STREAM)
*******************************************************************************/
/*******************************************************************************
 * There are two mechanisms working as Message Queues: QUEUEs and STREAMs
 *
 * QUEUES are Mailboxes of multiple messages (aka Multibox) - each slot is
 * fixed to 4-byte size - and you normally pass a pointer to a message,
 * unless the message is a 4-byte message (INT, UINT, LONG, ULONG).
 *
 * STREAMs are byte-oriented message queues which transmit fixed-size messages
 * by deep copy. Each STREAM will have its message size declared on initialisation
 * and, again, it is fixed.
 *
 * For more information look at the Docbook.
 *
 ******************************************************************************/

#if (K_DEF_QUEUE == ON)

/**
 * \brief			 Initialises a mail queue.
 * \param kobj		 Multibox address
 * \param memPtr     Pointer to the buffer that will store mail addresses
 * \param maxItems   Maximum number of mails.
 * \return           K_SUCCESS or specific error.
 */
K_ERR kQueueInit( K_QUEUE *const kobj, ADDR memPtr, ULONG maxItems);
/**
 * \brief               Send to a multilbox. Task blocks when full.
 * \param kobj          Multibox address.
 * \param sendPtr       Mail address.
 * \param timeout		Suspension time-out
 * \return              K_SUCCESS or specific error.
 */
K_ERR kQueuePost( K_QUEUE *const kobj, ADDR const sendPtr, TICK timeout);

/**
 * \brief               Receive from a mail queue. Block if empty.
 *
 * \param kobj          Multibox address.
 * \param recvPPtr      Address that will store the message address
 * 					  (pointer-to-pointer).
 * \param timeout		Suspension time-out
 * \return				K_SUCCESS or specific error.
 */
K_ERR kQueuePend( K_QUEUE *const kobj, ADDR *recvPPtr, TICK timeout);

#if (K_DEF_FUNC_QUEUE_PEEK==ON)

/**
 * \brief 			   Reads the head's mail without extracting it.
 * \param kobj		   Multibox address.
 * \param peekPPtr	   Pointer to receive address.
 * \return			   K_SUCCESS or specific error.
 */
K_ERR kQueuePeek( K_QUEUE *const kobj, ADDR *peekPPtr);

#endif

#if (K_DEF_FUNC_QUEUE_ISFULL==ON)
/**
 * \brief   		Check if a mail queue is full.
 * \param kobj		Multibox address.
 * \return  		TRUE or FALSE.
 */
BOOL kQueueIsFull( K_QUEUE *const kobj);

#endif

#if (K_DEF_FUNC_QUEUE_MAILCOUNT==ON)
/**
 * \brief			Gets the current number of mails within a queue.
 * \param kobj      Multibox address.
 * \return			Number of mails.
 */
ULONG kQueueMailCount( K_QUEUE *const kobj);

#endif

#endif /* MAIL QUEUE  */

#if (K_DEF_STREAM == ON)
/**
 *\brief 			Initialise a Message Queue (Stream)
 *\param kobj		Message Queue address
 *\param buffer		Allocated memory. It must be enough for the queue capacity
 *\					that is messsageSize*maxMessages
 *\param messageSize Message size
 *\param maxMessage  Max number of messages
 *\return 			 K_SUCCESS or specific errors
 */
K_ERR kStreamInit( K_STREAM *const kobj, ADDR buffer, ULONG messageSize,
		ULONG maxMessages);

#if (K_DEF_FUNC_STREAM_MESGCOUNT==ON)

/**
 *\brief 			Get the current number of messages within a message queue.
 *\param kobj		(Stream) Queue address
 *\param mesgCntPtr Address to store the message number
 *\return			K_SUCCESS or a specific error.
 */

K_ERR kStreamGetMesgCount( K_STREAM *const kobj, UINT *const mesgCntPtr);

#endif

#if (K_DEF_FUNC_STREAM_JAM == ON)

/**
 *\brief 			Sends a message to the queue front.
 *\param kobj		(Stream) Queue address
 *\param sendPtr	Message address
 *\param timeout	Suspension time
 *\return			K_SUCCESS or specific error
 */
K_ERR kStreamJam( K_STREAM *const kobj, ADDR const sendPtr, TICK timeout);

#endif

/**
 *\brief 			Receive a message from the queue
 *\param kobj		(Stream) Queue address
 *\param recvPtr	Receiving address
 *\param Timeout	Suspension time
 */
K_ERR kStreamRecv( K_STREAM *const kobj, ADDR recvPtr, TICK timeout);

/**
 *\brief 			Send a message to a message queue
 *\param kobj		(Stream) Queue address
 *\param recvPtr	Message address
 *\param Timeout	Suspension time
 */
K_ERR kStreamSend( K_STREAM *const kobj, ADDR const sendPtr, TICK timeout);

#if (K_DEF_FUNC_STREAM_PEEK==ON)

/**
 *\brief 			Receive the front message of a queue
 *					without changing its state
 *\param	kobj		(Stream) Queue object address
 *\param	recvPtr		Receiving pointer address
 *\return			K_SUCCESS or error.
 */
K_ERR kStreamPeek( K_STREAM *const kobj, ADDR recvPtr);

#endif

#endif /*K_DEF_STREAM*/

/*******************************************************************************
 * PUMP-DROP LIFO QUEUE (CYCLIC ASYNCHRONOUS BUFFERS - CABs)
 *******************************************************************************/

#if (K_DEF_PDMESG == ON)

/**
 * \brief               Pump-drop Message Control Block initialisation.
 *                      This initialisation associates a pool of buffers
 *                      to a unique control block.
 *
 * \param kobj    	    PD Control Block address.
 * \param memCtrlPtr	Pointer to the memory allocator control block
 * \param bufPool  		Pool of PD Buffers, statically allocated.
 * \param nBufs    		Number of buffers for this queue.
 * \return         K_SUCCESS or specific error
 */
K_ERR kPDMesgInit( K_PDMESG *const kobj, K_MEM *const memCtrlPtr,
		K_PDBUF *const bufPool, BYTE const nBufs);

/**
 * \brief          Reserves a pump-drop buffer before writing on it.
 * \param kobj     PD Message Control address.
 * \return         K_SUCCESS or specific error
 */
K_PDBUF* kPDMesgReserve( K_PDMESG *const kobj);
/**
 * \brief           Writes into a PD buffer the source address and the size
 *                  of a data message.
 * \param bufPtr    Buffer address.
 * \param srcPtr    Message address.
 * \param dataSize  Message size.
 * \return			K_SUCCESS or specific error.
 */
K_ERR kPDBufWrite( K_PDBUF *bufPtr, ADDR srcPtr, ULONG dataSize);

/**
 * \brief          Pump a buffer into the queue - make it available for readers.
 *
 * \param kobj     LIFO address.
 * \return         Address of a written PD buffer.
 */
K_ERR kPDMesgPump( K_PDMESG *const kobj, K_PDBUF *bufPtr);

/**
 * \brief          Fetches the most recent buffer pumped in the queue.
 *
 * \param kobj     LIFO address.
 * \return         Address of a PD buffer available for reading.
 */
K_PDBUF* kPDMesgFetch( K_PDMESG *const kobj);

/**
 * \brief          Copies the message from a PD Buffer to a chosen address.
 * \param bufPtr   Address of the PD buffer.
 * \param destPtr  Address that will store the message.
 * \return         K_SUCCESS or specific error
 */
K_ERR kPDBufRead( K_PDBUF *const bufPtr, ADDR destPtr);

/**
 * \brief         Called by reader to indicate it has consumed the
 *                message from a buffer. If there are no more readers
 *                using the buffer, and it is not the last buffer pumped
 *                in the queue, it will be reused.
 * \param kobj    Queue address;
 * \return        K_SUCCESS or specific error
 */
K_ERR kPDMesgDrop( K_PDMESG *const kobj, K_PDBUF *const bufPtr);

#endif

/******************************************************************************
 * DIRECT TASK SIGNALS - BINARY SEMAPHORE AND FLAGS
 ******************************************************************************/

/**
 * \brief A task pends on its own binary semaphore
 * \param timeout Suspension time until signalled
 * \return K_SUCCESS or specific error
 */
K_ERR kTaskPend( TICK timeout);

/**
 * \brief Signal a task's binary semaphore
 * \param taskHandlePtr Pointer to task handle
 * \return K_SUCCESS or specific error
 */
K_ERR kTaskSignal( K_TASK *const taskHandlePtr);


#if (K_DEF_TASK_FLAGS == ON)
/**
 * \brief Writes a bit string of events to a task´s private event flags
 * \param taskHandlePtr Pointer to the target task's handle
 * \param flagMask The bit string of flags
 * \param updatedFlagsPtr Address to store the updated flags return
 * \return K_SUCCESS or specific error
 */
K_ERR kTaskFlagsPost( K_TASK *const taskHandlerPtr, ULONG flagMask,
		ULONG *updatedFlagsPtr, ULONG option);
/**
 * \brief A task checks for a combination of events on its private flags
 * \param requiredFlags The combination required
 * \param option K_OR for ANY of the flags, K_AND for ALL flags
 * 				 K_OR_CLEAR/K_AND_CLEAR clears the flags that were met
 * 				 K_MAIL just returns whatever is on task flags
 * \param timeout Suspension timeout
 * \return The updated bit string of flags.
 */
ULONG kTaskFlagsPend( ULONG requiredFlags,  ULONG option, TICK timeout);
#endif

/******************************************************************************
 * EVENTS
 ******************************************************************************/

#if (K_DEF_EVENT==ON)
/**
 * \brief 			Initialise an event
 * \param kobj		Pointer to K_EVENT object
 * \return			K_SUCCESS/error
 */
K_ERR kEventInit( K_EVENT *const kobj);
/**
 * \brief 			Suspends a task waiting for a wake signal
 * \param kobj 		Pointer to a K_EVENT object
 * \param timeout	Suspension time.
 */
K_ERR kEventSleep( K_EVENT *const kobj, TICK timeout);

/**
 * \brief Wakes all tasks sleeping for a specific event
 * \param kobj Pointer to a K_EVENT object
 * \return K_SUCCESS or specific error
 */
K_ERR kEventWake( K_EVENT *const kobj);

/**
 * \brief Wakes a single task sleeping for a specific event
 *        (by priority)
 * \param kobj Pointer to a K_EVENT object
 * \return K_SUCCESS or specific error
 */
K_ERR kEventSignal( K_EVENT *const kobj);

/**
 * \brief  Return the number of tasks sleeping on an event.
 * \return Number of sleeping tasks;
 */
UINT kEventQuery( K_EVENT *const kobj);


#if (K_DEF_EVENT_FLAGS==ON)
/**
 * \brief  Set a combination of event flags on an Event object and
 *         wakes any tasks that happens to be waiting on that
 *         combination
 *
 * \param kobj Pointer to the event object
 * \param flagMask Flags to be set
 * \param updatedFlagsPtr Address to return the updated flags
 * \param options TX_OR / TX_AND - bitwise operation to perform over
 * 				  the current Flags
 * \return K_SUCCESS or specific error
 */
K_ERR kEventFlagsSet( K_EVENT *const kobj, ULONG flagMask,
		ULONG* updatedFlagsPtr, ULONG options);
/**
 * \brief Goes to sleep waiting for a combination of events
 *        If they are already met, task proceeds.
 * \param kobj Pointer to the event object
 * \param requiredFlags Combination of flags to be met
 * \param gotFlagsPtr Which flags have been captured as set
 * \param options K_ANY(_CLEAR) to wait for any raised flag
 *                K_ALL(_CLEAR) to wait for all flags to be raised
 *                (_CLEAR) will clear the satisfied bit flags.
 * \param timeout  Suspension time (in ticks)
 * \return K_SUCCESS or specific error
 */
K_ERR kEventFlagsGet( K_EVENT *const kobj, ULONG requiredFlags,
		ULONG* gotFlagsPtr, ULONG options, TICK timeout);

/**
 * \brief 	Returns the current event flags within an event object.
 * \param kobj Address to the event object.
 * \return  ULONG value representing the current flags.
 */
ULONG kEventFlagsQuery(K_EVENT *const kobj);

/* Alias for keeping the Event Semantics if one wishes */
#define kEventFlagsSleep kEventFlagsGet
#define kEventFlagsWake  kEventFlagsSet

#endif

/******************************************************************************
 * CONDITION VARIABLES
 ******************************************************************************/

/**
 * \brief (Helper) Condition Variable Wait. This function must be called
 *        within a mutex critical region when in the need to wait for a
 *        a condition. It atomically put the task to sleep and unlocks
 *        the mutex.
 * \param eventPtr Pointer to event associated to a condition variable.
 * \param mutexPtr Pointer to mutex associated to a condition variable.
 * \param timeout  Suspension timeout.
 * \return K_SUCCESS or specific error
 */

__attribute__((always_inline))
inline K_ERR kCondVarWait( K_EVENT *eventPtr, K_MUTEX *mutexPtr, TICK timeout);

/**
 * \brief The same as kEventSignal - for readability
 * \param eventPtr Pointer to event
 * \return K_SUCCESS or specific error
 */
__attribute__((always_inline))
inline K_ERR kCondVarSignal( K_EVENT *eventPtr);
/**
 * \brief The same as kEventWake (signal broadcast) - for readability
 * \param eventPtr Pointer to event
 * \return K_SUCCESS or specific error
 */
__attribute__((always_inline))
inline K_ERR kCondVarBroad( K_EVENT *eventPtr);

#endif

#if (K_DEF_CALLOUT_TIMER==ON)
/*******************************************************************************
 * APPLICATION TIMER AND DELAY
 ******************************************************************************/
/**
 * \brief Initialises an application timer
 * \param phase Initial phase delay
 * \param funPtr The callback when timer expires
 * \param argsPtr Address to callback function arguments
 * \param reload TRUE for reloading after timer-out. FALSE for an one-shot
 * \return K_SUCCESS/K_ERROR
 */
K_ERR kTimerInit( K_TIMER*, TICK, TICK, CALLOUT, ADDR, BOOL);
#endif

/**
 * \brief Busy-wait a specified delay in ticks.
 *        Task does not suspend and can be preempted by
 *        higher priority tasks.
 * \param delay The delay time in ticks
 */
VOID kBusyDelay( TICK const delay);
/**
 * \brief Put the current task to sleep for a number of ticks.
 *        Task switches to SLEEPING state.
 * \param ticks Number of ticks to sleep
 */
K_ERR kSleep( TICK ticks);

#if (K_DEF_SCH_TSLICE==OFF)

/**
 * \brief	Sleep for an absolute period of time adjusting for
 * 			eventual jitters, suitable for periodic tasks.
 */
VOID kSleepUntil( TICK period);

#endif
/**
 * \brief Gets the current number of  ticks
 * \return Global system tick value
 */
TICK kTickGet( VOID);

#if (K_DEF_ALLOC==ON)
/*******************************************************************************
 * BLOCK MEMORY POOL
 ******************************************************************************/

/**
 * \brief Memory Pool Control Block Initialisation
 * \param kobj Pointer to a pool control block
 * \param memPoolPtr Address of a pool (typically an array) \
 * 		  of objects to be handled
 * \param blkSize Size of each block in bytes
 * \param numBlocks Number of blocks
 * \return K_ERROR/K_SUCCESS
 */
K_ERR kMemInit( K_MEM *const kobj, ADDR const memPoolPtr, BYTE blkSize,
		BYTE const numBlocks);

/**
 * \brief Allocate memory from a block pool
 * \param kobj Pointer to the block pool
 * \return Pointer to the allocated block, or NULL on failure
 */
ADDR kMemAlloc( K_MEM *const kobj);

/**
 * \brief Free a memory block back to the block pool
 * \param kobj Pointer to the block pool
 * \param blockPtr Pointer to the block to free
 * \return Pointer to the allocated memory. NULL on failure.
 */
K_ERR kMemFree( K_MEM *const kobj, ADDR const blockPtr);
#endif
/*******************************************************************************
 * MISC
 ******************************************************************************/
/**
 * \brief Returns the kernel version.
 * \return Kernel version as an unsigned integer.
 */
unsigned int kGetVersion( void);
/**
 * \brief   Deep copy from srcPtr to destPtr
 * \param   srcPtr - address to copy from
 * \param   destPtr - address to copy to
 * \param   size - number of bytes to be copyied
 * \return  Effective number of copyied bytes.
 */
ULONG kMemCpy( ADDR destPtr, ADDR const srcPtr, ULONG size);

/**
 * \brief	Returns the lenght of a string
 * \param s Input string
 * \return  Lenght in bytes
 */
ULONG kStrLen( STRING s);

/* 				*				*				*				*			  */

/* Helpers */
#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

/* Running Task Get */
extern K_TCB *runPtr;
#define K_RUNNING_PID (runPtr->pid)
#define K_RUNNING_PRIO (runPtr->priority)

/* Enable/Disable global interrupts */
/* Note: use this on application-level only.
 * If tweaking kernel code, look at K_CR_*
 * system macros.
 */
__attribute__((always_inline))
static inline VOID kDisableIRQ(VOID)
{
  __ASM volatile ("CPSID I" : : : "memory");
}

__attribute__((always_inline))
static inline VOID kEnableIRQ(VOID)
{
 __ASM volatile ("CPSIE I" : : : "memory");
}


/*[EOF]*/

#endif /* KAPI_H */
