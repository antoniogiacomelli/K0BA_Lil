/************************************************************************
* @file		kernel.c
* @brief	C functions for the kernel. 
* @date		18/11/23
* @author	Antonio Giacomelli <antoniogiacomelli@protonmail.com>
** (C) Copyright 2022-23 www.antoniogiacomelli.com
***********************************************************************
/*include your CMSIS compliant HAL here*/
#include "kernel.h"
/************************************************************************/
/*	Thread Management                                                  */
/************************************************************************/
TCB_t tcbs[NTHREADS];				
uint32_t p_stacks[NTHREADS][STACK_SIZE];   
TCB_t* RunPtr; /* running */
TCB_t* chosen; /* scheduled */

static uint8_t taskIndex=0;
static volatile uint32_t crState;
void kEnterCR(void) {
    // Save current PRIMASK value and disable interrupts
	crState = __get_PRIMASK();
	if (crState == 0)
		__disable_irq();
}

void kExitCR(void)
{
    // Restore the original PRIMASK value (re-enables or keeps disabled based on prior state)
    __set_PRIMASK(crState);
}
void kSetInitStack(uint8_t i, uint8_t pid)
{
	tcbs[i].sp				= &p_stacks[i][STACK_SIZE-16];
	tcbs[i].pid				= i;
	p_stacks[i][STACK_SIZE-1]		= 0x01000000; //psr
	p_stacks[i][STACK_SIZE-2]		= 0x00000000; //pc  R15
	p_stacks[i][STACK_SIZE-3]		= 0xFFFFFFFD; //r14	LR
	p_stacks[i][STACK_SIZE-4]		= 0x12121212; //r12
	p_stacks[i][STACK_SIZE-5]		= 0x03030303; //r3
	p_stacks[i][STACK_SIZE-6]		= 0x02020202; //r2
	p_stacks[i][STACK_SIZE-7]		= 0x01010101; //r1
	p_stacks[i][STACK_SIZE-8]		= 0x00000000; //r0
	p_stacks[i][STACK_SIZE-9]		= 0x11111111; //r11
	p_stacks[i][STACK_SIZE-10]		= 0x10101010; //r10
	p_stacks[i][STACK_SIZE-11]		= 0x09090909; //r9
	p_stacks[i][STACK_SIZE-12]		= 0x08080808; //r8
	p_stacks[i][STACK_SIZE-13]		= 0x07070707; //r7
	p_stacks[i][STACK_SIZE-14]		= 0x06060606; //r6
	p_stacks[i][STACK_SIZE-15]		= 0x05050505; //r5
	p_stacks[i][STACK_SIZE-16]		= 0x04040404; //r4

}
static int32_t n_added_threads = 0;
void TaskIdle(void *args)
{
	while(1)
	{
		__DSB();
		__WFI();
		__ISB();
	}
}
int8_t kAddTask(Task t, void *args, uint8_t pid, uint8_t priority)
{
	if (taskIndex==0) /* add idle task */
	{
		tcbs[taskIndex].block_sema = 0;
		tcbs[taskIndex].block_mutex = 0;
		tcbs[taskIndex].sleeping = 0;
		tcbs[taskIndex].nmsg.value = 0;
		tcbs[taskIndex].mlock.value = 1;
		tcbs[taskIndex].priority = 2;
		tcbs[taskIndex].rpriority = 2;
		
		kSetInitStack(taskIndex);
		p_stacks[taskIndex][STACK_SIZE-2] = (int32_t)(TaskIdle); // PC
		p_stacks[taskIndex][STACK_SIZE-8] = (int32_t)0; // R0
	}
	else
	{
		RunPtr = &tcbs[0]; /* init RunPtr to &tcbs[0] is needed */
	   	
		tcbs[taskIndex].block_sema = 0;
		tcbs[taskIndex].block_mutex = 0;
		tcbs[taskIndex].sleeping = 0;
		tcbs[taskIndex].nmsg.value = 0;
		tcbs[taskIndex].mlock.value = 1;
		tcbs[taskIndex].priority = priority;
		tcbs[taskIndex].rpriority = priority;
		
		kSetInitStack(taskIndex);
		p_stacks[taskIndex][STACK_SIZE-2] = (int32_t)(t); // PC
		p_stacks[taskIndex][STACK_SIZE-8] = (int32_t)args; // R0
		if (n_added_threads == NTHREADS-1)
		{
			tcbs[taskIndex].next = &tcbs[0];
		}
		else if (n_added_threads < NTHREADS)
		{
			tcbs[taskIndex].next = &tcbs[taskIndex+1];
		}
		else
		{
			return NOK;
		}
	}
	taskIndex += 1;
	n_added_threads++;
	return OK;
}

