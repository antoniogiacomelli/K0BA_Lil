/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
 *
 ******************************************************************************
 ******************************************************************************
 *
 *  Module           : Inter-task Communication
 *  Depends on       : Memory Allocator, Timer
 *  Provides to      : Application
 *  Public API       : Yes
 *
 *  In this unit:
 *  				 Message Passing
 *
 *
 *	NOTE: for messages to be sent from ISRs they must be asynchronous,
 *		  that is, NO TIMEOUT.
 *
 *		  . A Mailbox hold a single 4-byte ADDR messages.
 *
 *		  . Queues hold N 4-byte ADDR messages.
 *
 *		  . Streams hold N fixed-size messages and work with deep copy.
 *		  All above need static memory allocation, but Queues and Mailbox
 *		  might benefit from dynamic allocation if keeping the scope is
 *		  a problem.
 *
 *		  . Pump-Drop Buffers are fully asynchronous mailboxes that
 *		  take care of message integrity with the methods reserve(),
 *		  pump() and drop(). They work with a memory allocator under
 *		  the hood.
 *
 *		  . Port: When a Mailbox, Queue or Stream is set as a 'Port' of
 *		  a task, only that task can receive from that object, others
 *		  can send. Having a unique receiver enables priority inheritance
 *		  on message passing.
 *
 *
 *****************************************************************************/

#define K_CODE

#include "kexecutive.h"

/*******************************************************************************
 * MAILBOXES (EXCHANGE)
 ******************************************************************************/
#if (K_DEF_MBOX==ON)
/*
 * a mailbox holds an ADDR variable as a mail
 * it can initialise full (non-null) or empty (NULL)
 * */

K_ERR kMboxInit( K_MBOX *const kobj, ADDR initMailPtr)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		K_CR_EXIT
		return (K_ERROR);
	}
	kobj->mailPtr = initMailPtr;
	K_ERR listerr;
	listerr = kListInit( &kobj->waitingQueue, "mailq");
	kassert( listerr == 0);
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = MAILBOX;
#if(K_DEF_MBOX_POSTPEND_PRIO_INH==ON)
	kobj->serverTask = NULL;
	kobj->clientTask = NULL;
#endif
	kobj->init = TRUE;
	K_CR_EXIT
	return (K_SUCCESS);
}

K_ERR kMboxPost( K_MBOX *const kobj, ADDR const sendPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (IS_BLOCK_ON_ISR( timeout))
	{
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if ((kobj == NULL) || (sendPtr == NULL))
	{
		KFAULT( FAULT_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	/* mailbox is full  */
	if (kobj->mailPtr != NULL)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_FULL);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}
		/* not-empty blocks a writer */
#if(K_DEF_MBOX_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
			if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
				kRemoveTimeoutNode( &kobj->timeoutNode);
		}
#if (K_DEF_MBOX_POSTPEND_PRIO_INH==ON)
		if (kobj->clientTask != NULL)
		{
			kobj->serverTask = runPtr;
			if (kobj->clientTask->priority > runPtr->priority)
			{
				runPtr->priority = kobj->clientTask->priority;
			}
		}
#endif
	}
	kobj->mailPtr = sendPtr;
	/*  full: unblock a reader, if any */
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeReadPtr;
		freeReadPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeReadPtr->status == RECEIVING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeReadPtr);
			kassert( freeReadPtr != NULL);
			kTCBQEnq( &readyQueue[freeReadPtr->priority], freeReadPtr);
			freeReadPtr->status = READY;
			if (freeReadPtr->priority < runPtr->priority)
				K_PEND_CTXTSWTCH
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

