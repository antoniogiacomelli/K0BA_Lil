#include "application.h"
#include <stdlib.h>
#include <stdio.h>

INT stack1[STACKSIZE];
INT stack2[STACKSIZE];

VOID kApplicationInit(VOID)
{

}

VOID Task1(VOID)
{
	while(1)
	{
		kSleep(1);
	}
}


VOID Task2(VOID)
{
	while(1)
	{
		kSleep(1);
	}
}
