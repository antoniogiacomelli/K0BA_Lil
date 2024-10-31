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

/*******************************************************************************
 o Example

   This application example uses a MAILBOX and a BYTEPOOL.
   Task1 iterates over an array of UINT32 with 4 words, posting a word on each
   kMailboxPost call. As the communication is fully synchronous.
   Task1 only proceeds after receiving an ACK.
   Task2 pends on this Mailbox. Every time it is signalled there is an item,
   it allocates 4 bytes from a byte pool and increments a counter.
   After receiving 4 messages, it deallocates 16 bytes altogether.

   Note this is for demonstration purposes. A way safer approach would pass the
   address of statically allocated UINT32, preferable a local global in applica_
   tion.c If it were to be shared, you would use a Mutex.
   As a second approach you could use a BLOCKPOOL, with fixed-size allocation,
   and deallocate it every time you consume the message.
   A third approach, still using the BYTE POOL would immediately deallocate
   the memory after 1  message is consumed, and not the 16 bytes altogether.

*******************************************************************************/

K_MAILBOX mailbox;
K_BYTEPOOL poolCB;
BYTE pool[17];
VOID kApplicationInit(VOID)
{
	kMailboxInit(&mailbox);
	kBytePoolInit(&poolCB, pool, 17);
}

VOID Task1(VOID)
{
	UINT32 mail[4] = { 0xBADC0FFE, 0x0BADFACE, 0xDEADC0DE, 0xDEADBEEF };
	UINT32 i=0; /* produce first message */
	while(1)
	{
		/* send the message */
		K_ERR ret = kMailboxPost(&mailbox, &mail[i], sizeof(&mail[0]));
		/* produce another message */
		i +=1;
		i %=4; /*wrap*/
		UNUSED(ret);
	}
}

VOID Task2(VOID)
{
	UINT32* PTR1;
	UINT32* PTR2;
	UINT32* PTR3;
	UINT32*  PTR4;

	while(1)
	{
		/*allocate space*/
		PTR1 = kBytePoolAlloc(&poolCB, 4);
		PTR2 = kBytePoolAlloc(&poolCB,4);
		PTR3 = kBytePoolAlloc(&poolCB,4);
		PTR4 = kBytePoolAlloc(&poolCB,4);
		*PTR1=0xAABBCCDD;
		*PTR2=0xDEADBEEF;
		*PTR3=0xBADC0FFE;
		*PTR4=0xDEADB050;
		kBytePoolFree(&poolCB, PTR1, 4);
		kBytePoolFree(&poolCB, PTR2, 4);
		kBytePoolFree(&poolCB, PTR3, 4);
		kBytePoolFree(&poolCB, PTR4, 4);

	}
}

volatile UINT32 counter3;
VOID Task3(VOID)
{
	while(1)
	{
		counter3++;
		kSleepDelay(10);
	}
}

volatile UINT32 counter4;


VOID Task4(VOID)
{
	while(1)
	{
		counter4++;
		kSleepDelay(5);
	}
}