#if (K_DEF_FUNC_MBOX_POSTOVW==(ON))
K_ERR kMboxPostOvw( K_MBOX *const kobj, ADDR const sendPtr)
{
	K_CR_AREA
	K_CR_ENTER

	if ((kobj == NULL) || (sendPtr == NULL))
	{
		KFAULT( FAULT_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	if (kobj == NULL)
	{
		kobj->mailPtr = sendPtr;
		/*  full: unblock a reader, if any */
		if (kobj->waitingQueue.size > 0)
		{
			K_TCB *freeReadPtr;
			freeReadPtr = kTCBQPeek( &kobj->waitingQueue);
			if (freeReadPtr->status == RECEIVING)
			{
				kTCBQDeq( &kobj->waitingQueue, &freeReadPtr);
				kassert( freeReadPtr != NULL);
				kTCBQEnq( &readyQueue[freeReadPtr->priority], freeReadPtr);
				freeReadPtr->status = READY;
				if (freeReadPtr->priority < runPtr->priority)
					K_PEND_CTXTSWTCH
			}
		}
	}
	else
	{
		kobj->mailPtr = sendPtr;
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

K_ERR kMboxPend( K_MBOX *const kobj, ADDR *recvPPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (IS_BLOCK_ON_ISR( timeout))
	{
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if ((kobj == NULL) || (recvPPtr == NULL))
	{
		KFAULT( FAULT_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
	if (kobj->mailPtr == NULL)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_EMPTY);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}

#if (K_DEF_MBOX_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = RECEIVING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	*recvPPtr = kobj->mailPtr;
	kobj->mailPtr = NULL;
	/* it only makes sense do deq a writer */
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeWriterPtr;
		freeWriterPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeWriterPtr->status == SENDING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeWriterPtr);
			kTCBQEnq( &readyQueue[freeWriterPtr->priority], freeWriterPtr);
			freeWriterPtr->status = READY;
			if ((freeWriterPtr->priority < runPtr->priority))
				K_PEND_CTXTSWTCH
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
#if (K_DEF_FUNC_MBOX_ISFULL==ON)
BOOL kMboxIsFull( K_MBOX *const kobj)
{
	return ((kobj->mailPtr == NULL) ? FALSE : TRUE );
}
#endif

#if (K_DEF_FUNC_MBOX_PEEK==ON)
K_ERR kMboxPeek( K_MBOX *const kobj, ADDR *peekPPtr)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL || peekPPtr == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (!kobj->init)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (kobj->mailPtr == NULL)
	{
		K_CR_EXIT
		return (K_ERR_MBOX_EMPTY);
	}
	*peekPPtr = kobj->mailPtr;
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

#if (K_DEF_FUNC_MBOX_POSTPEND==ON)
/*
 * This method cannot be issued from an ISR, even with no timeout
 * Optionally the server might inherit the LOWER priority of a
 * client, as explained on the Docbook
 * It can be used only on 1-1 communication, other than that it
 * will not work out
 ***/
K_ERR kMboxPostPend( K_MBOX *const kobj, ADDR const sendPtr,
		ADDR *const recvPPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kIsISR())
	{
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if ((kobj == NULL) || (sendPtr == NULL))
	{
		KFAULT( FAULT_OBJ_NULL);
	}
	if (kobj->init == FALSE)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
	}
#if (K_DEF_MBOX_POSTPEND_PRIO_INH==ON)
	kobj->clientTask = runPtr;
#endif
	/* a reader is yet to read */
	if (kobj->mailPtr != NULL)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_FULL);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}

		/* not-empty blocks a writer */
#if (K_DEF_MBOX_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);

	}
	kobj->mailPtr = sendPtr;
	/*  full: unblock a reader, if any */
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeReadPtr;
		freeReadPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeReadPtr->status == RECEIVING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeReadPtr);
			kassert( freeReadPtr != NULL);
			kTCBQEnq( &readyQueue[freeReadPtr->priority], freeReadPtr);
			freeReadPtr->status = READY;
		}
		/* do not pend here */
	}
	/* will pend after waiting for a recv */

	if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
	{
		kTimeOut( &kobj->timeoutNode, timeout);
	}

