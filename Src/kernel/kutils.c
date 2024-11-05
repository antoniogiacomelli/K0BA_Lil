/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module           : Utils
 *  Provides to      : All
 *  Application API  : Yes
 *
 * 	In this unit:
 * 		o Misc generic utils
 *
 ******************************************************************************/


#define K_CODE
#include "kconfig.h"
#include "ktypes.h"
#include "kobjs.h"
#include "kapi.h"
#include "kglobals.h"

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
