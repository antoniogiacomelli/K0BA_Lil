/**
 *****************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 *\file kobjs.h
 *\brief    Kernel objects
 *\version  1.1.0
 *\author   Antonio Giacomelli

 * \verbatim
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
 *		 Timers						| ktimer.c
 *		 Tracer						| ktracer.c
 *
 * \endverbatim
 *****************************************************************************/
#ifndef INC_KOBJS_H
#define INC_KOBJS_H

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/**
 *\brief Node structure for general circular doubly linked list
 */
struct kListNode
{
    struct kListNode* nextPtr; /**< Pointer to next node */
    struct kListNode* prevPtr; /**< Pointer to previous node */
};

/**
 *\brief  Circular doubly linked list structure
 */
struct kList
{
    struct kListNode  listDummy; /**< Dummy node for head/tail management */
    STRING			  listName;  /**< List name */
    UINT32            size;      /**< Number of items in the list */
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 * \brief Task Control Block
 */
struct kTcb
{
    UINT32*         sp;           /**< Saved stack pointer */
    K_TASK_STATUS   status;       /**< Task status */
    UINT32          runCnt;       /**< Dispatch count */
    UINT32*         stackAddrPtr; /**< Stack address */
    UINT32          stackSize;    /**< Stack size */
    PID           	pid;          /**< System-defined task ID */
    TID         	uPid;         /**< User-defined   task ID */
    PRIO          	priority;     /**< Task priority (0-254) 255 is invalid */
    PRIO			realPrio;	  /**< Real priority (for prio inheritance) */
    TICK          	timeSlice;    /**< Time-slice duration 		   */
    TICK          	timeLeft;     /**< Remaining time-slice 	   */
    STRING          taskName;     /**< Task name 				   */
    TICK          	busyWaitTime; /**< Delay in ticks 			   */
    ADDR			pendingObj;	  /**< Address obj task is blocked */
    BOOL			runToCompl;	  /**< Cooperative-only task 	   */
    struct kListNode tcbNode;     /**< Aggregated list node 	   */
}__attribute__((aligned));

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/**
 *\brief Record of ticks
 */
struct kRunTime
{
    TICK  globalTick; /**< Global system tick */
    UINT32  nWraps;     /**< Number of tick wraps */
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 *\brief  Counter Semaphore
 */
struct kSema
{
	INT32   	  	value;		/**< Semaphore value */
	struct kList  	queue;		/**< Semaphore waiting queue */
};

/**
*\brief Mutex
*/

struct kMutex
{
    struct kList 	queue;	  /**< Mutex waiting queue */
    BOOL 			lock; 	  /**< 0=unlocked, 1=locked */
    struct kTcb* 	ownerPtr; /**< Task owning the mutex */
    BOOL			init;	  /**< Init state */
};
#if (K_DEF_COND == ON)
/**
*\brief Condition Variables
*/

struct kCond
{
	struct kMutex   condMutex; /**< Lock */
	struct kList	queue;     /**< Waiting queue */
};
#endif /*K_DEF_COND*/
/**
 *\brief Generic Event
 */
struct kEvent
{
	struct kList		 queue;   /**< Waiting queue */
	BOOL				 init;    /**< Init flag */
	UINT32			     eventID; /**< event ID */
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 *\brief Fixed-size pool memory control block (BLOCK POOL)
 */
struct kMemBlock
{
    BYTE* freeListPtr;    /**< Pointer to the head of the free list*/
    BYTE blkSize;         /**< Size of each block (in bytes) */
    BYTE nMaxBlocks;      /**< Total number of blocks in the pool */
    BYTE nFreeBlocks;     /**< Current number of free blocks available */
    K_MUTEX poolMutex;	  /**< Pool lock */
};
/**
 * \brief Byte pool Memory Control Block (BYTE POOL)
 */
struct kMemByte
{
	BYTE* memPoolPtr;  /* Pool of bytes */
	UINT16 freeList;   /* Packed free list: 2-byte (index, next index) */
	BYTE poolSize;     /* Total size of the pool */
	BYTE nFreeBytes;   /* Number of free bytes available */
};


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#if ((K_DEF_MESGQ==ON))

/**
 *\brief Message Buffer for Message and Mailbox
 */
struct kMesgBuff
{
	struct kListNode		mesgNode;  /**< Mesg Queue Node */
	TID						senderTid; /**< Sender Task ID*/
	ADDR  					mesgPtr;   /**< Pointer to message contents */
	SIZE 	   			    mesgSize;  /**< Mesg size */
}__attribute__((aligned));

/**
*\brief Message Queue
*/
struct kMesgQ
{
	struct kList	 mesgList;   /**< Linked list of messages */
	struct kSema     semaItem;	 /**< Semaphore indicating a new message */
	struct kMutex    mutex;		 /**< Mutex for accessing message queue */
	struct kSema	 semaPool;   /**< Counter Semaphore for associated the mesg pool */
	struct kSema	 semaRoom;   /**< Counter Semaphore for items in the queue */
	struct kMemBlock mesgMemCtrlBlk; /**< Mem Ctrl Block of the associated mesg pool */
};
#endif /*K_DEF_MSG_QUEUE*/

#if (K_DEF_MAILBOX==ON)

/**
 *\brief Mail within a mailbox
 */
struct kMail
{
	TID			senderTid; /**< Sender TID */
	SIZE 	   	mailSize;  /**< Mail size */
	ADDR 		mailPtr;   /**< Mail address */
	BYTE	    mail[K_DEF_MAIL_SIZE]; /**< Mail Contents - Default: 4 bytes */

}__attribute__((aligned));

/**
 * \brief Mailbox
 */
struct kMailbox
{
#if (K_DEF_MAILBOX_ACK == ON)
	struct kSema		 semaAck; /**< Semaphore to ACK sender*/
#endif
	struct kSema  	 	 semaEmpty; /**< Signal/Wait producer/consumer */
	struct kMutex  	 	 mutex;     /**< Lock */
	struct kSema  	 	 semaFull;  /**< Signal/Wait consumer/producer */
	struct kMail	 	 mail;      /**< Message struct */
} __attribute__((aligned));

#endif /* K_DEF_MAILBOX_ACK */

#if (K_DEF_COND== ON)
	#if (K_DEF_PIPE==ON)
/**
 * \brief Pipes
 *
 */
	struct kPipe
	{
		UINT32 			 tail;      /**< read index */
		UINT32			 head;      /**< write index */
		UINT32 			 data; 	    /**< Number of bytes in the buffer */
		UINT32 	   		 room;      /**< Number of free slots in the buffer (bytes) */
		struct kCond	 condRoom;  /**< Cond Var for writers */
		struct kCond 	 condData;  /**< Cond Var for readers */
		struct kMutex	 mutex;     /**< Lock */
		BYTE		 	 buffer[K_DEF_PIPE_SIZE]; /**< Data buffer */
	};
	#endif /* K_DEF_PIPES */
#endif /* K_DEF_COND */

/**
 *\brief FIFO (simple pipe)
 */
struct kFifo
{
	UINT32				head; 		/**< write index */
	UINT32				tail; 		/**< read index */
	struct kSema	    semaItem;   /**< signal/wait consumer/producer */
	struct kSema		semaRoom;   /**< signal/wait producer/consumer */
	struct kMutex		mutex;      /**< Lock */
	BYTE				buffer[K_DEF_FIFO_SIZE]; /**< the data */
}__attribute__((aligned));

struct kCircBuff
{
	UINT32				head; 		/**< write index */
	UINT32				tail; 		/**< read index */
	BYTE				buffer[K_DEF_FIFO_SIZE]; /**< the data */
}__attribute__((aligned));

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 *\brief Application Timer
 */
struct kTimer
{
    STRING  	timerName; /**< Timer name */
	TICK  		dTicks;    /**< Delta ticks */
	TICK		ticks;     /**< Aboslute ticks */
	BOOL		reload;    /**< Reload/Oneshot */
	CALLBACK 	funPtr;    /**< Callback */
	ADDR    	argsPtr;   /**< Arguments */
    K_TIMER* 	nextPtr;   /**< Next timer in the queue */
}__attribute__((aligned));

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#if (K_DEF_TRACE==ON)
/**
 *\brief  Tracer Data
 */
struct kTraceData
{
	TICK 		timeStamp; /**< Current ticks when logged */
	K_TRACEID 	event;     /**< Event id */
#if (K_DEF_TRACE_NO_INFO == OFF)
	STRING   info;      /**< custom info */
#endif
	PID			pid;       /**< Which task was running */

};
/**
 * \brief The tracer.
 */
struct kTrace
{
	UINT32	 head;		/**< buffer head index */
	UINT32 	 tail;      /**< buffer tail index */
	UINT32 	 nAdded;    /**< number of items added */
	UINT32	 nWrap;     /**< number of buffer wrap-arounds */
	struct kTraceData buffer[K_DEF_TRACEBUFF_SIZE]; /**< trace data */
};
#endif /*K_DEF_TRACE */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


#endif /* K_OBJS_H */
