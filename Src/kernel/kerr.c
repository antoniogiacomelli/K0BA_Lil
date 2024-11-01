/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Error handling and Checks
 *
 *****************************************************************************/
#define K_CODE
#include "ksys.h"


void kErrHandler(K_FAULT fault)  /* generic error handler */
{
#if (ERR_HANDLER==ON)
	faultID=fault;
	__disable_irq();
	while (1);
#else
	return;
#endif /*err handler*/

}

void kErrCheckPrioInversion(void)
{
	K_CR_AREA;
	K_ENTER_CR;
	K_TCB* runPtr_ = runPtr;
	assert(runPtr_->status == RUNNING);
	PRIO prioRun = runPtr_->priority;
	if (prioRun == 0)
	{
		K_EXIT_CR;
		return;
	}
	for (UINT32 i = 0; i<NTHREADS; i++)
	{
		if (tcbs[i].status == READY)
		{
			if (tcbs[i].priority < prioRun)
			{
#if (K_DEF_PRIOINV_FAULT == ON)
				kErrHandler(FAULT_PRIO_INV);
#endif
			}
		}
	}
	K_EXIT_CR;

	return;
}


