/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o System Message Buffers pool Management
 *					o Mailbox
 *					o Message Queue
 *					o Pipes
 *					o FIFOs
 *
 *****************************************************************************/

#include <kapi.h>

K_MEM   		 mesgBuffMem; 			 /* global mesg pool control block */
K_MESGBUFF	  	 mesgBuffPool[N_MSGBUFF]; /* global mesg pool */
K_SEMA			 semaMesgCntr;



/*****************************************************************************
 *
 * MESSAGE POOL
 *
 ****************************************************************************/

K_ERR kMesgBuffPoolInit(VOID)
{
	K_ERR retVal = kMemInit(&mesgBuffMem, mesgBuffPool, MSGBUFF_SIZE, N_MSGBUFF);
	kSemaInit(&semaMesgCntr, N_MSGBUFF);
	return retVal;
}

K_MESGBUFF* kMesgBuffGet()
{
	K_CR_AREA;
	K_ENTER_CR;
	kSemaWait(&semaMesgCntr);
	K_MESGBUFF* mesgPtr = kMemAlloc(&mesgBuffMem);
	K_EXIT_CR;
	return mesgPtr;
}

K_ERR kMesgBuffPut(K_MESGBUFF* const self)
{

	K_CR_AREA;
	K_ENTER_CR;
	if (!kMemFree(&mesgBuffMem, (ADDR)self))
	{
		kSemaSignal(&semaMesgCntr);
		K_EXIT_CR;
		return K_SUCCESS;
	}
	K_EXIT_CR;
	return K_ERROR;
}


/******************************************************************************
 *
 * INDIRECT MAILBOX WITH EXTENDED RENDEZVOUS
 *
 ****************************************************************************/
K_ERR kMailboxInit(K_MAILBOX* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERROR;
	}
	assert(!kMutexInit(&(self->mutex))); /* data init as free to access */
	assert(!kSemaInit(&(self->semaEmpty), 1)); /* mailbox init as empty */
	assert(!kSemaInit(&(self->semaFull), 0));  /* mailbox init not full */
#if (K_CONF_MAILBOX_ACK==ON)
	assert(!kSemaInit(&(self->semaAck), 0));
#endif
	K_EXIT_CR;
	return K_SUCCESS;
}
K_ERR kMailboxPost(K_MAILBOX* self, const ADDR mesgPtr, SIZE mesgSize)
{
	if (IS_NULL_PTR(self) || IS_NULL_PTR(mesgPtr))
		kErrHandler(FAULT_NULL_OBJ);

	assert(!kSemaWait(&self->semaEmpty)); /*D EMPTY*/
	assert(!kMutexLock(&self->mutex));
	K_CR_AREA;
	K_ENTER_CR;
	kMemCpy((ADDR)&self->mail.mesgPtr, mesgPtr, mesgSize);
	self->mail.senderTid = runPtr->uPid;
	self->mail.mesgSize = mesgSize;
	K_EXIT_CR;
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaFull);
	kSemaWait(&self->semaAck);
	/*D ACK*/
	return K_SUCCESS;
}

PID kMailboxPend(K_MAILBOX* const self, ADDR rcvdMesgPtr)
{
	if (IS_NULL_PTR(self) || IS_NULL_PTR(rcvdMesgPtr))
		kErrHandler(FAULT_NULL_OBJ);
	kSemaWait(&self->semaFull);
	kSemaSignal(&self->semaAck); /*U ACK */
	kMutexLock(&self->mutex);
	K_CR_AREA;
	K_ENTER_CR;
	kMemCpy(rcvdMesgPtr, (ADDR)&self->mail.mesgPtr, self->mail.mesgSize);
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaEmpty);
	K_EXIT_CR;
#if(K_CONF_MAILBOX_ACK == ON)
	return  self->mail.senderTid;
#endif

}
/****************************************************************************
 *
 * INDIRECT BLOCKING MESSAGE QUEUE
 *
 ****************************************************************************/

VOID kMesgQInit(K_MESGQ* const self, ADDR mesgPoolPtr, BYTE queueSize,
				BYTE mesgSize)
{
	K_CR_AREA;
	K_ENTER_CR;
	assert(kSemaInit(&(self->semaItem), 0) == K_SUCCESS);
	assert(kMutexInit(&(self->mutex)) == K_SUCCESS);
	assert(kListInit(&self->mesgList, "MesgList") == K_SUCCESS);
	kSemaInit(&self->semaRoom, queueSize);
	kMemInit(&(self->mesgMemCtrlBlk), (ADDR)mesgPoolPtr, (BYTE)mesgSize,
			queueSize);


	K_EXIT_CR;
}

K_ERR kMesgQPut(K_MESGQ* const self, ADDR mesgPtr, BYTE mesgSize)
{
	if (IS_NULL_PTR(self) || IS_NULL_PTR(mesgPtr))
		kErrHandler(FAULT_NULL_OBJ);
	kSemaWait(&self->semaRoom);
	kMutexLock(&(self->mutex));
	K_CR_AREA;
	K_ENTER_CR;
	K_MESGBUFF* mesgBuffPtr = kMesgBuffGet();
	mesgBuffPtr->mesgPtr = (ADDR)kMemAlloc(&self->mesgMemCtrlBlk);
	if (mesgBuffPtr->mesgPtr == NULL)
		assert(0);
	kMemCpy((ADDR)mesgBuffPtr->mesgPtr, mesgPtr, mesgSize);
	mesgBuffPtr->senderTid = runPtr->uPid;
	mesgBuffPtr->mesgSize = mesgSize;
	K_EXIT_CR;
	kListAddTail(&(self->mesgList), &mesgBuffPtr->mesgNode);
	kMutexUnlock(&(self->mutex));
	kSemaSignal(&(self->semaItem));
	return K_SUCCESS;
}

