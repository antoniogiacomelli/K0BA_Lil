/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 ******************************************************************************/

/**
 * \file     kapi.h
 * \brief    Kernel API
 * \version  0.3.1
 * \author   Antonio Giacomelli
 *
 *
 * \verbatim
 *  ________________________________________
 * |                                        |--
 * | [TASK1][TASK2]  ....         [TASKN]   | application.c
 * |----------------------------------------|--
 * |                 API                    |
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
 * By default, it is placed in "application.h"
 *
 *
 **/

#ifndef KAPI_H
#define KAPI_H

#include "kconfig.h"

#if (CUSTOM_ENV==1)
#include "kenv.h"
#endif

#include "ktypes.h"
#include "kobjs.h"
#include "kversion.h"

/******************************************************************************/

/**
 * \brief 			   Create a new task.
 * \param taskFuncPtr  Pointer to the task entry function.
 *
 * \param taskName     Task name. Keep it as much as 8 Bytes.
 *
 * \param id           user-defined Task ID - valid range: 1-254
 *                     (0 and 255 are reserved).
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
K_ERR kCreateTask
		(TASKENTRY const taskFuncPtr,
		STRING taskName,
		TID const taskID,
        INT* const stackAddrPtr,
		UINT32 const stackSize,
#if(K_DEF_SCH_TSLICE==ON)
        TICK const timeSlice,
#endif
		PRIO const priority,
		BOOL const runToCompl
		);


/**
 * \brief Initialises the kernel. To be called in main()
 *        after hardware initialisation and task creation.
 *
 */
VOID kInit(VOID);

/**
 * \brief Yields the current task.
 *
 */
VOID kYield(VOID);
/**
 * \brief			A task changes its priority
 * \param newPrio   New priority value
 * \return			K_SUCCESS or specific error
 */
K_ERR kTaskChangePrio(PRIO newPrio);

/**
 * \brief			A task restore its original priority
  * \return			K_SUCCESS or specific error
 */
K_ERR kTaskRestorePrio(VOID);
/*******************************************************************************
 SEMAPHORES
 *******************************************************************************/
#if (K_DEF_SEMA==ON)
/**
 *\brief       Initialise a semaphore
 *\param kobj  Semaphore address
 *\param value Initial value
 *\return      \see ktypes.h
 */

K_ERR kSemaInit(K_SEMA* const kobj, INT32 const value);

/**
 *\brief 			Wait on a semaphore
 *\param kobj 		Semaphore address
 *\param timeout	Maximum suspension time
 */
K_ERR kSemaWait(K_SEMA* const kobj, TICK const timeout);

/**
 *\brief Signal a semaphore
 *\param kobj Semaphore address
 */
VOID kSemaSignal(K_SEMA* const kobj);

/**
 *\brief Return the counter's value of a semaphore
 *\param kobj Semaphore address
 *\return      Counter's value
 */
INT32 kSemaQuery(K_SEMA* const kobj);

#endif
/*******************************************************************************
 * MUTEX
 *******************************************************************************/
#if (K_DEF_MUTEX==ON)
/**
 *\brief Init a mutex
 *\param kobj mutex address
 *\return K_SUCCESS / K_ERROR
 */

K_ERR kMutexInit(K_MUTEX* const kobj);

/**
 *\brief Lock 		a mutex
 *\param kobj 		mutex address
 *\param timeout	Maximum suspension time
 *\return K_SUCCESS or a specific error \see ktypes.h
 */
K_ERR kMutexLock(K_MUTEX* const kobj, TICK timeout);

/**
 *\brief Unlock a mutex
 *\param kobj mutex address
 */
VOID kMutexUnlock(K_MUTEX* const kobj);

/**
 * \brief Return the state of a mutex (locked/unlocked)
 */
K_ERR kMutexQuery(K_MUTEX* const kobj);

#endif

/*******************************************************************************
 * MAILBOX
 *******************************************************************************/

#if (K_DEF_MBOX == ON)

#if (K_DEF_MBOX_TYPE==EXCHANGE)
/**
 * \brief               Initialises an indirect single mailbox.
 *
 * \param kobj          Mailbox address.
 * \param initMail		If initialising full, address of initial mail.
 * \					Otherwise NULL.
 * \return              K_SUCCESS or specific error.
 */