/************************************************************************/
/* Scheduler                                                            */
/************************************************************************/
static inline void kSchedule(void) 
{
	
	int32_t max = 255;
	TCB_t* pt;
	pt = RunPtr;
	chosen = NULL;
	do
	{
		pt = pt->next;
		if ((pt->priority < max) && (pt->status != BLOCKED) && (pt->status != SLEEPING))
		{
			max = pt->priority;
			chosen = pt;
			
		}
		
	} while (RunPtr != pt);
}
static int32_t kTickTaskSwitch_(void)
{
	int32_t retVal = 0; /* no context switch need, a priori */
	for (int i = 0; i<NTHREADS; i++)
	{
		if (tcbs[i].sleeping > 0)
		{
			tcbs[i].sleeping--;
			if (tcbs[i].sleeping == 0)
				tcbs[i].status = READY;
		}
	}

	kSchedule_();
	
	if (chosen != NULL) /* need switch*/
	{
	        RunPtr->status = READY; /* make preempted task as READY */
		retVal=1;               /* return 1 to signal context switching */
	}
	return retVal;
	
}

void SysTick_Handler(void)
{
	kEnterCR();
	if (kTickTaskSwitch_()) /* need switch? */
	{
	  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  /* there is another task to run, defer full context switching to PendSV */
	}
	kExitCR();
}

void kTaskSwitch(void) /* just update RunPtr */
{
	RunPtr = chosen;
        RunPtr->status = RUNNING;
	chosen = NULL; /* clear scheduled task */
}

void kYield(void)
{
  asm volatile ("svc #0");
}

void SVC_Handler(void)
{
	kEnterCR();
	if (RunPtr->status == RUNNING) /* if yielded not blocked/sleeping, make it READY */
            RunPtr->status = READY;
	kSchedule_(); 
	if (chosen == NULL) /* there is not another task to run */
	{
		kExitCR(); 
		return; /* return to caller, restoring context */
	} /* there is another task to run, defer full context switching to PendSV */
	SCB->ICSR |=  SCB_ICSR_PENDSVSET_Msk;
	kExitCR();
}


void kSleepTicks(uint32_t ticks)
{
	kEnterCR();
	RunPtr->sleeping = ticks;
	RunPtr->status = SLEEPING;
	kExitCR();
	kYield();
}

void kWakeTicks(uint32_t taskIndex)
{
	kEnterCR();
	TCB_t* RunPtr_ = &tcbs[taskIndex];
	RunPtr_->sleeping = 0;
	RunPtr_->status = READY;
	kExitCR();
}

void kSleep(int32_t event)
{
	kEnterCR();
	RunPtr->event = event;
	RunPtr->status = SLEEPING;
	kExitCR();
	kYield();
}

void kWake(int32_t event)
{
	kEnterCR();
	volatile uint32_t flag_wake=0;
	for (size_t i = 1; i<NTHREADS; i++)
	{
		if (tcbs[i].event == event && tcbs[i].status == SLEEPING)
		{
			tcbs[i].event = 0;
			tcbs[i].status = READY;
			flag_wake=1;
		}
	}
	kExitCR();
	if (flag_wake)
	{
		kYield();
	}
}

void kDisableTaskSwitch()
{
	kEnterCR();
	SysTick->CTRL &= 0xFFFFFFFE;
	kExitCR();
}

