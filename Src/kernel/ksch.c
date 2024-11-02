/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module      :  Nucleus
 * 	Sub-module  :  High-Level Scheduler
 * 	Provides to :  All services
 *  Depends  on :  Low-Level Scheduler
 *
 *  The high-level scheduler is regarded as a sub-module for maintanability
 *  concerns. It's the core service for everything else.
 *
 * 	In this unit:
 * 					o Scheduler routines
 * 		            o Task Queues Management
 *			 		o Task Control Block Management
 *
 *******************************************************************************
 * 		  o Global Ready Queue:
 * 		  The Ready Queue is a table of FIFO queues each one dedicated to a
 * 		  priority level. Status=READY.
 *
 * 		  o Global Sleeping Queue:
 * 		  The Pending Queue is a global single queue where tasks that suspended
 * 		  themselves (either by kPend or kSleepDelay) are placed waiting for a
 * 		  a signal. Status=PENDING/SLEEPING.
 *
 * 		  o Each semaphore/mutex/condition variable/sleep event has its
 * 		  dedicated sleeping queue. Tasks are placed on this queue if the
 * 		  when calling a kWait/kSleep on a kernel object.
 * 		  Status=BLOCKED/SLEEPING.
 *
 * 		  o In both Pending and Sleeping Queues, tasks are removed on the order
 * 		  they entered. If they were removed by priority,
 * 		  lower priority tasks would starve waiting for a resource.
 * 		  Note mutexes implement a priority inheritance protocol.
 *
 * 		  o Once a higher priority task than the running task is made ready it
 * 		  will instantly preempt the running taks.
 *
 *
 ******************************************************************************/

#define K_CODE
#include "ksys.h"

BOOL kSchNeedReschedule(K_TCB *newPtr)
{

	if (newPtr->priority < runPtr->priority)
	{
		return TRUE;
	}
	return FALSE;
}

static inline void kSchFindTask_(void)
{
	PRIO prio = K_PRIO_TYPE_MAX;
	/*TODO: improve this bubble sort*/
	for (prio = highestPrio; prio < NPRIO; prio++)
	{
		if (readyQueue[prio].size > 0)
		{
			nextTaskPrio = prio;
			break;
		}
	}
	return;
}
void kSchSwtch(void)
{

	kSchFindTask_();
	K_TCB *nextRunPtr = NULL;
	kTCBQDeq(&readyQueue[nextTaskPrio], &nextRunPtr);
	if (nextRunPtr == NULL)
	{

		kErrHandler(FAULT_TCB_NULL);
	}
	nextTaskPrio = K_PRIO_TYPE_MAX; /*reset*/
	runPtr = nextRunPtr;
}



/*******************************************************************************
   TASK	QUEUE MANAGEMENT
*******************************************************************************/


K_ERR kTCBQInit(K_TCBQ *const self, STRING listName)
{
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERR_NULL_OBJ;
	}
	return kListInit(self, listName);
}

K_ERR kTCBQEnq(K_TCBQ *const self, K_TCB *const tcbPtr)
{
	if (self == NULL || tcbPtr == NULL)
	{
		return K_ERR_NULL_OBJ;
	}
	return kListAddTail(self, &(tcbPtr->tcbNode));
}