K_ERR kMboxInit(K_MBOX *const kobj, ADDR initMail);
/**
 * \brief               Send to a mailbox. Task blocks when full.
 * \param kobj          Mailbox address.
 * \param sendPtr       Mail address.
 * \param timeout		Suspension time-out
 * \return              K_SUCCESS or specific error.
 */

#if (K_DEF_MBOX_SENDRECV==ON)

/**
 * \brief               Send and receive from the same mailbox.
 * \param kobj          Mailbox address.
 * \param sendPtr		Address of sending message.
 * \param recvPPtr      Address to store the response. (can be &sendPtr)
  * \param timeout		Suspension time-out
 * \return				K_SUCCESS or specific error.
 */
K_ERR kMboxPostPend(K_MBOX *const kobj, ADDR const sendPtr, ADDR* const recvPPtr,
		TICK timeout);

/**
 * \brief   Check if a mailbox is full.
 * \return  TRUE or FALSE.
 */
BOOL kMboxIsFull(K_MBOX *const kobj);

#endif

#elif (K_DEF_MBOX_TYPE==QUEUE)

/**
 * \brief				Initialises an indirect multi-item mailbox.
 * \param kobj			Mailbox address.
 * \param memPPtr		Pointer-to-pointer to the mailbox memory.
 * \param maxItems		Maximum number of items.
 * \return
 */
K_ERR kMboxInit(K_MBOX *const kobj, ADDR *memPPtr, SIZE maxItems);

/**
 * \brief   Get the number of mails on a mailbox.
 * \return  Number of mails.
 */
SIZE kMboxMailCount(K_MBOX *const kobj);


#endif

/*Post and Pend methods are common to both modes */
K_ERR kMboxPost(K_MBOX *const kobj, ADDR const sendPtr, TICK timeout);
/**
 * \brief               Receive from a mailbox. Block if empty.
 *
 * \param kobj          Mailbox address.
 * \param recvPPtr      Address that will store the message address (pointer-to-pointer).
 * \param timeout		Suspension time-out
 * \return				K_SUCCESS or specific error.
 */
K_ERR kMboxPend(K_MBOX *const kobj, ADDR* recvPPtr, TICK timeout);

#if (K_DEF_FUNC_MBOX_PEEK==ON)
/**
 * \brief 			   Reads the message on head of queue
 * 					   without extracting it.
 * \param kobj		   Mailbox address.
 * \param peekPPtr	   Pointer to receive address.
 * \return			   K_SUCCESS or specific error.
 */
K_ERR kMboxPeek(K_MBOX *const kobj, ADDR *peekPPtr);
#endif

#if (K_DEF_FUNC_MBOX_MAILCOUNT==ON)
/**
 * \brief			  Get the number of mails within a queue
 * \param kobj
 * \return
 */
SIZE kMboxMailCount(K_MBOX *const kobj);
#endif



#endif

/******************************************************************************/
/* MESSAGE STREAM (PIPE/MESSAGE QUEUE)                                        */
/******************************************************************************/
#if (K_DEF_MESGQ == ON)
/**
 *\brief 			Initialise a Message Queue
 *\param kobj		Queue address
 *\param buffer		Allocated memory. It must be enough for the queue capacity
 *\					that is messsageSize*maxMessages
 *\param messageSize Message size
 *\param maxMessage  Max number of messages
 *\return 			 K_SUCCESS or specific errors
 */
K_ERR kMesgQInit(K_MESGQ *const kobj, ADDR buffer, SIZE messageSize,
		SIZE maxMessages);

#if (K_DEF_FUNC_MESGQ_MESGCOUNT==ON)

/**
 *\brief 			Get the current number of messages within a message queue.
 *\param kobj		Queue address
 *\param mesgCntPtr Address to store the message number
 *\return			K_SUCCESS or a specific error.
 */

K_ERR kMesgQGetMesgCount(K_MESGQ *const kobj, UINT32 *const mesgCntPtr);

#endif

#if (K_DEF_FUNC_MESGQ_JAM == ON)

/**
 *\brief 			Sends a message to the queue front.
 *\param kobj		Queue address
 *\param sendPtr	Message address
 *\param timeout	Suspension time
 *\return			K_SUCCESS or specific error
 */
