#include <application.h>

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


/******************************************************************************
*
* Kernel Objects and data shared with the kernel
*
*******************************************************************************/



/******************************************************************************
 * Customise your application init
 *
 * Initialise kernel objects: mutexes, seamaphores, timers, etc.
 *******************************************************************************/
volatile UINT32 counter1; counter2; counter3; counter4;

VOID kApplicationInit(ADDR* args)
{
 UNUSED(args);
 counter1=0;
 counter2=0;
 counter3=0;
 counter4=0;
}

void Task1(void)
{

	while(1)
	{
		counter1++;
	}
}

void Task2(void)
{
	while(1)
	{
			counter2++;

	}
}


void Task3(void)
{
	while(1)
	{
		counter3++;

	}


}

void Task4(void)
{
	while(1)
	{
		counter4++;

	}

}

