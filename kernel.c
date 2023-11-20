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
TCB_t* RunPtr;

void kSetInitStack(uint8_t i)
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
		kYield();
	}
}

int8_t kAddTask(Task t, void *args, uint8_t pid, uint8_t priority)
{
	tcbs[pid].block_sema = 0;
	tcbs[pid].block_mutex = 0;
	tcbs[pid].sleeping = 0;
	tcbs[pid].nmsg.value = 0;
	tcbs[pid].mlock.value = 1;
	tcbs[pid].priority = priority;
	tcbs[pid].rpriority = priority;
	if (pid == 0)
		RunPtr = &tcbs[0];
	kSetInitStack(pid);
	p_stacks[pid][STACK_SIZE-2] = (int32_t)(t); // PC
	p_stacks[pid][STACK_SIZE-8] = (int32_t)args; // R0
	if (n_added_threads == NTHREADS-1)
	{
				tcbs[pid].next = &tcbs[0];
	}
	else if (n_added_threads < NTHREADS)
	{
			tcbs[pid].next = &tcbs[pid+1];
	}
	else
	{
		return NOK;
	}
	n_added_threads++;
	return OK;
}

/************************************************************************/
/* Scheduler                                                            */
/************************************************************************/
void SysTick_Handler(void)
{
	HAL_IncTick();
	kYield();
}

void kTaskSwitch(void)
{
	__disable_irq();
	for (int i = 0; i<NTHREADS; i++)
	{
		if (tcbs[i].sleeping > 0)
		{
			tcbs[i].sleeping--;
			if (tcbs[i].sleeping == 0)
				tcbs[i].status = READY;
		}
	}
	if (RunPtr->status != BLOCKED && RunPtr->status != SLEEPING)
	{
		RunPtr->status = READY;
	}
	int32_t max = 255;
	TCB_t* pt;
	pt = RunPtr;
	TCB_t* chosen=0;
	do
	{
		pt = pt->next;
		if ((pt->priority < max) && (pt->status != BLOCKED) && (pt->status != SLEEPING))
		{
			max = pt->priority;
			chosen = pt;
		}
		RunPtr = chosen;
		RunPtr->status = RUNNING;
	} while (RunPtr != pt);
	__enable_irq();
}


void kYield(void) /*triggers pendsv*/
{
	SCB->ICSR |= (1<<28);
}

void kSleepTicks(uint32_t ticks)
{
	__disable_irq();
	RunPtr->sleeping = ticks;
	RunPtr->status = SLEEPING;
	__enable_irq();
	kYield();
}

void kWakeTicks(uint32_t pid)
{
	__disable_irq();
	TCB_t* RunPtr_ = &tcbs[pid];
	RunPtr_->sleeping = 0;
	RunPtr_->status = READY;
	__enable_irq();
}

void kSleep(int32_t event)
{
	__disable_irq();
	RunPtr->event = event;
	RunPtr->status = SLEEPING;
	__enable_irq();
	kYield();
}

void kWake(int32_t event)
{
	__disable_irq();
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
	__enable_irq();
	if (flag_wake)
	{
		kYield();
	}
}

void kDisableTaskSwitch()
{
	__disable_irq();
	SysTick->CTRL &= 0xFFFFFFFE;
	__enable_irq();
}

void kEnableTaskSwitch()
{
	__disable_irq();
	SysTick->CTRL |= 0x00000001;
	__enable_irq();
}

/************************************************************************/
/* Semaphores, Mutexes                                                  */
/************************************************************************/
static void bsqueue_init(SEMA_t* s)
{
	__disable_irq();
	s->queue.head = 0;
	s->queue.tail = 0;
	s->queue.init = 1;
	__enable_irq();
}

static void bmqueue_init(MUTEX_t* m)
{
	__disable_irq();
	m->queue.head = 0;
	m->queue.tail = 0;
	m->queue.init = 1;
	__enable_irq();
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
	__disable_irq();
	s->value = value;
	s->queue.init = 1;
	s->queue.head = 0;
	s->queue.tail = 0;
	__enable_irq();
}

void kSemaWait(SEMA_t* s)
{
	__disable_irq();
	if (s->queue.init == 0)
		bsqueue_init(s);
	(s->value) = (s->value) - 1;
	if ((s->value) < 0)
	{
		RunPtr->block_sema = (uint32_t*)s; // reason it is blocked
		RunPtr->status = BLOCKED;
		benqueue(&s->queue, RunPtr);
		__enable_irq();
		kYield();
	}
	__enable_irq();
}

void kSemaSignal(SEMA_t* s)
{
	__disable_irq();
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
	__enable_irq();
}
void kMutexInit(MUTEX_t* m, int value)
{
	__disable_irq();
	m->lock = value;
	m->queue.init = 1;
	m->queue.head = 0;
	m->queue.tail = 0;
	__enable_irq();
}

void kLock(MUTEX_t* m)
{
	__disable_irq();
	if (m->queue.init == 0)
		bmqueue_init(m);
	if (m->lock == 0) //unlocked
	{
		m->lock = 1; //lock
		m->owner = RunPtr; //set owner
	}
	else //locked
	{
		if ((m->owner) != RunPtr) //not owner
		{
			RunPtr->block_mutex = (uint32_t*)m;
			benqueue(&m->queue, RunPtr);
			RunPtr->priority = RunPtr->rpriority; //restore real prio in case of prio inheritance
			__enable_irq();
			kYield();
		}
	}
	__enable_irq();
}

