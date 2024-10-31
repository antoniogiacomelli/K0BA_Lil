/*******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 *******************************************************************************
 *******************************************************************************
 * 	In this unit:
 *
 * 		o Task Queues Management
 *
 *******************************************************************************
 * 		  o Ready Queue:
 * 		  The Ready Queue is a table of FIFO queues. Each index is a priority:
 *
 * 		  		[][][][][][][][] <- highest priority tasks queue (0)
 * 		  		[][][][][][][][]
 *                     .
 *                     .
 *                     .
 *              [][][][][][][][] <- lowest priority tasks queue (NPRIO-1)
 *              --------------->
 *              First-In-Fist-Out
 *
 *
 * 		  o Sleeping Queue:
 * 		  The Sleeping Queue is a single queue where tasks who SLEEPING them
 * 		  selves by pending or sleeping are placed. Since they depend on other
 * 		  tasks to be removed from the queue, and placed on the ready queue, a
 * 		  single Sleeping Queue is enough.
 *
 * 		  Note each semaphore/mutex/condition variable/sleep event has its
 * 		  dedicated sleeping queue and tasks are removed by the order they
 * 		  entered. If they were ordered by priority, lower priority tasks
 * 		  would always starve waiting for a resource.
 *
 * 		  Once a higher priority task than the running task is made ready it
 * 		  will instantly preempt it.
 *
 *
 ******************************************************************************/




/*******************************************************************************
 *
 * TASK	QUEUE MANAGEMENT
 *
 ******************************************************************************/
#define K_CODE
#include <kapi.h>
#define INSTANT_PREEMPT_LOWER_PRIO

K_ERR kTCBQInit(K_TCBQ* const self, STRING listName)
{
    if (IS_NULL_PTR(self))
    {
    	kErrHandler(FAULT_NULL_OBJ);
        return K_ERR_NULL_OBJ;
    }
    return kListInit(self, listName);
}

K_ERR kTCBQEnq(K_TCBQ* const self, K_TCB* const tcbPtr)
{
    if (self == NULL || tcbPtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    return kListAddTail(self, &(tcbPtr->tcbNode));
}

K_ERR kTCBQDeq(K_TCBQ* const self, K_TCB** const tcbPPtr)
{
    if (self == NULL)
    {
        assert(0);
    }
    K_LISTNODE* dequeuedNodePtr = NULL;
    K_ERR retVal = kListRemoveHead(self, &dequeuedNodePtr);

    if (retVal != K_SUCCESS)
    {
        kErrHandler(FAULT_LIST);
    }
    *tcbPPtr = K_LIST_GET_TCB_NODE(dequeuedNodePtr, K_TCB);

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
    K_ERR retVal = kListRemove(self, dequeuedNodePtr);
    if (retVal != K_SUCCESS)
    {
        kErrHandler(FAULT_LIST);
        return retVal;
    }
    *tcbPPtr = K_LIST_GET_TCB_NODE(dequeuedNodePtr, K_TCB);
    if (*tcbPPtr == NULL)
    {
        kErrHandler(FAULT_TCB_NULL);
        return K_ERR_NULL_OBJ;
    }
    return K_SUCCESS;
}

K_TCB* kTCBQPeek(K_TCBQ* const self)
{
    if (self == NULL || self->listDummy.nextPtr == &(self->listDummy))
    {
        return NULL;
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
			K_TCB* currTcbPtr = K_LIST_GET_TCB_NODE(currNodePtr, K_TCB);
			if (currTcbPtr->uPid == uPid)
			{
				return currTcbPtr;
			}
			currNodePtr = currNodePtr->nextPtr;
		}
	}
	return NULL;
}

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
		}
#endif
		K_EXIT_CR;
		return K_SUCCESS;
	}
	K_EXIT_CR;
	return K_ERROR;
}

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
