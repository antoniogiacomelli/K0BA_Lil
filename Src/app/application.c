#include "application.h"
/*******************************************************************************
*
* Application Tasks Stacks
*
*******************************************************************************/

/*
 * Declare your stacks here
 * They must be already extern data on application.h
 */
UINT32 stack1[STACKSIZE];
UINT32 stack2[STACKSIZE];
UINT32 stack3[STACKSIZE];
UINT32 stack4[STACKSIZE];


/*******************************************************************************
 * Customise your application init
 *
 * Initialise kernel objects: mutexes, seamaphores, timers, etc.
 *******************************************************************************/
VOID kApplicationInit(VOID)
{

}

VOID Task1(VOID)
{
	while(1)
	{

	}
}

VOID Task2(VOID)
{


	while(1)
	{

	}
}

volatile UINT32 counter3;
VOID Task3(VOID)
{
	while(1)
	{

	}
}

volatile UINT32 counter4;


VOID Task4(VOID)
{
	while(1)
	{

	}
}

