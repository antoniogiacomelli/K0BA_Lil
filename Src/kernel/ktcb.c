/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 *
 * 		o Task Control Block Management
 *
 *****************************************************************************/


#define K_CODE
#include "kapi.h"
#include "ksystasks.h"

static PID pPid=0;	/*<* system pid for each task 	*/

static K_ERR kInitStack_(UINT32* const stackAddrPtr, const UINT32 stackSize,
		const TASKENTRY taskFuncPtr); /*< init stacks */


static K_ERR kInitTcb_(TASKENTRY const taskFuncPtr, UINT32* const stackAddrPtr,
		UINT32 const stackSize);


static K_ERR kInitStack_(UINT32* const stackAddrPtr,
		const UINT32 stackSize, const TASKENTRY taskFuncPtr)
{
	if (stackAddrPtr == NULL || stackSize == 0 ||taskFuncPtr == NULL)
	{
		return K_ERROR;
	}
	stackAddrPtr[stackSize-PSR_OFFSET]	= 0x01000000; //**PSR**
	stackAddrPtr[stackSize-PC_OFFSET]	= (UINT32)taskFuncPtr; //r15 **PC**
	stackAddrPtr[stackSize-LR_OFFSET]	= 0x14141414; //r14	**LR**
	stackAddrPtr[stackSize-R12_OFFSET]	= 0x12121212; //r12
	stackAddrPtr[stackSize-R3_OFFSET]	= 0x03030303; //r3
	stackAddrPtr[stackSize-R2_OFFSET]	= 0x02020202; //r2
	stackAddrPtr[stackSize-R1_OFFSET]	= 0x01010101; //r1
	stackAddrPtr[stackSize-R0_OFFSET]	= 0x00000000; //r0
	stackAddrPtr[stackSize-R11_OFFSET]	= 0x11111111; //r11
	stackAddrPtr[stackSize-R10_OFFSET]	= 0x10101010; //r10
	stackAddrPtr[stackSize-R9_OFFSET]	= 0x09090909; //r9
	stackAddrPtr[stackSize-R8_OFFSET]	= 0x08080808; //r8
	stackAddrPtr[stackSize-R7_OFFSET]	= 0x07070707; //r7
	stackAddrPtr[stackSize-R6_OFFSET]	= 0x06060606; //r6
	stackAddrPtr[stackSize-R5_OFFSET]	= 0x05050505; //r5
	stackAddrPtr[stackSize-R4_OFFSET]	= 0x04040404; //r4
	/*for (UINT32 j=17; j<=stackSize-1; j++)
	{
		if (stackSize-j == 0)
			stackAddrPtr[stackSize-j]=0xDEADBEEF;
		else
			stackAddrPtr[stackSize-j]=0xDEADC0DE;
	}*/

	return K_SUCCESS;
}

K_ERR kInitTcb_(const TASKENTRY taskFuncPtr, UINT32* const stackAddrPtr,
		const UINT32 stackSize)
{
	if (kInitStack_(stackAddrPtr, stackSize, taskFuncPtr) == K_SUCCESS)
	{
		tcbs[pPid].stackAddrPtr          = stackAddrPtr;
		tcbs[pPid].sp	                = &stackAddrPtr[stackSize-R4_OFFSET];
		tcbs[pPid].stackSize             = stackSize;
		tcbs[pPid].status = READY;
		tcbs[pPid].pid = pPid;

		return K_SUCCESS;
	}
	return K_ERROR;
}

K_ERR kCreateTask(const TASKENTRY taskFuncPtr, const char* taskName, const PID id,
		UINT32* const stackAddrPtr, const UINT32 stackSize,
		const TICK timeSlice, const PRIO priority, const BOOL runToCompl)

{
	assert(id!=255);

	if (pPid==0)
	{

		for (UINT32 idx; idx<NTHREADS; idx++)
		{
			tidTbl[idx]=0xFF;
		}
		assert(kInitTcb_(IdleTask, idleStack, stackSize)
				== K_SUCCESS);


		tcbs[pPid].priority = NPRIO-1;
		tcbs[pPid].realPrio = NPRIO-1;
		tcbs[pPid].taskName = "IdleTask";
		tcbs[pPid].uPid = 0;
		tcbs[pPid].runToCompl = FALSE;
		tcbs[pPid].timeSlice = 0;
		assert(tidTbl[pPid]==0xFF);
		tidTbl[pPid]=0;
		pPid+=1;

	}
	if (kInitTcb_(taskFuncPtr, stackAddrPtr, stackSize) == K_SUCCESS)
	{
		if (priority > NPRIO-1)
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
		assert(tidTbl[pPid]==0xFF);
		tidTbl[pPid]=id;
		pPid+=1;
		return K_SUCCESS;
	}


	return K_ERROR;
}