K_ERR kMesgQJam(K_MESGQ *const kobj, ADDR const sendPtr, TICK timeout);

#endif

/**
 *\brief 			Receive a message from the queue
 *\param kobj		Queue address
 *\param recvPtr	Receiving address
 *\param Timeout	Suspension time
 */
K_ERR kMesgQRecv(K_MESGQ *const kobj, ADDR recvPtr, TICK timeout);

/**
 *\brief 			Send a message to a queue
 *\param kobj		Queue address
 *\param recvPtr	Message address
 *\param Timeout	Suspension time
 */
K_ERR kMesgQSend(K_MESGQ *const kobj, ADDR const sendPtr, TICK timeout);

#if (K_DEF_FUNC_MESGQ_PEEK==ON)

/**
*\brief 			Receive the front message of a queue
*					without changing its state
*\param	kobj		Message Queue object address
*\param	recvPtr		Receiving pointer address
*\return			K_SUCCESS or error.
*/
K_ERR kMesgQPeek(K_MESGQ *const kobj, ADDR recvPtr);

#endif

#if (K_DEF_FUNC_MESGQ_RESET==ON)
/**
 * \brief			Reset queue indexes and message counter.
 * \param kobj		Queue address
 * \return			K_SUCCESS or K_ERR_OBJ_NULL
 */
K_ERR kMesgQReset(K_MESGQ* kobj);

#endif

#endif /*K_DEF_MESGQ*/

/*******************************************************************************
 * PUMP-DROP QUEUE (CYCLIC ASYNCHRONOUS BUFFERS - CABs)
 *******************************************************************************/

#if (K_DEF_PDMESG == ON)

/**
 * \brief          Pump-drop queue initialisation.
 *
 * \param kobj    	    PD queue address.
 * \param memCtrlPtr	Pointer to the memory allocator control block
 * \param bufPool  		Pool of PD Buffers, statically allocated.
 * \param nBufs    		Number of buffers for this queue.
 * \return         see ktypes.h
 */
K_ERR kPDMesgInit(K_PDMESG* const kobj, K_MEM* const memCtrlPtr, K_PDBUF* const bufPool, BYTE const nBufs);

/**
 * \brief          Reserves a pump-drop buffer before writing on it.
 *
 * \param kobj     Queue address.
 * \return         see ktypes.h
 */
K_PDBUF* kPDMesgReserve(K_PDMESG* const kobj);


/**
 * \brief           Writes into a PD buffer the source address and the size
 *                  of a data message.
 *
 * \param bufPtr    Buffer address.
 * \param srcPtr    Message address.
 * \param dataSize  Message size.
 * \return
 */
K_ERR kPDBufWrite(K_PDBUF* bufPtr, ADDR srcPtr, SIZE dataSize);

/**
 * \brief          Pump a buffer into the queue - make it available for readers.
 *
 * \param kobj     Queue address.
 * \return         Address of a written PD buffer.
 */
K_ERR kPDMesgPump(K_PDMESG* const kobj, K_PDBUF* bufPtr);

/**
 * \brief          Fetches the most recent buffer pumped in the queue.
 *
 * \param kobj     Queue address.
 * \return         Address of a PD buffer available for reading.
 */
K_PDBUF* kPDMesgFetch(K_PDMESG* const kobj);

/**
 * \brief          Copies the message from a PD Buffer to a chosen address.
 *
 * \param bufPtr   Address of the PD buffer.
 * \param destPtr  Address that will store the message.
 * \return         see ktypes.h
 */
K_ERR kPDBufRead(K_PDBUF* const bufPtr, ADDR destPtr);

/**
 * \brief         Called by reader to indicate it has consumed the
 *                message from a buffer. If there are no more readers
 *                using the buffer, and it is not the last buffer pumped
 *                in the queue, it will be reused.
 *
 * \param kobj    Queue address;
 * \return        see ktypes.h
 */
K_ERR kPDMesgDrop(K_PDMESG* const kobj, K_PDBUF* const bufPtr);


#endif

/******************************************************************************
 * DIRECT TASK SIGNAL
 ******************************************************************************/
