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

K_MEM mesgMem;
K_MESGQ mesgq;
K_MAILBOX mailbox;
K_EVENT	  event1;
struct MESG mesgPool[10]; /* the kernel must have access to this pool
 	 	 	 	 	 	 	 so it */
#define SIZE_MESG sizeof(struct MESG)



volatile UINT32 counterA, counterB, counter1, counter2, counter3, counter4;

/**
 ******************************************************************************
 *
 * Application Callbacks prototypes and/or definitions
 *
 ******************************************************************************/

VOID callBackCount1(ADDR args)
{

	UNUSED(args);
	counterA++;
}
VOID callBackCount2(ADDR args)
{
	UNUSED(args);
	counterB++;

}

/**
 *******************************************************************************
 * Customise your application init
 *
 * Initialise kernel objects: mutexes, seamaphores, timers, etc.
 *******************************************************************************/
VOID kApplicationInit(ADDR* args)
{
	UNUSED(args);
	kTimerInit("Timer1", 20, callBackCount1, NULL, RELOAD);
	kTimerInit("Timer2", 15, callBackCount2, NULL, RELOAD);

	kMesgQInit(&mesgq, (ADDR)&mesgPool, 10, SIZE_MESG);
	kMailboxInit(&mailbox);
}
//VOID kMesgQueueInit(K_MSGQ* const self, ADDR mesgPoolPtr, BYTE queueSize, SIZE mesgSize)


void Task1(void)
{
	UINT32 send = 0xABCDEFAA;

	while(1)
	{
		kMailboxPost(&mailbox, &send, sizeof(send));
	}
}

void Task2(void)
{
	UINT32 rcvmesg;
	PID id;
	while(1)
	{
		id = kMailboxPend(&mailbox, &rcvmesg);
		if (id==5)
		{
			kWake(&event1);
		}
	}
}


void Task3(void)
{
	UINT32 woke=0;
	while(1)
	{
		kSleep(&event1);
		woke++;
	}


}

void Task4(void)
{
	UINT32 send = 0xABCDEF00;
	while(1)
	{
		kMailboxPost(&mailbox, &send, sizeof(send));
	}

}