void kEnableTaskSwitch()
{
	kEnterCR();
	SysTick->CTRL |= 0x00000001;
	kExitCR();
}

/************************************************************************/
/* Semaphores, Mutexes                                                  */
/************************************************************************/
static void bsqueue_init(SEMA_t* s)
{
	kEnterCR();
	s->queue.head = 0;
	s->queue.tail = 0;
	s->queue.init = 1;
	kExitCR();
}

static void bmqueue_init(MUTEX_t* m)
{
	kEnterCR();
	m->queue.head = 0;
	m->queue.tail = 0;
	m->queue.init = 1;
	kExitCR();
}

static int benqueue(BQUEUE_t* me, TCB_t *p)
{
	me->blocked[me->head] = p;
	me->head = (me->head + 1) % NTHREADS;
	return OK;
}

static TCB_t *bdequeue(BQUEUE_t *me)
{
	TCB_t* ret;
	ret = me->blocked[me->tail];
	me->tail = (me->tail + 1) % NTHREADS;
	return ret;
}

void kSemaInit(SEMA_t* s, int32_t value)
{
	kEnterCR();
	s->value = value;
	s->queue.init = 1;
	s->queue.head = 0;
	s->queue.tail = 0;
	kExitCR();
}

void kSemaWait(SEMA_t* s)
{
	kEnterCR();
	if (s->queue.init == 0)
		bsqueue_init(s);
	(s->value) = (s->value) - 1;
	if ((s->value) < 0)
	{
		RunPtr->block_sema = (uint32_t*)s; // reason it is blocked
		RunPtr->status = BLOCKED;
		benqueue(&s->queue, RunPtr);
		kExitCR();
		kYield();
	}
	kExitCR();
}

void kSemaSignal(SEMA_t* s)
{
	kEnterCR();
	TCB_t* next_tcb_ptr;
	(s->value) = (s->value) + 1;
	if ((s->value) <= 0)
	{
		next_tcb_ptr = bdequeue(&s->queue);
		while (next_tcb_ptr->block_sema != (uint32_t*)s)
		{
			next_tcb_ptr = next_tcb_ptr->next;
		}
		next_tcb_ptr->block_sema = 0;
		next_tcb_ptr->status = READY;
	}
	kExitCR();
}
void kMutexInit(MUTEX_t* m, int value)
{
	kEnterCR();
	m->lock = value;
	m->queue.init = 1;
	m->queue.head = 0;
	m->queue.tail = 0;
	kExitCR();
}

void kLock(MUTEX_t* m)
{
	kEnterCR();
	if (m->queue.init == 0)
		bmqueue_init(m);
	if (m->lock == 0) //unlocked
	{
		m->lock = 1; //lock
		m->owner = RunPtr; //set owner
		kExitCR();
		return;
	}
	else //locked
	{
		if ((m->owner) != RunPtr) //not owner
		{
			RunPtr->block_mutex = (uint32_t*)m;
			RunPtr->status = BLOCKED;
			benqueue(&m->queue, RunPtr);
			// priority inheritance
			if (RunPtr->priority < m->owner->priority)
			{
				m->owner->rpriority = m->owner->priority;
				m->owner->priority = RunPtr->priority;
			}
		}
		kExitCR();
		kYield();
	}
}

void kRelease(MUTEX_t* m)
{
	kEnterCR();
	TCB_t* t;
	if ((m->queue.head == 0) && (m->queue.tail == 0)) //no waiters
	{
		m->lock = 0;
		m->owner = 0;
	}
	else //unblock, set new owner
	{
		t = bdequeue(&m->queue);
		t->block_mutex = 0;
		t->status = READY;
		m->owner = t;
		RunPtr->priority = RunPtr->rpriority;
	}
	kExitCR();
	return;
}

