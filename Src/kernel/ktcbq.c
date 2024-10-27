/*
 * ktcbq.c
 *
 *  Created on: Oct 21, 2024
 *      Author: anton
 */

#define INSTANT_PREEMPT_LOWER_PRIO


/*******************************************************************************
 *
 * TASK	QUEUE MANAGEMENT
 *
 ******************************************************************************/

#include <kapi.h>
K_ERR kTCBQInit(K_TCBQ* const self, STRING listName)
{
    if (IS_NULL_PTR(self))
    {
    	kErrHandler(FAULT_NULL_OBJ);
        return K_ERR_NULL_OBJ;
    }

    return kListInit(self, listName);  // Initialize the list with size tracking
}


K_ERR kTCBQEnq(K_TCBQ* const self, K_TCB* const tcbPtr)
{
    if (self == NULL || tcbPtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }

    return kListAddTail(self, &(tcbPtr->tcbNode));  // Use the generic kListInsert function
}
K_ERR kTCBQDeq(K_TCBQ* const self, K_TCB** const tcbPPtr)
{
    if (self == NULL)
    {
        assert(0);  // You may want to replace assert with a different error handler
    }

    K_LISTNODE* dequeuedNodePtr = NULL;
    K_ERR retVal = kListRemoveHead(self, &dequeuedNodePtr);  // Dequeue the first item

    if (retVal != K_SUCCESS)
    {
        kErrHandler(FAULT_LIST);
    }

    // Retrieve the TCB from the list head node using the K_GET_CONTAINER_TYPE_PTR macro
    *tcbPPtr = K_GET_TCB_ADDR(dequeuedNodePtr, K_TCB);

    if (*tcbPPtr == NULL)
    {
        kErrHandler(FAULT_TCB_NULL);
        return K_ERR_NULL_OBJ;
    }

    return K_SUCCESS;
}

K_ERR kTCBQRem(K_TCBQ* const self, K_TCB** const tcbPPtr)
{
    if (self == NULL || tcbPPtr == NULL)
    {
        return K_ERROR;
    }

    K_LISTNODE* dequeuedNodePtr = &((*tcbPPtr)->tcbNode);
    K_ERR retVal = kListRemove(self, dequeuedNodePtr);  // Remove the specific node

    if (retVal != K_SUCCESS)
    {
        kErrHandler(FAULT_LIST);
        return retVal;
    }

    // Retrieve the TCB from the list head node using the K_GET_CONTAINER_TYPE_PTR macro
    *tcbPPtr = K_GET_TCB_ADDR(dequeuedNodePtr, K_TCB);

    if (*tcbPPtr == NULL)
    {
        kErrHandler(FAULT_TCB_NULL);
        return K_ERR_NULL_OBJ;
    }

    return K_SUCCESS;
}
// Peek the first element in the queue
K_TCB* kTCBQPeek(K_TCBQ* const self)
{
    if (self == NULL || self->listDummy.nextPtr == &(self->listDummy))
    {
        return NULL;  // Return NULL if the list is empty
    }

    K_LISTNODE* nodePtr = self->listDummy.nextPtr;
    return K_GET_CONTAINER_ADDR(nodePtr, K_TCB, tcbNode);
}

K_TCB* kTCBQSearchPID(K_TCBQ* const self, PID uPid)
{
	if (self == NULL || self->listDummy.nextPtr == &(self->listDummy))
    {
	        return NULL;
    }
	else
	{
		K_LISTNODE* currNodePtr = K_LIST_HEAD(self);
		while(currNodePtr != &(self->listDummy))
		{
			K_TCB* currTcbPtr = K_GET_TCB_ADDR(currNodePtr, K_TCB);
			if (currTcbPtr->uPid == uPid)
			{
				return currTcbPtr;
			}
			currNodePtr = currNodePtr->nextPtr;
		}
	}
	return NULL;
}


// Enqueue task to the ready queue
K_ERR kReadyQEnq(K_TCB* const tcbPtr)
{
	K_CR_AREA;
	K_ENTER_CR;
	if (IS_NULL_PTR(tcbPtr))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR;
		return K_ERR_NULL_OBJ;
	}
	if (tcbPtr->realPrio != tcbPtr->priority)
		assert(0);
	if (kTCBQEnq(&readyQueue[tcbPtr->priority], tcbPtr) == K_SUCCESS)
	{
		tcbPtr->status = READY;
		tcbPtr->pendingObj=NULL;
#ifdef INSTANT_PREEMPT_LOWER_PRIO
		if (READY_HIGHER_PRIO(tcbPtr))
		{
			K_PEND_CTXTSWTCH;
			K_EXIT_CR;
		}
#endif
		return K_SUCCESS;
	}
	K_EXIT_CR;
	return K_ERROR;
}

// Dequeue task from the ready queue
K_ERR kReadyQDeq(K_TCB** const tcbPPtr, PRIO priority)
{
	if (kTCBQDeq(&readyQueue[priority], tcbPPtr) == K_SUCCESS)
	{

		return K_SUCCESS;
	}
	else
	{
		kErrHandler(FAULT_TCB_NULL);
		return K_ERROR;
	}

}
