/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module      : Inter-task Communication
 *  Depends on  : Inter-task Synchronisation
 *  Provides to : Application and System Tasks
 *  Public API  : Yes
 *
 * 	In this unit:
 * 					o System Message Buffers pool Management
 *					o Mailbox
 *					o Message Queue
 *					o Pipes
 *					o FIFOs
 *
 *****************************************************************************/

#define K_CODE
#include "kglobals.h"

#if (K_DEF_MESGQ==ON)
/*****************************************************************************
 *
 * MESSAGE POOL
 *
 ****************************************************************************/
K_BLOCKPOOL mesgBuffMem; /* global mesg pool control block */
K_MESGBUFF mesgBuffPool[K_DEF_N_MESGBUFF]; /* global mesg pool */
K_SEMA semaMesgCntr;

K_MESGBUFF* kMesgBuffGet()
{
	K_CR_AREA;
	K_ENTER_CR
	;
	kSemaWait(&semaMesgCntr);
	K_MESGBUFF* mesgPtr = kBlockPoolAlloc(&mesgBuffMem);
	K_EXIT_CR
	;
	return mesgPtr;
}

K_ERR kMesgBuffPut(K_MESGBUFF* const self)
{

	K_CR_AREA;
	K_ENTER_CR
	;
	if (!kBlockPoolFree(&mesgBuffMem, (ADDR) self))
	{
		kSemaSignal(&semaMesgCntr);
		K_EXIT_CR
		;
		return K_SUCCESS;
	}
	K_EXIT_CR
	;
	return K_ERROR;
}

/*******************************************************************************
 * INDIRECT BLOCKING MESSAGE QUEUE
 *******************************************************************************/

K_ERR kMesgQInit(K_MESGQ* const self, ADDR const mesgPoolPtr,
		BYTE const queueSize, BYTE const mesgSize)
{
	K_CR_AREA;
	K_ENTER_CR;

	if (IS_NULL_PTR(self) || IS_NULL_PTR(mesgPoolPtr))
	{
		return K_ERR_NULL_OBJ;
	}
	if (queueSize == 0)
	{
		return K_ERR_INVALID_Q_SIZE;
	}
	if (mesgSize == 0)
	{
		return K_ERR_INVALID_QMESG_SIZE;
	}

	kSemaInit(&(self->semaItem), 0);
	kMutexInit(&(self->mutex));
	kListInit(&self->mesgList, "MesgList");
	kSemaInit(&self->semaRoom, queueSize);
	kBlockPoolInit(&(self->mesgMemCtrlBlk), (ADDR )mesgPoolPtr, (BYTE )mesgSize, \
	queueSize);
	K_EXIT_CR;
	return K_SUCCESS;
}

K_ERR kMesgQPut(K_MESGQ* const self, ADDR const mesgPtr, BYTE const mesgSize)
{
	if (IS_NULL_PTR(self) ||IS_NULL_PTR(mesgPtr))
		kErrHandler(FAULT_NULL_OBJ);
	kSemaWait(&self->semaRoom);
	kMutexLock(&(self->mutex));
	K_CR_AREA;
	K_ENTER_CR
	;
	K_MESGBUFF* mesgBuffPtr = kMesgBuffGet();
	mesgBuffPtr->mesgPtr = (ADDR) kBlockPoolAlloc(&self->mesgMemCtrlBlk);
	if (mesgBuffPtr->mesgPtr ==NULL)
	{
		K_EXIT_CR
		;
		return K_ERR_MEM_ALLOC;
	}
	kMemCpy((ADDR) mesgBuffPtr->mesgPtr, mesgPtr, mesgSize);
	mesgBuffPtr->senderTid = runPtr->uPid;
	mesgBuffPtr->mesgSize = mesgSize;
	K_EXIT_CR
	;
	kListAddTail(&(self->mesgList), &mesgBuffPtr->mesgNode);
	kMutexUnlock(&(self->mutex));
	kSemaSignal(&(self->semaItem));
	return K_SUCCESS;
}

PID kMesgQGet(K_MESGQ* const self, ADDR const rcvdMesgPtr)
{
	if (IS_NULL_PTR(self) ||IS_NULL_PTR(rcvdMesgPtr))
		kErrHandler(FAULT_NULL_OBJ);
	kSemaWait(&self->semaItem);
	kMutexLock(&(self->mutex));
	K_CR_AREA;
	K_ENTER_CR
	;
	K_LISTNODE* nodePtr;
	kListRemoveHead(&self->mesgList, &nodePtr);
	K_MESGBUFF* mesgBuffPtr = K_LIST_GET_MESGBUFFER_NODE(nodePtr);
	kMemCpy(rcvdMesgPtr, (ADDR) mesgBuffPtr->mesgPtr, mesgBuffPtr->mesgSize);
	PID senderPid = mesgBuffPtr->senderTid;
	assert(!(kBlockPoolFree(&self->mesgMemCtrlBlk, mesgBuffPtr->mesgPtr)));
	kMesgBuffPut(mesgBuffPtr);
	K_EXIT_CR
	;
	kMutexUnlock(&(self->mutex));
	kSemaSignal(&self->semaRoom);

	return senderPid;
}

#endif /*K_DEF_MESGQ*/

/*******************************************************************************
 * FULLY SYNCHRONOUS MAILBOX
 *******************************************************************************/
#if (K_DEF_MAILBOX==ON)