static inline void copyString_(uint8_t *dest, const uint8_t* src, size_t size)
{
	for (size_t i = 0; i<size; ++i)
	{
		dest[i] = src[i];
	}

}
static inline void copyMSG_(MSG_t *dest, const MSG_t* src, size_t size)
{
	for (size_t i = 0; i<size; ++i) 
	{ 
		dest[i] = src[i] ; 
	}

}

/************************************************************************/
/* Message Passing                                                 */
/************************************************************************/
MBUFF_t mbuff[NMBUF];
MBUFF_t *mbufflist=0;
SEMA_t nmbuf;
SEMA_t mlock;
static int32_t menqueue(MBUFF_t **queue, MBUFF_t *p)
{
	MBUFF_t *q  = *queue;
	if (q==0)  //if it is free
	{
		*queue = p; //take it
		p->next = 0; //next slot is free
		return OK;
	}
	while (q->next) //search for a free slot
	{
		q = q->next;
	}
	q->next = p; //free slot found
	p->next = 0; //next is free
	return OK;
}

static MBUFF_t *mdequeue(MBUFF_t **queue)
{
	MBUFF_t *mp = *queue;
	if (mp)
		*queue = mp->next;
	return mp;
}

void kMbufInitAll(void)
{
	int i;
	for (i=0; i<NMBUF; i++)
	{
		menqueue(&mbufflist, &mbuff[i]);
	}
	nmbuf.value = NMBUF;
	mlock.value = 1;
}

static MBUFF_t *get_mbuf(void)
{
	MBUFF_t *mp;
	kSemaWait(&nmbuf);
	kSemaWait(&mlock);
	mp = mdequeue(&mbufflist);
	kSemaSignal(&mlock);
	return mp;
}

static int32_t put_mbuf(MBUFF_t *mp)
{
	kSemaWait(&mlock);
	menqueue(&mbufflist, mp);
	kSemaSignal(&mlock);
	kSemaSignal(&nmbuf);
	return OK;
}

int8_t kSendMsg(const uint8_t *msg, uint8_t taskIndex)
{
	TCB_t *r_task;
	r_task = &tcbs[taskIndex];
	MBUFF_t *mp = get_mbuf();
	if (mp == NULL)
	{
		return  NOK;
	}
	mp->sender_taskIndex = RunPtr->taskIndex;
	copyString_(mp->contents, msg, MSG_SIZE);
	kSemaWait(&r_task->mlock);
	menqueue(&r_task->mqueue, mp);
	kSemaSignal(&r_task->mlock);
	kSemaSignal(&r_task->nmsg);
	return OK;
}

int8_t kRcvMsg(uint8_t *msg)
{
	int taskIndex=-1;
	kSemaWait(&RunPtr->nmsg);
	kSemaWait(&RunPtr->mlock);
	MBUFF_t *mp = mdequeue(&RunPtr->mqueue);
	if (mp == NULL)
	{
		kSemaSignal(&RunPtr->mlock);
		return NOK;
	}
	kSemaSignal(&RunPtr->mlock);
	copyString_(msg, mp->contents, MSG_SIZE);
	taskIndex = mp->sender_taskIndex;
	put_mbuf(mp);
	return taskIndex;
}

int8_t kAsendMsg(uint8_t* msg, uint8_t taskIndex)
{
	TCB_t *r_task;
	r_task = &tcbs[taskIndex];
	MBUFF_t *mp = get_mbuf();
	if (mp == NULL)
	{
		return NOK;
	}
	mp->sender_taskIndex = RunPtr->taskIndex;
	copyString_(mp->contents, msg, MSG_SIZE);
	menqueue(&r_task->mqueue, mp);
	kSemaSignal(&r_task->nmsg);
	return OK;
}

int8_t kArecvMsg(uint8_t *msg)
{
	int taskIndex;
	MBUFF_t *mp = mdequeue(&RunPtr->mqueue);
	if (mp == NULL)
	{
		return NOK;
	}
	copyString_(msg, mp->contents, MSG_SIZE);
	taskIndex = mp->sender_taskIndex;
	put_mbuf(mp);
	return taskIndex;
}
