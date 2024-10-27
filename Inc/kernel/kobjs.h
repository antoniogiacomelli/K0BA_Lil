/*****************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 *
 * 	In this header:
 *
 * 		o Definition of all kernel objects:
 *
 * 		 OBJECT TYPE 				| IMPLEMENTED IN
 * 		 ---------------------------------------------
 * 		 Lists 		  				| klist.c
 * 		 Task Control Block 		| ktcb.c
 *		 Run-time record			| ktick.c
 *		 Inter-task synchronisation	| ksynch.c
 *		 Inter-task communication	| kmesg.c
 *		 Memory Pool Control Block	| kmem.c
 *		 Timers						| ktimer.h
 *		 Tracer						| ktracer.c
 *
 *****************************************************************************/
#ifndef INC_KOBJS_H_
#define INC_KOBJS_H_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*
 * Node structure for general circular doubly linked list
 */
struct kListNode
{
    struct kListNode* nextPtr; /* Pointer to next node */
    struct kListNode* prevPtr; /* Pointer to previous node */
};

/*
 *  Circular doubly linked list structure
 */
struct kList
{
    struct kListNode  listDummy; /* Dummy node for head/tail management */
    STRING			  listName;
    UINT32            size;
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


struct kTcb
{
    UINT32*         sp;           /* Saved stack pointer */
    K_TASK_STATUS   status;       /* Task status */
    UINT32          runCnt;       /* Dispatch count */
    UINT32*         stackAddrPtr; /* Stack address */
    UINT32          stackSize;    /* Stack size */
    PID           	pid;          /* System task ID */
    TID         	uPid;         /* User-defined task ID */
    PRIO          	priority;     /* Task priority (0-254) 255 is invalid */
    PRIO			realPrio;	  /* Real priority (priority inheritance) */
    TICK          	timeSlice;    /* Time-slice duration 		   */
    TICK          	timeLeft;     /* Remaining time-slice 	   */
    STRING          taskName;     /* Task name 				   */
    TICK          	busyWaitTime; /* Delay in ticks 			   */
    ADDR			pendingObj;	  /* Address obj task is blocked */
    BOOL			runToCompl;	  /* Cooperative-only task 	   */
    struct kListNode tcbNode;    /* Aggregated list node 	   */
}__attribute__((aligned));

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*
 * Record of ticks
 */
struct kRunTime
{
    TICK  globalTick; /* Global system tick */
    UINT32  nWraps;     /* Number of tick wraps */
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
 *  Counter Semaphore
 */
struct kSema
{
	INT32   	  	value;		/* Semaphore value */
	struct kList  	queue;		/* Semaphore waiting queue */
	struct kTcb*	ownerPtr;
};

/*
* Mutex
*/

struct kMutex
{
    struct kList 	queue;	  /* Mutex waiting queue */
    BOOL 			lock; 	  /* 0=unlocked, 1=locked */
    struct kTcb* 	ownerPtr; /* Pointer to current ownerPtr */
};
#if (K_CONF_COND_VAR == ON)
/*
* Condition Variables
*/

struct kCond
{
	struct kMutex   condMutex;
	struct kList	queue;
};
#endif
/*
 * Generic Event
 */
struct kEvent
{
	struct kList		 queue;
	BOOL				 init;
	UINT32			     eventID;
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
 * Memory Pool Control Block
 */

struct kMem
{
	BYTE*  			freeListPtr;   /* Free list pointer */
    BYTE    		blkSize;       /* Block size */
    BYTE    		nMaxBlocks;    /* Maximum number of blocks */
    BYTE    		nFreeBlocks;   /* Number of free blocks */
    struct kMutex	poolMutex;	   /* pool mutex; */

}__attribute__((aligned));
#if ((K_CONF_MSG_QUEUE==ON) || (K_CONF_MAILBOX == ON))
/*
* Message contents within a message buffer
*/
#if 0
union kMesg
{
	BYTE		mesgCHAR[MSG_SIZE]; /* Message in bytes */
	UINT32		mesgU32;			   /* Message as unsigned integer 32-bit */
};
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
 * Message Buffer for Message and Mailbox
 */
struct kMesgBuff
{
	struct kListNode		mesgNode;
	TID						senderTid;
	ADDR  					mesgPtr;
	SIZE 	   			    mesgSize;
}__attribute__((aligned));
#endif

#if (K_CONF_MSG_QUEUE==ON)
/*
*  Message Queue
*/
struct kMesgQ
{
	struct kList		 	 mesgList;		 /* Linked list of messages */
	struct kSema     		 semaItem;	 /* Semaphore indicating a new message */
	struct kMutex			 mutex;		 /* Mutex for accessing message queue */
	struct kSema			 semaPool;
	struct kSema			 semaRoom;
	struct kMem				 mesgMemCtrlBlk;
};
#endif
#if (K_CONF_MAILBOX==ON)

/*
 * Mailbox
 */
struct kMail
{
	TID					  senderTid;
	ADDR  				  mesgPtr;
	SIZE 	   			  mesgSize;
}__attribute__((aligned));

struct kMailbox
{
#if (K_CONF_MAILBOX_ACK == ON)
	struct kSema		 semaAck;
#endif
	struct kSema  	 	 semaEmpty;
	struct kMutex  	 	 mutex;
	struct kSema  	 	 semaFull;
	struct kMail	 	 mail;
} __attribute__((aligned));

#endif
#if (K_CONF_COND_VAR== ON)


#endif

#if (K_CONF_PIPES==ON)
/*
 *  Pipes
 */
struct kPipe
{
	UINT32 			 tail;      // Where to read
	UINT32			 head;      // Where to write
	UINT32 			 data; 	    // Number of bytes in the buffer
	UINT32 	   		 room;      // number of free entries in the buffer
	struct kCond	 condRoom;
	struct kCond 	 condData;
	struct kMutex	 mutex;
	BYTE		 	 buffer[PIPE_SIZE]; // The data
};
#endif

/*
 * FIFO (simple pipe)
 */
struct kFifo
{
	UINT32				head;
	UINT32				tail;
	struct kSema	    semaItem;
	struct kSema		semaRoom;
	struct kMutex		mutex;
	BYTE				buffer[FIFO_SIZE];
}__attribute__((aligned)) ;

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
 * Application Timer
 */
struct kTimer
{
    STRING  	timerName;
	TICK  		dTicks;
	TICK		ticks;
	BOOL		reload;
	CALLBACK 	funPtr;
	ADDR    	argsPtr;
    K_TIMER* 	nextPtr;
}__attribute__((aligned));

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#if (K_CONF_TRACE==ON)
/*
 *  Tracer
 */
struct kTraceData
{
	TICK 		timeStamp;
	K_TRACEID 	event;
	STRING	    info;
	PID			pid;

};

struct kTrace
{
	UINT32	 head;
	UINT32 	 tail;
	UINT32 	 nAdded;
	UINT32	 nWrap;
	STRING   info;
	struct kTraceData buffer[TRACEBUFF_SIZE];
};
#endif

/******************************************************************************/
/******************************************************************************/

//[EOF]

#endif /* INC_KOBJS_H_ */
