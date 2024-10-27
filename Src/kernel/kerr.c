/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Error handling and Checks
 *
 *****************************************************************************/
#include <kapi.h>

void kErrHandler(K_FAULT fault)  /* generic error handler */
{
	faultID=fault;
	__disable_irq();
	while (1);
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
			assert(tcbs[i].priority >= prioRun);
		}
	}
	K_EXIT_CR;

	return;
}