void kRelease(MUTEX_t* m)
{
	__disable_irq();
	TCB_t* t;
	if ((m->queue.head == 0) && (m->queue.tail == 0)) //no waiters
	{
		m->lock = 0;
		m->owner = 0;
		__enable_irq();
	}
	else //unblock, set new owner
	{
		t = bdequeue(&m->queue);
		t->block_mutex = 0;
		t->status = READY;
		m->owner = t;
		// priority inheritance
		if (RunPtr->priority < m->owner->priority)
		{
			m->owner->rpriority = m->owner->priority;
			m->owner->priority = RunPtr->priority;
		}
		__enable_irq();
		kYield();
	}
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
	for (size_t i = 0; i<size; ++i) { dest[i] = src[i] ; }

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

int8_t kSendMsg(const uint8_t *msg, uint8_t pid)
{
	TCB_t *r_task;
	r_task = &tcbs[pid];
	MBUFF_t *mp = get_mbuf();
	if (mp == NULL)
	{
		return  NOK;
	}
	mp->sender_pid = RunPtr->pid;
	copyString_(mp->contents, msg, MSG_SIZE);
	kSemaWait(&r_task->mlock);
	menqueue(&r_task->mqueue, mp);
	kSemaSignal(&r_task->mlock);
	kSemaSignal(&r_task->nmsg);
	return OK;
}

int8_t kRcvMsg(uint8_t *msg)
{
	int pid=-1;
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
	pid = mp->sender_pid;
	put_mbuf(mp);
	return pid;
}

int8_t kAsendMsg(uint8_t* msg, uint8_t pid)
{
	TCB_t *r_task;
	r_task = &tcbs[pid];
	MBUFF_t *mp = get_mbuf();
	if (mp == NULL)
	{
		return NOK;
	}
	mp->sender_pid = RunPtr->pid;
	copyString_(mp->contents, msg, MSG_SIZE);
	menqueue(&r_task->mqueue, mp);
	kSemaSignal(&r_task->nmsg);
	return OK;
}

int8_t kArecvMsg(uint8_t *msg)
{
	int pid;
	MBUFF_t *mp = mdequeue(&RunPtr->mqueue);
	if (mp == NULL)
	{
		return NOK;
	}
	copyString_(msg, mp->contents, MSG_SIZE);
	pid = mp->sender_pid;
	put_mbuf(mp);
	return pid;
}

/************************************************************************/
/* PIPES                                                                */
/************************************************************************/
PIPE_t pipe[NPIPE];

void kPipeInitAll()
{
	size_t i;
	for (i=0; i<NPIPE; i++)
	{
		pipe[i].status = 0; //all pipes are free
	}
}

PIPE_t *kPipeCreate()
{
	size_t i; 
	PIPE_t *p;
	for (i=0; i<NPIPE; i++){
		p = &pipe[i];
		if (p->status==0)
		{
			p->status = 1; //take this pipe
			break;
		}
	}
	p->head = p->tail = p->data = 0; //initialize it
	p->room = PSIZE;
	return p;
}

int32_t kPipeRead(PIPE_t *p, uint8_t *buf, size_t n)
{
	int32_t r = 0;
	if (n<=0) 
	{
		return NOK;
	}
	if (p->status==0) 
	{
		return NOK;
	}
	while(n)
	{
		while(p->data)  //while there is data
		{
			*buf = p->buf[p->tail++];
			p->tail %= PSIZE;
			p->data--;
			p->room++;
			buf++;
			r++;
			n--;
			if (n==0) 
				break;
		}
		kWake((uint32_t)&p->room); //wake up waiting for room
		if (r) 
		{ 
			return r; 
		}
		kSleep((uint32_t)&p->data); //no data read, sleep for data
	}
	return r;
}

uint32_t kPipeWrite(PIPE_t *p, uint8_t *buf, size_t n)
{
	uint32_t w = 0;
	if (p->status == 0) return -1;
	if (n<=0) 
		return 0;
	while(n)
	{
		while(p->room)
		{
			p->buf[p->head++] = *buf;
			p->head %= PSIZE;
			p->data++;
			p->room--;
			buf++;
			w++;
			n--;
			if (n==0)
			{
				break;
			}
		}
		kWake((uint32_t)&p->data);
		if (n==0) 
		{  
			return w; 
		}
		kSleep((uint32_t)&p->room);

	}
	return w;
}

/************************************************************************/
/* Data Queue		                                                    */
/************************************************************************/
void FifoInit(FIFO_t* me)
{
	me->head = 0;
	me->tail = 0;
	me->size.value = 0;
	me->roomleft.value = FIFO_SIZE;
	me->mutex.lock = 0;
	me->mutex.owner = 0;
	for (size_t i=0;i<FIFO_SIZE;++i)	
		me->buffer[i] = 0;
}

int32_t FifoPut(FIFO_t* me, uint8_t data)
{
	kSemaWait(&me->roomleft);
	kLock(&me->mutex);
	me->buffer[me->head] = data;
	me->head = (me->head + 1)%FIFO_SIZE;
	kRelease(&me->mutex);
	kSemaSignal(&me->size);
	return OK;
}
uint8_t FifoGet(FIFO_t* me)
{
	uint8_t get_data;
	kSemaWait(&me->size);
	kLock(&me->mutex);
	get_data = me->buffer[me->tail];
	me->tail = (me->tail + 1) % FIFO_SIZE;
	kRelease(&me->mutex);
	kSemaSignal(&me->roomleft);
	return get_data;
}