PID kMesgQGet(K_MESGQ* const self, ADDR rcvdMesgPtr)
{
	if (IS_NULL_PTR(self) || IS_NULL_PTR(rcvdMesgPtr))
		kErrHandler(FAULT_NULL_OBJ);
	kSemaWait(&self->semaItem);
	kMutexLock(&(self->mutex));
	K_CR_AREA;
	K_ENTER_CR;
	K_LISTNODE* nodePtr;
	kListRemoveHead(&self->mesgList, &nodePtr);
	K_MESGBUFF* mesgBuffPtr =  GET_MSG_BUFFER_FROM_NODE(nodePtr);
	kMemCpy(rcvdMesgPtr, (ADDR)mesgBuffPtr->mesgPtr, mesgBuffPtr->mesgSize);
	PID senderPid = mesgBuffPtr->senderTid;
	assert(!(kMemFree(&self->mesgMemCtrlBlk,  mesgBuffPtr->mesgPtr)));
	kMesgBuffPut(mesgBuffPtr);
	K_EXIT_CR;
	kMutexUnlock(&(self->mutex));
	kSemaSignal(&self->semaRoom);

	return senderPid;
}


/******************************************************************************
 *
 * PIPE/FIFO
 *
 ******************************************************************************/

VOID kPipeInit(K_PIPE* const self)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	self->head=0;
	self->tail=0;
	self->room=PIPE_SIZE;
	self->data=0;
	kMutexInit(&(self->mutex));
	kCondInit(&(self->condData));
	kCondInit(&(self->condRoom));
}

INT32 kPipeRead(K_PIPE* const self, BYTE* destPtr, UINT32 nBytes)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	UINT32 readBytes = 0;
	if (nBytes == 0)
		return 0;

	while (nBytes)	/* while there still bytes to be read  */
	{
		kMutexLock(&(self->mutex));
		while(self->data)  /* while there is data */
		{
			*destPtr++ = self->buffer[self->tail++]; /* read from the tail  */
			self->tail %= PIPE_SIZE;  /* wrap around 	    */
			self->data--;		  /* decrease data	    */
			self->room++;		  /* increase room	    */
			readBytes++;		  /* increase read bytes */
			nBytes--;		  /* decrease asked bytes number	*/

			if (nBytes==0)		 /* read all bytes asked */
				break;		 /* break reading loop 	*/
		}
		kMutexUnlock(&(self->mutex));
		kCondWake(&(self->condRoom)); /*either there is no more data or all req data was read*/

		if (readBytes > 0)
		{
			return readBytes; /* return number of read bytes	*/
		}
		/* if here, no data was read */
		kCondWait(&(self->condData));	/* wait for data from writers */
	}
	return 0;
}

INT32 kPipeWrite(K_PIPE* const self, const BYTE* srcPtr, UINT32 nBytes)
{
	if (IS_NULL_PTR(self))
		kErrHandler(FAULT_NULL_OBJ);
	UINT32 writeBytes = 0;
	if (nBytes == 0)
		return 0;
	if (nBytes <= 0)
		return 0;
	while (nBytes)	/* while there still bytes to be written */
	{
		kMutexLock(&(self->mutex));
		while(self->room)  /* while there is room */
		{
			self->buffer[self->head++] = *srcPtr;
			self->head %= PIPE_SIZE;
			self->data++;
			self->room--;
			writeBytes++;
			srcPtr++;
			nBytes--;
			if (nBytes==0)
			{
				break;
			}
		}
		kMutexUnlock(&(self->mutex));
		if (writeBytes)
		{
			kCondWake(&(self->condData));
		}
		if (nBytes==0)	/* finished writing */
		{
			return writeBytes;
		}
		kCondWait(&(self->condRoom)); /*still data to write, but no room*/
	}
	return 0;
}

K_ERR kFifoInit(K_FIFO* const self)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERROR;
	}
	self->head = 0;
	self->tail = 0;
	kSemaInit(&self->semaRoom, FIFO_SIZE);
	kSemaInit(&self->semaItem, 0);
	kMutexInit(&self->mutex);
	for (SIZE idx=0; idx<FIFO_SIZE; ++idx)
		self->buffer[idx] = 0;

	return K_SUCCESS;
}

K_ERR kFifoPut(K_FIFO* self, BYTE data)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERROR;
	}
	kSemaWait(&self->semaRoom);
	kMutexLock(&self->mutex);
	self->buffer[self->head] = data; /* put on head */
	self->head = (self->head + 1) % FIFO_SIZE;
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaItem);
	return K_SUCCESS;
}
BYTE kFifoGet(K_FIFO* self)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
	}
	BYTE data;
	kSemaWait(&self->semaItem);
	kMutexLock(&self->mutex);
	data = self->buffer[self->tail]; /* get from tail */
	self->tail = (self->tail + 1) % FIFO_SIZE;
	kMutexUnlock(&self->mutex);
	kSemaSignal(&self->semaRoom);
	return data;
}