#if (K_DEF_MBOX_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
	kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
	/* pend for reply with timeout parameter */
	runPtr->status = RECEIVING;
	K_PEND_CTXTSWTCH
	K_CR_EXIT
	K_CR_ENTER
	if (runPtr->timeOut)
	{
		runPtr->timeOut = FALSE;
		K_CR_EXIT
		return (K_ERR_TIMEOUT);
	}
	if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
		kRemoveTimeoutNode( &kobj->timeoutNode);

	*recvPPtr = kobj->mailPtr;
	kobj->mailPtr = NULL; /* empty again */
#if (K_DEF_MBOX_POSTPEND_PRIO_INH==ON)
	kobj->clientTask->priority = kobj->clientTask->realPrio;
	kobj->serverTask->priority = kobj->serverTask->realPrio;
	kobj->clientTask = NULL;
	kobj->serverTask = NULL;
#endif
	K_CR_EXIT
	return (K_SUCCESS);

}
#endif /* sendrecv */

#endif /* mailbox */

/*******************************************************************************
 * MAIL QUEUE
 ******************************************************************************/
#if (K_DEF_QUEUE==(ON))
K_ERR kQueueInit( K_QUEUE *const kobj, ADDR memPtr, ULONG maxItems)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL || memPtr == NULL || maxItems == 0)
	{
		KFAULT( FAULT_OBJ_NULL);
		return (K_ERROR);
	}
	kobj->mailQPtr = memPtr;
	kobj->headIdx = 0;
	kobj->tailIdx = 0;
	kobj->maxItems = maxItems;
	kobj->countItems = 0;
	kobj->init = TRUE;
	K_ERR listerr = kListInit( &kobj->waitingQueue, "qq");
	kassert( listerr == 0);
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = QUEUE;
	K_CR_EXIT
	return (K_SUCCESS);
}

K_ERR kQueuePost( K_QUEUE *const kobj, ADDR sendPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (IS_BLOCK_ON_ISR( timeout))
	{
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}

	if (kobj == NULL || sendPtr == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		return (K_ERROR);
	}
	if (!kobj->init)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
		return (K_ERROR);
	}
	if (kobj->countItems == kobj->maxItems)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_FULL);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}

#if(K_DEF_QUEUE_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	/* post on tail */
	/* cast to ULONG guarantees a 4-byte step-size for the address */
	ADDR *tailAddr = (ADDR*) ((ULONG*) kobj->mailQPtr + kobj->tailIdx);
	/* sendPtr is enqueued at tailAddr */
	*tailAddr = sendPtr;
	kobj->tailIdx = (kobj->tailIdx + 1) % kobj->maxItems;
	kobj->countItems++;
	/* unblock a receiver if any */
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeReadPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeReadPtr->status == RECEIVING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeReadPtr);
			kTCBQEnq( &readyQueue[freeReadPtr->priority], freeReadPtr);
			freeReadPtr->status = READY;
			if (freeReadPtr->priority < runPtr->priority)
			{
				K_PEND_CTXTSWTCH
			}
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

K_ERR kQueuePend( K_QUEUE *const kobj, ADDR *recvPPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL || recvPPtr == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (IS_BLOCK_ON_ISR( timeout))
	{
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if (!kobj->init)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (kobj->countItems == 0)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_EMPTY);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}
#if(K_DEF_QUEUE_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = RECEIVING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	/* cast to ULONG* guarantees a 4-byte step-size */
	ADDR *headAddr = (ADDR*) ((ULONG*) kobj->mailQPtr + kobj->headIdx);
	/* value stored on headAddr is dequeued */
	*recvPPtr = *headAddr;
	kobj->headIdx = (kobj->headIdx + 1) % kobj->maxItems;
	kobj->countItems--;
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeSendPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeSendPtr->status == SENDING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeSendPtr);
			kTCBQEnq( &readyQueue[freeSendPtr->priority], freeSendPtr);
			freeSendPtr->status = READY;
			if (freeSendPtr->priority < runPtr->priority)
			{
				K_PEND_CTXTSWTCH
			}
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

