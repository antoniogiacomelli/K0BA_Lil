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

#include "ktypes.h"


typedef enum
{

#if (K_DEF_MBOX==ON)
    MAILBOX,
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
    NONE
} K_OBJ_SYNCH;


struct kListNode
{
	struct kListNode* nextPtr;
	struct kListNode* prevPtr;
};

struct kList
{
	struct kListNode listDummy;
	STRING listName;
	UINT32 size;
	BOOL init;
};


struct kTcb
{
/* Don't change */

	INT* sp;
	K_TASK_STATUS status;
	UINT32 runCnt;

/**/
	STRING taskName;
	INT* stackAddrPtr;
	UINT32 stackSize;
	PID pid;              /* System-defined task ID */
	TID uPid;             /* User-defined   task ID */
	PRIO priority;        /* Task priority (0-31) 32 is invalid */
	PRIO realPrio;        /* Real priority (for prio inheritance) */

#if (K_DEF_SCH_TSLICE == ON)
	TICK timeSlice;
	TICK timeLeft;
#endif

    TICK busyWaitTime;

#if (K_DEF_SCH_TSLICE==OFF)
	TICK   lastWakeTime;
#endif

/* Resources */

#if (K_DEF_SEMA == ON)
    K_SEMA* pendingSema;
#endif

#if (K_DEF_MUTEX==ON)
	K_MUTEX* pendingMutx;
#endif
#if (K_DEF_SLEEPWAKE==ON)
	K_EVENT* pendingEv;
#endif

#if (K_DEF_MBOX==ON)
	K_MBOX* pendingMbox;
#endif
	K_TIMER* pendingTmr;

/* Monitoring */

	BOOL   runToCompl;
    BOOL   yield;
    BOOL   timeOut;
	UINT32 lostSignals;
    TID    signalledBy;
	UINT32 nPreempted;
	PID    preemptedBy;

	struct kListNode tcbNode;
} __attribute__((aligned));


struct kRunTime
{
	TICK globalTick;
	UINT32 nWraps;
};
extern struct kRunTime runTime;

typedef struct kTimeoutNode
{
    struct kTimeoutNode *nextPtr;
    TICK timeout;
    ADDR kobj;
    K_OBJ_SYNCH objectType;
} K_TIMEOUT_NODE;


#if (K_DEF_SEMA==ON)

struct kSema
{
	BOOL init;
	INT32 value;
	struct kTcb* owner;
	struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
};

#endif

#if (K_DEF_MUTEX == ON)

struct kMutex
{
	struct kList waitingQueue;
	BOOL lock;
	struct kTcb* ownerPtr;
	BOOL init;
	K_TIMEOUT_NODE timeoutNode;
};
#endif

#if (K_DEF_SLEEPWAKE==ON)

struct kEvent
{
	struct kList waitingQueue;
	BOOL init;
	UINT32 eventID;
	K_TIMEOUT_NODE timeoutNode;

};
#endif /* K_DEF_SLEEPWAKE */

#define MEMBLKLAST (1)

/* Fixed-size pool memory control block (BLOCK POOL) */
struct kMemBlock
{
	BYTE* freeListPtr;
	BYTE* poolPtr;
	BYTE blkSize;
	BYTE nMaxBlocks;
	BYTE nFreeBlocks;
#if (MEMBLKLAST)
	BYTE* lastUsed;
#endif
	BOOL init;
};


#if (K_DEF_MBOX==ON)

#if (K_DEF_MBOX_TYPE==(EXCHANGE))
/* Mailbox (single capcacity)*/
struct kMailbox
{
    BOOL   init;
    ADDR   mailPtr;
    struct kList waitingQueue;
    K_TIMEOUT_NODE timeoutNode;

} __attribute__((aligned(4)));

#elif (K_DEF_MBOX_TYPE==(QUEUE))
/* Mailbox (multi capacity) */
struct kMailbox
{
    BOOL init;
    ADDR mailQPtr;
    UINT headIdx;
    UINT tailIdx;
    SIZE maxItems;
    SIZE countItems;
    struct kList waitingQueue;
    K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));

#endif

#endif



#if ((K_DEF_MESGQ==ON))

/* Message Queue (Stre*/
struct kMesgQ
{
    BOOL init;
    SIZE mesgSize;
    SIZE maxMesg;
    SIZE mesgCnt;
    ADDR buffer;
    SIZE  readIndex;
    SIZE  writeIndex;
    K_TCB* owner;
    struct kList waitingQueue;
	K_TIMEOUT_NODE timeoutNode;
} __attribute__((aligned(4)));

#endif /*K_DEF_MSG_QUEUE*/

#if (K_DEF_PDMESG== ON)

struct kPumpDropBuf
{

     ADDR   dataPtr;
     SIZE   dataSize;                /* mesg size in this buf */
     UINT32 nUsers;                  /* number of tasks using */
};
struct kPumpDropQueue
{
    struct kMemBlock*       memCtrlPtr;    /* associated allocator */
    struct kPumpDropBuf*    currBufPtr;    /* current buffer   */
    UINT32 failReserve;
    BOOL                    init;
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
	K_TIMER* nextPtr;
	BOOL init;
} __attribute__((aligned));


/*[EOF]*/
#ifdef __cplusplus
}
#endif

#endif /* KOBJS_H */
