/*******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]
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
#if (K_DEF_QUEUE==ON)
	QUEUE,
#endif
#if (K_DEF_SEMA==ON)
	SEMAPHORE,
#endif
#if (K_DEF_MUTEX==ON)
	MUTEX,
#endif
#if (K_DEF_STREAM==ON)
	STREAM,
#endif
#if(K_DEF_EVENT==ON)
	EVENT,
#endif
#if(K_DEF_CALLOUT_TIMER==ON)
	TIMER,
#endif
	TASK_HANDLE,
	NONE
} K_OBJ_TYPE;


struct kTimeoutNode
{
	struct kTimeoutNode *nextPtr;
	struct kTimeoutNode *prevPtr;
	TICK timeout;
	TICK dtick;
	ADDR kobj;
	K_OBJ_TYPE objectType;
};

#if (K_DEF_CALLOUT_TIMER==ON)
struct kTimer
{
	BOOL reload;
	TICK phase;
	CALLOUT funPtr;
	ADDR argsPtr;
	struct kTimeoutNode timeoutNode;
} __attribute__((aligned));
#endif

struct kListNode
{
	struct kListNode *nextPtr;
	struct kListNode *prevPtr;
};

struct kList
{
	struct kListNode listDummy;
	STRING listName;
	ULONG size;
	BOOL init;
};

struct kTcb
{
	/* Don't change */
	INT *sp;
	K_TASK_STATUS status;
	ULONG runCnt;
	/**/
	STRING taskName;
	INT *stackAddrPtr;
	UINT stackSize;
	PID pid; /* System-defined task ID */
	PRIO priority; /* Task priority (0-31) 32 is invalid */
	struct kTask *taskHandlePtr;
#if ( (K_DEF_FUNC_DYNAMIC_PRIO==ON) || (K_DEF_MUTEX_PRIO_INH==ON) )
	PRIO realPrio; /* Real priority  */
#endif
	BOOL   signalled; /* private binary semaphore */
#if ((K_DEF_TASK_FLAGS==ON) || (K_DEF_EVENT_FLAGS==ON))
	ULONG  currFlags; /* event flags */
#endif
#if (K_DEF_EVENT_FLAGS==ON)
    ULONG flagsOptions;
    ULONG gotFlags;
#endif
#if (K_DEF_SCH_TSLICE == ON)
	TICK timeSlice;
	TICK yieldTime;
#endif
	TICK busyWaitTime;
 	TICK lastWakeTime;
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
	LONG value;
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

#if (K_DEF_EVENT==ON)

struct kEvent
{
	struct kList waitingQueue;
	BOOL init;
#if (K_DEF_EVENT_FLAGS)
	ULONG eventFlags;
#endif
	K_TIMEOUT_NODE timeoutNode;

};

#endif /* K_DEF_EVENT */

#if (K_DEF_ALLOC==ON)

#define MEMBLKLAST (0)

/* Fixed-size pool memory control block (BLOCK POOL) */
struct kMemBlock
{
	BYTE *freeListPtr;
	BYTE *poolPtr;
	ULONG blkSize;
	ULONG nMaxBlocks;
	ULONG nFreeBlocks;
#if (MEMBLKLAST)
	BYTE* lastUsed;
#endif
	BOOL init;
};
#endif

#if (K_DEF_MBOX==ON)
/* Mailbox (single capcacity)*/
struct kMailbox
{
	BOOL init;
	ADDR mailPtr;
#if (K_DEF_MBOX_POSTPEND_PRIO_INH==ON)
	struct kTcb* clientTask;
	struct kTcb* serverTask;
#endif
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));
#endif

#if (K_DEF_QUEUE==ON)
struct kQ
{
	BOOL init;
	ADDR mailQPtr;
	ULONG headIdx;
	ULONG tailIdx;
	ULONG maxItems;
	ULONG countItems;
	K_TCB* port;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));
#endif

#if ((K_DEF_STREAM==ON))

/* Message Stream */
struct kStream
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
	ULONG nUsers; /* number of tasks using */
};
struct kPumpDropQueue
{
	struct kMemBlock *memCtrlPtr; /* associated allocator */
	struct kPumpDropBuf *currBufPtr; /* current buffer   */
	UINT failReserve;
	BOOL init;
};

#endif


struct kTask
{
	struct kTcb *tcbPtr;
	struct kTimeoutNode timeoutNode;
};

/*[EOF]*/
#ifdef __cplusplus
}
#endif

#endif /* KOBJS_H */
