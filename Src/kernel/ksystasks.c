/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o  System Tasks and Deferred ISR handler tasks
 *
 *****************************************************************************/


#include <kapi.h>
#include <ksystasks.h>

UINT32 idleStack[STACKSIZE];
UINT32 timerHandlerStack[STACKSIZE];

void IdleTask(void)
{

	while(1)
	{
		__DSB();
		__WFI();
		__ISB();
	}
}

void TimerHandlerTask(void)
{
	while(1)
	{
		kPend();

		if(dTimOneShotList || dTimReloadList)
		{
			kTimerHandler();
		}
	}
}
