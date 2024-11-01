/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Critical Regions Enter/Exit
 *
 *****************************************************************************/
#define K_CODE

#include "kapi.h"


UINT32 kEnterCR(void)
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

void kExitCR(UINT32 crState)
{
	asm volatile ("ISB");
	__set_PRIMASK(crState);
	asm volatile("DSB");
}