K_ERR kMailboxInit(K_MAILBOX* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return K_ERROR;
	}

	kMutexInit(&(self->mutex)); /* data init as free to access */
	kSemaInit(&(self->semaEmpty), 1); /* mailbox init as empty */
	kSemaInit(&(self->semaFull), 0); /* mailbox init not full */
	kSemaInit(&(self->semaAck), 0); /* no ack yet*/
	self->mail.mailPtr = &(self->mail.mail[0]);
	K_EXIT_CR
	;
	return K_SUCCESS;
}
K_ERR kMailboxPost(K_MAILBOX* self, ADDR const mailPtr, SIZE const mailSize)
{
	if (IS_NULL_PTR(self) ||IS_NULL_PTR(mailPtr))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERR_NULL_OBJ;
	}
	kSemaWait(&self->semaEmpty); /* wait it is empty */
	kMutexLock(&self->mutex); /* lock, and write */
	K_CR_AREA;
	K_ENTER_CR
	;
	kMemCpy(self->mail.mailPtr, mailPtr, mailSize);
	self->mail.senderTid = runPtr->uPid;
	self->mail.mailSize = mailSize;
	K_EXIT_CR
	;
	kSemaSignal(&self->semaFull); /* mark as full */
	kSemaWait(&self->semaAck); /* wait for an ack to unlock */
	kMutexUnlock(&self->mutex);
	return K_SUCCESS;
}

TID kMailboxPend(K_MAILBOX* const self, ADDR const recvMailPtr)
{
	if (IS_NULL_PTR(self) ||IS_NULL_PTR(recvMailPtr))
	{
		kErrHandler(FAULT_NULL_OBJ);
	}
	kSemaWait(&self->semaFull); /* wait there is an item */
	kSemaSignal(&self->semaAck); /* ack sender, so it unlocks the mutex */
	kMutexLock(&self->mutex); /* lock to receive */
	K_CR_AREA;
	K_ENTER_CR
	;
	kMemCpy(recvMailPtr, self->mail.mailPtr, self->mail.mailSize);
	K_EXIT_CR
	;
	kSemaSignal(&self->semaEmpty); /*  mark as empty */
	kMutexUnlock(&self->mutex); /*  then, unlock */
	return self->mail.senderTid;
}
#endif
/*******************************************************************************
 * PIPES
 *******************************************************************************/
#if (K_DEF_PIPE==ON)

VOID kPipeInit(K_PIPE* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	self->head = 0;
	self->tail = 0;
	self->room = K_DEF_PIPE_SIZE;
	self->data = 0;
	kMutexInit(&(self->mutex));
	kCondInit(&(self->condData));
	kCondInit(&(self->condRoom));
}

INT32 kPipeRead(K_PIPE* const self, BYTE* destPtr, UINT32 nBytes)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	UINT32 readBytes = 0;
	if (nBytes ==0)
		return 0;

	while (nBytes) /* while there still bytes to be read  */
	{
		kMutexLock(&(self->mutex));
		while (self->data) /* while there is data */
		{
			*destPtr++ = self->buffer[self->tail++]; /* read from the tail  */
			self->tail %= K_DEF_PIPE_SIZE; /* wrap around 	    */
			self->data--; /* decrease data	    */
			self->room++; /* increase room	    */
			readBytes++; /* increase read bytes */
			nBytes--; /* decrease asked bytes number	*/

			if (nBytes ==0) /* read all bytes asked */
				break; /* break reading loop 	*/
		}
		kMutexUnlock(&(self->mutex));
		kCondWake(&(self->condRoom)); /*either there is no more data or all req data was read*/

		if (readBytes >0)
		{
			return readBytes; /* return number of read bytes	*/
		}
		/* if here, no data was read */
		kCondWait(&(self->condData)); /* wait for data from writers */
	}
	return 0;
}

INT32 kPipeWrite(K_PIPE* const self, BYTE* srcPtr, UINT32 nBytes)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	UINT32 writeBytes = 0;
	if (nBytes ==0)
		return 0;
	if (nBytes <=0)
		return 0;
	while (nBytes) /* while there still bytes to be written */
	{
		kMutexLock(&(self->mutex));
		while (self->room) /* while there is room */
		{
			self->buffer[self->head++] = *srcPtr;
			self->head %= K_DEF_PIPE_SIZE;
			self->data++;
			self->room--;
			writeBytes++;
			srcPtr++;
			nBytes--;
			if (nBytes ==0)
			{
				break;
			}
		}
		kMutexUnlock(&(self->mutex));
		if (writeBytes)
		{
			kCondWake(&(self->condData));
		}
		if (nBytes ==0) /* finished writing */
		{
			return writeBytes;
		}
		kCondWait(&(self->condRoom)); /*still data to write, but no room*/
	}
	return 0;
}
#endif /*K_DEF_PIPES*/

/******************************************************************************
 *
 * FIFOS
 *
 ******************************************************************************/
K_ERR kFifoInit(K_FIFO* const self)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERROR;
	}
	self->head = 0;
	self->tail = 0;
	kSemaInit(&self->semaRoom, K_DEF_FIFO_SIZE);
	kSemaInit(&self->semaItem, 0);
	kMutexInit(&self->mutex);
	for (SIZE idx = 0; idx <K_DEF_FIFO_SIZE; ++idx)
		self->buffer[idx] = 0;

	return K_SUCCESS;
}

K_ERR kFifoPut(K_FIFO* const self, BYTE const data)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERROR;
	}
	kSemaWait(&self->semaRoom);
	kMutexLock(&self->mutex);
	self->buffer[self->head] = data; /* put on head */
	self->head = (self->head +1) %K_DEF_FIFO_SIZE;
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaItem);
	return K_SUCCESS;
}
BYTE kFifoGet(K_FIFO* const self)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
	}
	BYTE data;
	kSemaWait(&self->semaItem);
	kMutexLock(&self->mutex);
	data = self->buffer[self->tail]; /* get from tail */
	self->tail = (self->tail +1) %K_DEF_FIFO_SIZE;
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaRoom);
	return data;
}
