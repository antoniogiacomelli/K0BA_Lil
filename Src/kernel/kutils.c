/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module: N/A
 * 	In this unit:
 * 		o Misc generic utils
 *
 *****************************************************************************/


#define K_CODE
#include "ksys.h"


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

ADDR kMemCpy(ADDR destPtr, ADDR const srcPtr, SIZE size)
{
	if ((IS_NULL_PTR(destPtr)) || (IS_NULL_PTR(srcPtr)))
	{
		kErrHandler(FAULT_NULL_OBJ);
	}
	BYTE* destTempPtr = (BYTE*)destPtr;
	BYTE const* srcTempPtr = (BYTE const*) srcPtr;
	for (SIZE i = 0; i < size; ++i)
	{
		destTempPtr[i] = srcTempPtr[i];
	}
	return destTempPtr;
}