/**
 * \brief A caller task goes to a PENDING state, waiting for a kSignal.
 */
VOID kPend(VOID);

/**
 * \brief Direct Signal a task. Target task is resu
 *        med.
 * \param taskID The ID of the task to signal
 */
K_ERR kSignal(TID const taskID);

/**
 * \brief Suspends a lower priority task.
 * 		  The task must be READY if issued by another task
 * 		  or RUNNING/READY if issued by an ISR.
 * \param taskID User-assigned task ID
 * \return K_SUCCESS or specific error
 */
K_ERR kSuspend(TID const id);

/******************************************************************************
 * EVENTS
 ******************************************************************************/

#if (K_DEF_SLEEPWAKE==ON)
/**
 * \brief 			Initialise an event
 * \param kobj		Pointer to K_EVENT object
 * \return			K_SUCCESS/error
 */
K_ERR kEventInit(K_EVENT* const kobj);
/**
 * \brief 			Suspends a task waiting for a specific event
 * \param kobj 		Pointer to a K_EVENT object
 * \param timeout	Suspension time.
 */
K_ERR kEventSleep(K_EVENT* const kobj, TICK timeout);

/**
 * \brief Wakes all tasks sleeping for a specific event
 * \param kobj Pointer to a K_EVENT object
 */
VOID kEventWake(K_EVENT* const kobj);

/**
 * \brief Wakes a single task sleeping for a specific event
 *        (by priority)
 * \param kobj Pointer to a K_EVENT object
 */
VOID kEventSignal(K_EVENT* const kobj);


/**
 * \brief  Return the number of tasks sleeping on an event.
 */
UINT32 kEventQuery(K_EVENT* const kobj);

#endif

/*******************************************************************************
 * APPLICATION TIMER AND DELAY
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



K_ERR kTimerInit(STRING timerName, TICK const ticks, CALLOUT const funPtr,
        ADDR const argsPtr, BOOL const reload);

/**
 * \brief Busy-wait a specified delay in ticks.
 *        Task does not suspend.
 * \param delay The delay time in ticks
 */
VOID kBusyDelay(TICK const delay);
/**
 * \brief Put the current task to sleep for a number of ticks.
 *        Task switches to SLEEPING state.
 * \param ticks Number of ticks to sleep
 */
VOID kSleep(TICK const ticks);

#if (K_DEF_SCH_TSLICE==OFF)
/**
 * \brief	Sleep for an absolute period of time adjusting for
 * 			eventual jitters, suitable for periodic tasks.
 */
VOID kSleepUntil(TICK period);
#endif
/**
 * \brief Gets the current number of  ticks
 * \return Global system tick value
 */
TICK kTickGet(VOID);

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
K_ERR kMemInit(K_MEM* const kobj, ADDR const memPoolPtr,
        BYTE blkSize, BYTE const numBlocks);

/**
 * \brief Allocate memory from a block pool
 * \param kobj Pointer to the block pool
 * \return Pointer to the allocated block, or NULL on failure
 */
ADDR kMemAlloc(K_MEM* const kobj);

/**
 * \brief Free a memory block back to the block pool
 * \param kobj Pointer to the block pool
 * \param blockPtr Pointer to the block to free
 * \return Pointer to the allocated memory. NULL on failure.
 */
K_ERR kMemFree(K_MEM* const kobj, ADDR const blockPtr);

/*******************************************************************************
 * MISC
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
 * \brief Returns the kernel version.
 * \return Kernel version as an unsigned integer.
 */
unsigned int kGetVersion(void);
/**
 * \brief   Deep copy from srcPtr to destPtr
 * \param   srcPtr - address to copy from
 * \param   destPtr - address to copy to
 * \param   size - number of bytes to be copyied
 * \return  Effective number of copyied bytes.
 */
SIZE kMemCpy(ADDR destPtr, ADDR const srcPtr, SIZE size);


/* 				*				*				*				*			  */


/* Helpers */
#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

/* Running Task Get */
extern K_TCB* runPtr;
#define K_RUNNING_TID (runPtr->uPid)
#define K_RUNNING_PID (runPtr->pid)
#define K_RUNNING_PRIO (runPtr->priority)


/*[EOF]*/

#endif /* KAPI_H */
