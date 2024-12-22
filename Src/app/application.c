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
INT stack1[STACKSIZE];
INT stack2[STACKSIZE];
INT stack3[STACKSIZE];

/*******************************************************************************
 * Customise your application init
 * Declare/init app objects
 * Declare  kernel objects: mutexes, semaphores, timers, etc.
 *******************************************************************************/

K_EVENT syncEvent; /* state event				    */
UINT32 syncCounter; /* state representation        */
K_MUTEX syncMutex; /* monitor lock				    */
K_MUTEX resourceLock; /* if there is a resource */
#define SYNC_CONDITION (syncCounter>=3) /* needed tasks in the barrier */

/* only one task can be active within a monitor
 they are enqueued either on the mutex or on the event
 */
static VOID synch(VOID)
{

	kMutexLock(&syncMutex, K_WAIT_FOREVER);
	syncCounter += 1;
	if (!(SYNC_CONDITION))
	{
		kMutexUnlock(&syncMutex);

		kEventSleep(&syncEvent, K_WAIT_FOREVER);
		kMutexLock(&resourceLock, K_WAIT_FOREVER);
		kMutexUnlock(&resourceLock);
	}
	else
	{
		syncCounter = 0;
		kEventWake(&syncEvent);
		kMutexUnlock(&syncMutex);

	}
}

VOID kApplicationInit(VOID)
{
	kMutexInit(&syncMutex);
	kEventInit(&syncEvent);
	kMutexInit(&resourceLock);
	syncCounter = 0;
}

VOID Task1(VOID)
{

	while (1)
	{
		kSleep(5);
		synch();
	}
}

VOID Task2(VOID)
{
	while (1)
	{
		kSleep(8);
		synch();
	}
}

VOID Task3(VOID)
{
	while (1)
	{
		kSleep(3);
		synch();
	}

}