K_ERR kTCBQDeq(K_TCBQ *const self, K_TCB **const tcbPPtr)
{
	if (self == NULL)
	{
		assert(0);
	}
	K_LISTNODE *dequeuedNodePtr = NULL;
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

K_ERR kTCBQRem(K_TCBQ *const self, K_TCB **const tcbPPtr)
{
	if (self == NULL || tcbPPtr == NULL)
	{
		return K_ERROR;
	}
	K_LISTNODE *dequeuedNodePtr = &((*tcbPPtr)->tcbNode);
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

K_TCB* kTCBQPeek(K_TCBQ *const self)
{
	if (self == NULL || self->listDummy.nextPtr == &(self->listDummy))
	{
		return NULL;
	}
	K_LISTNODE *nodePtr = self->listDummy.nextPtr;
	return K_GET_CONTAINER_ADDR(nodePtr, K_TCB, tcbNode);
}

K_TCB* kTCBQSearchPID(K_TCBQ *const self, TID uPid)
{
	if (self == NULL || self->listDummy.nextPtr == &(self->listDummy))
	{
		return NULL;
	}
	else
	{
		K_LISTNODE *currNodePtr = K_LIST_HEAD(self);
		while (currNodePtr != &(self->listDummy))
		{
			K_TCB *currTcbPtr = K_LIST_GET_TCB_NODE(currNodePtr, K_TCB);
			if (currTcbPtr->uPid == uPid)
			{
				return currTcbPtr;
			}
			currNodePtr = currNodePtr->nextPtr;
		}
	}
	return NULL;
}

#define INSTANT_PREEMPT_LOWER_PRIO

K_ERR kReadyQEnq(K_TCB *const tcbPtr)
{
	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(tcbPtr))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return K_ERR_NULL_OBJ;
	}
	if (tcbPtr->realPrio != tcbPtr->priority)
		assert(0);
	if (kTCBQEnq(&readyQueue[tcbPtr->priority], tcbPtr) == K_SUCCESS)
	{
		tcbPtr->status = READY;
		tcbPtr->pendingObj = NULL;
#ifdef INSTANT_PREEMPT_LOWER_PRIO
		if (READY_HIGHER_PRIO(tcbPtr))
		{
			assert(!kTCBQEnq(&readyQueue[runPtr->priority], runPtr));
			runPtr->status = READY;
			K_PEND_CTXTSWTCH
			;
		}
#endif
		K_EXIT_CR
		;
		return K_SUCCESS;
	}
	K_EXIT_CR
	;
	return K_ERROR;
}

K_ERR kReadyQDeq(K_TCB **const tcbPPtr, PRIO priority)
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
/*******************************************************************************
* Task Control Block Management
*******************************************************************************/
static PID pPid = 0; /*<* system pid for each task 	*/

static K_ERR kInitStack_(UINT32* const stackAddrPtr, UINT32 const stackSize,
		TASKENTRY const taskFuncPtr); /*< init stacks */

static K_ERR kInitTcb_(TASKENTRY const taskFuncPtr, UINT32* const stackAddrPtr,
		UINT32 const stackSize);

static K_ERR kInitStack_(UINT32* const stackAddrPtr, UINT32 const stackSize,
		TASKENTRY const taskFuncPtr)
{
	if (stackAddrPtr == NULL || stackSize == 0 || taskFuncPtr == NULL)
	{
		return K_ERROR;
	}
	stackAddrPtr[stackSize - PSR_OFFSET] = 0x01000000; //**PSR**
	stackAddrPtr[stackSize - PC_OFFSET] = (UINT32) taskFuncPtr; //r15 **PC**
	stackAddrPtr[stackSize - LR_OFFSET] = 0x14141414; //r14	**LR**
	stackAddrPtr[stackSize - R12_OFFSET] = 0x12121212; //r12
	stackAddrPtr[stackSize - R3_OFFSET] = 0x03030303; //r3
	stackAddrPtr[stackSize - R2_OFFSET] = 0x02020202; //r2
	stackAddrPtr[stackSize - R1_OFFSET] = 0x01010101; //r1
	stackAddrPtr[stackSize - R0_OFFSET] = 0x00000000; //r0
	stackAddrPtr[stackSize - R11_OFFSET] = 0x11111111; //r11
	stackAddrPtr[stackSize - R10_OFFSET] = 0x10101010; //r10
	stackAddrPtr[stackSize - R9_OFFSET] = 0x09090909; //r9
	stackAddrPtr[stackSize - R8_OFFSET] = 0x08080808; //r8
	stackAddrPtr[stackSize - R7_OFFSET] = 0x07070707; //r7
	stackAddrPtr[stackSize - R6_OFFSET] = 0x06060606; //r6
	stackAddrPtr[stackSize - R5_OFFSET] = 0x05050505; //r5
	stackAddrPtr[stackSize - R4_OFFSET] = 0x04040404; //r4
	/*for (UINT32 j=17; j<=stackSize-1; j++)
	 {
	 if (stackSize-j == 0)
	 stackAddrPtr[stackSize-j]=0xDEADBEEF;
	 else
	 stackAddrPtr[stackSize-j]=0xDEADC0DE;
	 }*/

	return K_SUCCESS;
}