#if (K_DEF_FUNC_QUEUE_PEEK==ON)
K_ERR kQueuePeek( K_QUEUE *const kobj, ADDR *peekPPtr)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL || peekPPtr == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (!kobj->init)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
		K_CR_EXIT
		return (K_ERROR);
	}
	ADDR *headAddr = (ADDR*) ((ULONG*) kobj->mailQPtr + kobj->headIdx);
	*peekPPtr = *headAddr;
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

#if (K_DEF_FUNC_QUEUE_JAM==ON)
K_ERR kQueueJam( K_QUEUE *const kobj, ADDR sendPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if (kobj == NULL || sendPtr == NULL)
	{
		KFAULT( FAULT_OBJ_NULL);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (!kobj->init)
	{
		KFAULT( FAULT_OBJ_NOT_INIT);
		K_CR_EXIT
		return (K_ERROR);
	}
	if (IS_BLOCK_ON_ISR( timeout))
	{

		KFAULT( FAULT_INVALID_ISR_PRIMITVE);

	}
	if (kobj->countItems == kobj->maxItems)
	{
		if (timeout == 0)
		{
			K_CR_EXIT
			return (K_ERR_MBOX_FULL);
		}
		if ((timeout > 0) && (timeout < K_WAIT_FOREVER))
		{
			kTimeOut( &kobj->timeoutNode, timeout);
		}
#if(K_DEF_QUEUE_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	/* empty or wrapped ? just place. otherwise, get back - to match the tail */
	kobj->headIdx = (kobj->headIdx == 0) ? (0) : (kobj->headIdx - 1);
	ADDR *putAddr = (ADDR*) ((ULONG*) kobj->mailQPtr + kobj->headIdx);
	*putAddr = sendPtr;
	kobj->countItems++;
	/* unblock a receiver if any */
	if (kobj->waitingQueue.size > 0)
	{
		K_TCB *freeReadPtr = kTCBQPeek( &kobj->waitingQueue);
		if (freeReadPtr->status == RECEIVING)
		{
			kTCBQDeq( &kobj->waitingQueue, &freeReadPtr);
			kTCBQEnq( &readyQueue[freeReadPtr->priority], freeReadPtr);
			freeReadPtr->status = READY;
			if (freeReadPtr->priority < runPtr->priority)
			{
				K_PEND_CTXTSWTCH
			}
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

#if (K_DEF_FUNC_QUEUE_MAILCOUNT==ON)

ULONG kQueueMailCount( K_QUEUE *const kobj)
{
	return (kobj->countItems);
}
#endif

#if (K_DEF_FUNC_QUEUE_ISFULL==ON)
BOOL kQueueIsFull( K_QUEUE *const kobj)
{
	return (kobj->countItems == kobj->maxItems);
}

#endif

#endif

/*******************************************************************************
 * MESSAGE STREAM
 *******************************************************************************/
#if(K_DEF_STREAM==ON)

K_ERR kStreamInit( K_STREAM *const kobj, ADDR const buffer,
		ULONG const mesgSize, ULONG const nMesg)

{
	K_CR_AREA
	K_CR_ENTER
	if ((kobj == NULL) || (buffer == NULL))
	{
		K_CR_EXIT
		return (K_ERR_OBJ_NULL);
	}
	if (mesgSize == 0)
	{
		K_CR_EXIT
		return (K_ERR_INVALID_MESG_SIZE);
	}
	if (nMesg == 0)
	{
		K_CR_EXIT
		return (K_ERR_INVALID_QUEUE_SIZE);
	}
	kobj->buffer = buffer;
	kobj->mesgSize = mesgSize;
	kobj->maxMesg = nMesg;
	kobj->mesgCnt = 0;
	kobj->readIndex = 0;
	kobj->writeIndex = 0;
	K_ERR err = kListInit( &kobj->waitingQueue, "waitingQueue");
	if (err != 0)
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	kobj->timeoutNode.nextPtr = NULL;
	kobj->timeoutNode.timeout = 0;
	kobj->timeoutNode.kobj = kobj;
	kobj->timeoutNode.objectType = STREAM;
	kobj->init = 1;
	K_CR_EXIT
	return (K_SUCCESS);
}

#if (K_DEF_FUNC_STREAM_PEEK==ON)
K_ERR kStreamPeek( K_STREAM *const kobj, ADDR recvPtr)
{
	K_CR_AREA
	K_CR_ENTER
	if ((kobj == NULL) || (recvPtr == NULL) || (kobj->init == 0))
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	if (kobj->mesgCnt == 0)
	{
		K_CR_EXIT
		return (K_ERR_STREAM_EMPTY);
	}
	BYTE const *src = (BYTE*)kobj->buffer + (kobj->readIndex * kobj->mesgSize);
	BYTE *dest = (BYTE*) recvPtr;
	ULONG err = 0;
	CPYQ( dest, src, kobj->mesgSize, err);
	if (err != kobj->mesgSize)
	{
		kobj->readIndex = (kobj->readIndex + 1) % kobj->maxMesg;
		K_CR_EXIT
		return (K_ERR_MESG_CPY);
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

K_ERR kStreamSend( K_STREAM *const kobj, ADDR const sendPtr, TICK const timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if ((kobj == NULL) || (sendPtr == NULL) || (kobj->init == 0))
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	if (IS_BLOCK_ON_ISR( timeout))
	{
		K_CR_EXIT
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if (kobj->mesgCnt >= kobj->maxMesg) /*full*/
	{
		if (timeout == K_NO_WAIT)
		{
			K_CR_EXIT
			return (K_ERR_STREAM_FULL);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kTimeOut( &kobj->timeoutNode, timeout);

#if (K_DEF_STREAM_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	BYTE *dest = (BYTE*)kobj->buffer + (kobj->writeIndex * kobj->mesgSize);
	BYTE const *src = (BYTE const*) sendPtr;
	ULONG err = 0;
	CPYQ( dest, src, kobj->mesgSize, err);
	if (err != kobj->mesgSize)
	{
		K_CR_EXIT
		return (K_ERR_MESG_CPY);
	}
	kobj->writeIndex = (kobj->writeIndex + 1) % kobj->maxMesg;
	kobj->mesgCnt++;
	/* was empty ?*/
	if ((kobj->waitingQueue.size > 0) && (kobj->mesgCnt == 1))
	{
		K_TCB *freeTaskPtr;
		kTCBQDeq( &kobj->waitingQueue, &freeTaskPtr);
		kTCBQEnq( &readyQueue[freeTaskPtr->priority], freeTaskPtr);
		freeTaskPtr->status = READY;
		if (freeTaskPtr->priority < runPtr->priority)
		{
			K_PEND_CTXTSWTCH
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

K_ERR kStreamRecv( K_STREAM *const kobj, ADDR recvPtr, TICK const timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if ((kobj == NULL) || (recvPtr == NULL) || (kobj->init == 0))
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	if (IS_BLOCK_ON_ISR( timeout))
	{
		K_CR_EXIT
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if (kobj->mesgCnt == 0)
	{
		if (timeout == K_NO_WAIT)
		{
			K_CR_EXIT
			return (K_ERR_STREAM_EMPTY);
		}

		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kTimeOut( &kobj->timeoutNode, timeout);
#if (K_DEF_STREAM_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = RECEIVING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	BYTE const *src = (BYTE*)kobj->buffer + (kobj->readIndex * kobj->mesgSize);
	BYTE *dest = (BYTE*) recvPtr;
	ULONG err = 0;
	CPYQ( dest, src, kobj->mesgSize, err);
	if (err != kobj->mesgSize)
	{
		K_CR_EXIT
		return (K_ERR_MESG_CPY);
	}
	kobj->readIndex = (kobj->readIndex + 1) % kobj->maxMesg;
	kobj->mesgCnt--;

	/* was full ? unblock */
	if ((kobj->waitingQueue.size > 0) && (kobj->mesgCnt == (kobj->maxMesg - 1)))
	{
		K_TCB *freeTaskPtr;
		kTCBQDeq( &kobj->waitingQueue, &freeTaskPtr);
		kTCBQEnq( &readyQueue[freeTaskPtr->priority], freeTaskPtr);
		freeTaskPtr->status = READY;

		if (freeTaskPtr->priority < runPtr->priority)
		{
			K_PEND_CTXTSWTCH
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}

#if (K_DEF_FUNC_STREAM_JAM == ON)
K_ERR kStreamJam( K_STREAM *const kobj, ADDR const sendPtr, TICK timeout)
{
	K_CR_AREA
	K_CR_ENTER
	if ((kobj == NULL) || (sendPtr == NULL) || (kobj->init == 0))
	{
		K_CR_EXIT
		return (K_ERROR);
	}
	if (IS_BLOCK_ON_ISR( timeout))
	{
		K_CR_EXIT
		KFAULT( FAULT_INVALID_ISR_PRIMITVE);
	}
	if (kobj->mesgCnt >= kobj->maxMesg) /*full*/
	{
		if (timeout == K_NO_WAIT)
		{
			K_CR_EXIT
			return (K_ERR_STREAM_FULL);
		}
		if (timeout > K_NO_WAIT && timeout < K_WAIT_FOREVER)
			kTimeOut( &kobj->timeoutNode, timeout);
#if (K_DEF_STREAM_ENQ==K_DEF_ENQ_FIFO)
			kTCBQEnq(&kobj->waitingQueue, runPtr);
#else
		kTCBQEnqByPrio( &kobj->waitingQueue, runPtr);
#endif
		runPtr->status = SENDING;
		K_PEND_CTXTSWTCH
		K_CR_EXIT
		K_CR_ENTER
		if (runPtr->timeOut)
		{
			runPtr->timeOut = FALSE;
			K_CR_EXIT
			return (K_ERR_TIMEOUT);
		}
		if ((timeout > K_NO_WAIT) && (timeout < K_WAIT_FOREVER))
			kRemoveTimeoutNode( &kobj->timeoutNode);
	}
	kobj->readIndex =
			(kobj->readIndex == 0) ?
					(kobj->maxMesg - 1) : (kobj->readIndex - 1);
	BYTE *dest = (BYTE*)kobj->buffer + (kobj->readIndex * kobj->mesgSize);
	BYTE const *src = (BYTE const*) sendPtr;
	ULONG err = 0;
	CPYQ( dest, src, kobj->mesgSize, err);
	if (err != kobj->mesgSize)
	{
		/* restore the read index on failure */
		kobj->readIndex = (kobj->readIndex + 1) % kobj->maxMesg;
		K_CR_EXIT
		return (K_ERR_MESG_CPY);
	}
	/*succeded */
	kobj->mesgCnt++;
	/*was empty? there're waiting readers */
	if ((kobj->mesgCnt == 1) && (kobj->waitingQueue.size > 0))
	{
		K_TCB *freeTaskPtr;
		kTCBQDeq( &kobj->waitingQueue, &freeTaskPtr);
		kTCBQEnq( &readyQueue[freeTaskPtr->priority], freeTaskPtr);
		freeTaskPtr->status = READY;
		if (freeTaskPtr->priority < runPtr->priority)
		{
			K_PEND_CTXTSWTCH
		}
	}
	K_CR_EXIT
	return (K_SUCCESS);
}
#endif

#if (K_DEF_FUNC_STREAM_MESGCOUNT==ON)
K_ERR kStreamGetMesgCount( K_STREAM *const kobj, UINT *const mesgCntPtr)
{
	K_CR_AREA
	if (kobj)
	{
		K_CR_ENTER
		*mesgCntPtr = kobj->mesgCnt;
		K_CR_EXIT
		return (K_SUCCESS);
	}
	return (K_ERR_OBJ_NULL);
}
#endif

#endif /*K_DEF_STREAM*/

#if (K_DEF_PDMESG == ON)

/******************************************************************************
 * PUMP-DROP QUEUE (CYLICAL ASYNCHRONOUS BUFFERS)
 ******************************************************************************
 * Producer:
 * reserve - write - pump
 *
 * Consumer:
 * fetch - read - drop
 *
 **/

K_ERR kPDMesgInit( K_PDMESG *const kobj, K_MEM *const memCtrlPtr,
		K_PDBUF *bufPool, BYTE nBufs)
{
	K_CR_AREA
	K_CR_ENTER
	K_ERR err = K_ERROR;
	kobj->memCtrlPtr = memCtrlPtr;
	err = kMemInit( kobj->memCtrlPtr, bufPool, sizeof(K_PDBUF), nBufs);
	if (!err)
	{
		/* nobody is using anything yet */
		kobj->currBufPtr = NULL;
		kobj->init = TRUE;
	}
	K_CR_EXIT
	return (err);
}

K_PDBUF* kPDMesgReserve( K_PDMESG *const kobj)
{

	K_CR_AREA
	K_CR_ENTER
	K_PDBUF *allocPtr = NULL;
	if ((kobj->currBufPtr == NULL))
	{
		allocPtr = kMemAlloc( kobj->memCtrlPtr);
	}
	else if ((kobj->currBufPtr->nUsers == 0) && (kobj->currBufPtr != NULL))
	{
		allocPtr = kobj->currBufPtr;
	}

	if (!allocPtr)
	{
		kobj->failReserve++;
	}
	K_CR_EXIT
	return (allocPtr);
}

K_ERR kPDMesgPump( K_PDMESG *const kobj, K_PDBUF *const buffPtr)
{
	K_CR_AREA
	if (!kobj->init)
		return (K_ERR_OBJ_NOT_INIT);
	/* first pump will skip this if */
	K_CR_ENTER
	if ((kobj->currBufPtr != NULL) && (kobj->currBufPtr != buffPtr)
			&& (kobj->currBufPtr->nUsers == 0))
	{ /*deallocate curr buf if not null and not used */
		K_ERR err = kMemFree( kobj->memCtrlPtr, kobj->currBufPtr);
		if (err)
		{
			K_CR_EXIT
			return (err);
		}
	}
	/* replace current buffer */
	kobj->currBufPtr = buffPtr;
	K_CR_EXIT
	return (K_SUCCESS);
}

K_PDBUF* kPDMesgFetch( K_PDMESG *const kobj)
{
	K_CR_AREA
	if (!kobj->init)
		return (NULL);
	if (kobj->currBufPtr)
	{
		K_CR_ENTER
		K_PDBUF *ret = kobj->currBufPtr;
		kobj->currBufPtr->nUsers++;
		K_CR_EXIT
		return (ret);
	}
	return (NULL);
}

K_ERR kPDMesgDrop( K_PDMESG *const kobj, K_PDBUF *const bufPtr)
{
	if (!kobj->init)
		return (K_ERR_OBJ_NULL);
	K_ERR err = 0;
	if (bufPtr)
	{
		K_CR_AREA
		K_CR_ENTER
		if (bufPtr->nUsers > 0)
			bufPtr->nUsers--;
		/* deallocate if not used and not the curr buf */
		if ((bufPtr->nUsers == 0) && (kobj->currBufPtr != bufPtr))
		{
			err = kMemFree( kobj->memCtrlPtr, bufPtr);
		}
		K_CR_EXIT
		return (err);
	}
	return (K_ERR_OBJ_NULL);
}

#endif
