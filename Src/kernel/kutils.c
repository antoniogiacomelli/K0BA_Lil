/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 		o Misc of helper functions and generic utils
 *
 *****************************************************************************/

#include <kapi.h>

PID kGetTaskPID(TID const taskID)
{
	PID pid=0;
	for (pid=0;pid<NTHREADS;pid++)
	{
		if (uPidTbl[pid]==taskID)
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

SIZE kStrLen(STRING s)
{
	SIZE len=0;
	while(*s !='\0')
	{
		s++;
		len++;
	}
	return len;
}

ADDR kMemCpy(ADDR destPtr, const ADDR srcPtr, SIZE size)
{
	if ((IS_NULL_PTR(destPtr)) || (IS_NULL_PTR(srcPtr)))
	{
		kErrHandler(FAULT_NULL_OBJ);
	}
	BYTE* destTempPtr = (BYTE*)destPtr;
	const BYTE* srcTempPtr = (const BYTE*)srcPtr;
	for (SIZE i = 0; i < size; ++i)
	{
		destTempPtr[i] = srcTempPtr[i];
	}
	return destTempPtr;
}