K_ERR kInitTcb_(TASKENTRY const taskFuncPtr, UINT32* const stackAddrPtr,
		UINT32 const stackSize)
{
	if (kInitStack_(stackAddrPtr, stackSize, taskFuncPtr) == K_SUCCESS)
	{
		tcbs[pPid].stackAddrPtr = stackAddrPtr;
		tcbs[pPid].sp = &stackAddrPtr[stackSize - R4_OFFSET];
		tcbs[pPid].stackSize = stackSize;
		tcbs[pPid].status = READY;
		tcbs[pPid].pid = pPid;

		return K_SUCCESS;
	}
	return K_ERROR;
}

K_ERR kCreateTask(TASKENTRY const taskFuncPtr, char const* taskName,
		PID const id, UINT32* const stackAddrPtr, UINT32 const stackSize,
		TICK const timeSlice, PRIO const priority, BOOL const runToCompl)

{
	if (id == 0xFF || id == 0)
	{
		return K_ERR_INVALID_TID;
		assert(0);
	}
	/* if private PID is 0, system tasks hasn't been started yet */
	if (pPid == 0)
	{
		/*init table thap maps tid to pids. mark as 255 all fields, to
		 * indicate they are empty */
		for (UINT32 idx; idx < NTHREADS; idx++)
		{
			tidTbl[idx] = 0xFF;
		}

		/* initialise IDLE TASK */
		assert(kInitTcb_(IdleTask, idleStack, IDLE_STACKSIZE) == K_SUCCESS);

		tcbs[pPid].priority = NPRIO - 1;
		tcbs[pPid].realPrio = NPRIO - 1;
		tcbs[pPid].taskName = "IdleTask";
		tcbs[pPid].uPid = IDLETASK_TID;
		tcbs[pPid].runToCompl = FALSE;
		tcbs[pPid].timeSlice = 0;
		assert(tidTbl[pPid] == 0xFF);
		tidTbl[pPid] = 0;
		pPid += 1;
		/*initialise TIMER HANDLER TASK */
		assert(
				kInitTcb_(TimerHandlerTask, timerHandlerStack, TIMHANDLER_STACKSIZE) == K_SUCCESS);

		tcbs[pPid].priority = 0;
		tcbs[pPid].realPrio = 0;
		tcbs[pPid].taskName = "TimHandlerTask";
		tcbs[pPid].uPid = TIMHANDLER_TID;
		tcbs[pPid].runToCompl = TRUE;
		tcbs[pPid].timeSlice = 0;
		assert(tidTbl[pPid] == 0xFF);
		tidTbl[pPid] = 255;
		pPid += 1;

	}
	if (kInitTcb_(taskFuncPtr, stackAddrPtr, stackSize) == K_SUCCESS)
	{
		if (priority > NPRIO - 1)
		{
			assert(0);
		}

		tcbs[pPid].priority = priority;
		tcbs[pPid].realPrio = priority;
		tcbs[pPid].taskName = taskName;
		tcbs[pPid].timeSlice = timeSlice;
		tcbs[pPid].timeLeft = timeSlice;
		tcbs[pPid].uPid = id;
		tcbs[pPid].runToCompl = runToCompl;
		assert(tidTbl[pPid] == 0xFF);
		tidTbl[pPid] = id;
		pPid += 1;
		return K_SUCCESS;
	}

	return K_ERROR;
}
/*******************************************************************************
* Critical Regions
*******************************************************************************/

UINT32 kEnterCR(VOID)
{
	UINT32 crState;
	crState = __get_PRIMASK();
	if (crState == 0)
	{
		asm volatile("DSB");
		asm volatile ("CPSID I");
		asm volatile("ISB");

		return crState;
	}
	asm volatile("DMB");
	return crState;
}

VOID kExitCR(UINT32 crState)
{
	asm volatile ("ISB");
	__set_PRIMASK(crState);
	asm volatile("DSB");
}

/******************************************************************************
 HELPERS
*******************************************************************************/
PID kGetTaskPID(TID const taskID)
{
	PID pid=0;
	for (pid=0;pid<NTHREADS;pid++)
	{
		if (tidTbl[pid]==taskID)
			break;
	}
	if (pid == NTHREADS)
		assert(0);
	return pid;
}

PRIO kGetTaskPrio(TID const taskID)
{
	PID pid=kGetTaskPID(taskID);
	return tcbs[pid].priority;
}
