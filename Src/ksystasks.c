/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module       : System Tasks
 * 	Depends on   : Scheduler
 * 	Provides to  : Application Timers
 *  Public API	 : N/A
 * 	In this unit :
 * 					o  System Tasks and Deferred ISR handler tasks
 *
 *****************************************************************************/

#define K_CODE
#include "kexecutive.h"

INT idleStack[IDLE_STACKSIZE];
INT timerHandlerStack[TIMHANDLER_STACKSIZE];

VOID IdleTask( VOID)
{

	while (1)
	{
		__DSB();
		__WFI();
		__ISB();
	}
}

VOID TimerHandlerTask( VOID)
{

	while (1)
	{
		kTaskPend( K_WAIT_FOREVER);

#if (K_DEF_CALLOUT_TIMER==ON)
		K_CR_AREA
		K_CR_ENTER
		timerListHeadPtr = timeOutListHeadPtr;
		while (timerListHeadPtr != NULL && timerListHeadPtr->dtick == 0)
		{
			K_TIMEOUT_NODE *node = (K_TIMEOUT_NODE*) timerListHeadPtr;
			timerListHeadPtr = node->nextPtr;
			kRemoveTimerNode( node);
			switch (node->objectType)
			{
			case TIMER:
			{
				K_TIMER *timer = (K_TIMER*) node->kobj;
				if (timer->funPtr != NULL)
				{
					timer->funPtr( timer->argsPtr);
				}
				if (timer->reload)
				{
					kTimerInit(timer, 0, timer->timeoutNode.timeout, \
							timer->funPtr, timer->argsPtr, timer->reload);
				}
			}
				break;
			default:
				break;
			}
		}
		K_CR_EXIT
#endif
	}
}
