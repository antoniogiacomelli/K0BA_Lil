/****************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 *******************************************************************************
 * 	In this header:
 * 					o Kernel objects definition
 *
 ******************************************************************************/
#ifndef KOBJS_H
#define KOBJS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{

#if (K_DEF_MBOX==ON)
	MAILBOX,
#endif
#if (K_DEF_MMBOX==ON)
	MMBOX,
#endif
#if (K_DEF_SEMA==ON)
	SEMAPHORE,
#endif
#if (K_DEF_MUTEX==ON)
	MUTEX,
#endif
#if (K_DEF_MESGQ==ON)
	MESGQ,
#endif
#if(K_DEF_SLEEPWAKE==ON)
	EVENT,
#endif
	TASK_HANDLE, NONE
} K_OBJ_TYPE;


struct kTimeoutNode
{
	struct kTimeoutNode *nextPtr;
	TICK timeout;
	ADDR kobj;
	K_OBJ_TYPE objectType;
};


struct kListNode
{
	struct kListNode *nextPtr;
	struct kListNode *prevPtr;
};

struct kList
{
	struct kListNode listDummy;
	STRING listName;
	UINT size;
	BOOL init;
};

struct kTcb
{
	/* Don't change */

	INT *sp;
	K_TASK_STATUS status;
	UINT runCnt;

	/**/
	STRING taskName;
	INT *stackAddrPtr;
	UINT stackSize;
	PID pid; /* System-defined task ID */
	TID uPid; /* User-defined   task ID */
	PRIO priority; /* Task priority (0-31) 32 is invalid */
#if ( (K_DEF_FUNC_DYNAMIC_PRIO==ON) || (K_DEF_MUTEX_PRIO_INH==ON) )
	PRIO realPrio; /* Real priority  */
#endif
#if ((K_DEF_TASK_SIGNAL_BIN_SEMA==(ON)))
	BOOL signalled;
	struct kTimeoutNode timeoutNode;
#endif
	struct kTaskHandle *taskHandlePtr; /* K_TASK_HANDLE is a pointer to struct */
#if (K_DEF_SCH_TSLICE == ON)
	TICK timeSlice;
	TICK timeLeft;
#endif
	TICK busyWaitTime;
#if (K_DEF_SCH_TSLICE==OFF)
	TICK lastWakeTime;
#endif
	/* Resources */
#if (K_DEF_SEMA == ON)
	K_SEMA *pendingSema;
#endif
#if (K_DEF_MUTEX==ON)
	K_MUTEX *pendingMutx;
#endif
#if (K_DEF_SLEEPWAKE==ON)
	K_EVENT *pendingEv;
#endif
#if (K_DEF_MBOX==ON)
	K_MBOX *pendingMbox;
#endif
	K_TIMER *pendingTmr;
	BOOL runToCompl;
	BOOL yield;
	BOOL timeOut;
	/* Monitoring */
	UINT nPreempted;
	PID preemptedBy;
	struct kListNode tcbNode;
} __attribute__((aligned));

struct kRunTime
{
	TICK globalTick;
	UINT nWraps;
};
extern struct kRunTime runTime;


#if (K_DEF_SEMA==ON)

struct kSema
{
	BOOL init;
	INT value;
	struct kTcb *owner;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
};

#endif

#if (K_DEF_MUTEX == ON)

struct kMutex
{
	struct kList waitingQueue;
	BOOL lock;
	struct kTcb *ownerPtr;
	BOOL init;
	K_TIMEOUT_NODE timeoutNode;
};
#endif

#if (K_DEF_SLEEPWAKE==ON)

struct kEvent
{
	struct kList waitingQueue;
	BOOL init;
	K_TIMEOUT_NODE timeoutNode;

};
#endif /* K_DEF_SLEEPWAKE */

#define MEMBLKLAST (0)

/* Fixed-size pool memory control block (BLOCK POOL) */
struct kMemBlock
{
	BYTE *freeListPtr;
	BYTE *poolPtr;
	BYTE blkSize;
	BYTE nMaxBlocks;
	BYTE nFreeBlocks;
#if (MEMBLKLAST)
	BYTE* lastUsed;
#endif
	BOOL init;
};

#if (K_DEF_MBOX==ON)
/* Mailbox (single capcacity)*/
struct kMailbox
{
	BOOL init;
	ADDR mailPtr;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;

} __attribute__((aligned(4)));
#endif

#if (K_DEF_MMBOX==ON)
struct kMultibox
{
	BOOL init;
	ADDR mailQPtr;
	UINT headIdx;
	UINT tailIdx;
	ULONG maxItems;
	ULONG countItems;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));
#endif

#if ((K_DEF_MESGQ==ON))

/* Message Queue (Stre*/
struct kMesgQ
{
	BOOL init;
	ULONG mesgSize;
	ULONG maxMesg;
	ULONG mesgCnt;
	ADDR buffer;
	ULONG readIndex;
	ULONG writeIndex;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));

#endif /*K_DEF_MSG_QUEUE*/

#if (K_DEF_PDMESG== ON)

struct kPumpDropBuf
{

	ADDR dataPtr;
	ULONG dataSize; /* mesg size in this buf */
	UINT nUsers; /* number of tasks using */
};
struct kPumpDropQueue
{
	struct kMemBlock *memCtrlPtr; /* associated allocator */
	struct kPumpDropBuf *currBufPtr; /* current buffer   */
	UINT failReserve;
	BOOL init;
};

#endif

struct kTimer
{
	STRING timerName;
	TICK dTicks;
	TICK ticks;
	BOOL reload;
	CALLOUT funPtr;
	ADDR argsPtr;
	TID taskID;
	K_TIMER *nextPtr;
	BOOL init;
} __attribute__((aligned));

struct kTaskHandle
{
	struct kTcb *handle;
	struct kTimeoutNode timeoutNode;
};

/*[EOF]*/
#ifdef __cplusplus
}
#endif

#endif /* KOBJS_H */
