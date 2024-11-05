/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module      : Error Handler
 * 	Depends on  : System Globals
 * 	Provides to : System
 *  Public API : No
 * 	In this unit:
 * 			   Error Handling
 *
 ******************************************************************************/

#include "kglobals.h"

/******************************************************************************
* ERROR HANDLING
*******************************************************************************/

void kErrHandler(K_FAULT fault) /* generic error handler */
{
#if (K_DEF_ERRHANDLER==ON)
	faultID=fault;
	__disable_irq();
	while (1);
#endif
}

#if 0
void kErrCheckPrioInversion(void)
{
	K_CR_AREA;
	K_ENTER_CR;
	K_TCB *runPtr_ = runPtr;
	assert(runPtr_->status == RUNNING);
	PRIO prioRun = runPtr_->priority;
	if (prioRun == 0)
	{
		K_EXIT_CR
		;
		return;
	}
	for (UINT32 i = 0; i < NPRIO; i++)
	{
		if (readyQueue[i].size > 0)
		{
			if (i < prioRun)
			{
				assert(0);
			}
		}
	}
	K_EXIT_CR;
	return;
}
#endif
