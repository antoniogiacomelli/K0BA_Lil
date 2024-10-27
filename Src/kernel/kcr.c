/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Critical Regions Enter/Exit
 *
 *****************************************************************************/
#include <kapi.h>

UINT32 kEnterCR(void)
{
	UINT32 crState;
	crState = __get_PRIMASK();
	if (crState == 0)
	{
		__disable_irq();
		asm volatile("ISB");
		asm volatile("DMB");
		return crState;
	}
	asm volatile("DMB");
	return crState;
}

void kExitCR(UINT32 crState)
{
	asm volatile("DMB");
	__set_PRIMASK(crState);
	asm volatile("ISB");
}
