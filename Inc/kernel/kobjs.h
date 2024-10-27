
/*****************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]
 *
 ******************************************************************************/
/**
 *\file kobjs.h
 *\brief    Kernel objects
 *\version  0.1.0
 *\author   Antonio Giacomelli
 *
 *
 ******************************************************************************
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
 *		 Timers						| ktimer.h
 *		 Tracer						| ktracer.c
 *
 * \endverbatim
 *****************************************************************************/
#ifndef INC_KOBJS_H_
#define INC_KOBJS_H_

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
    STRING			  listName;
    UINT32            size;
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
    PID           	pid;          /**< System task ID */
    TID         	uPid;         /**< User-defined task ID */
    PRIO          	priority;     /**< Task priority (0-254) 255 is invalid */
    PRIO			realPrio;	  /**< Real priority (priority inheritance) */
    TICK          	timeSlice;    /**< Time-slice duration 		   */
    TICK          	timeLeft;     /**< Remaining time-slice 	   */
    STRING          taskName;     /**< Task name 				   */
    TICK          	busyWaitTime; /**< Delay in ticks 			   */
    ADDR			pendingObj;	  /**< Address obj task is blocked */
    BOOL			runToCompl;	  /**< Cooperative-only task 	   */
    struct kListNode tcbNode;    /**< Aggregated list node 	   */
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
    BOOL 			lock; 	  /** 0=unlocked, 1=locked */
    struct kTcb* 	ownerPtr; /** Task owning the mutex */
};
#if (K_CONF_COND_VAR == ON)
/**
*\brief Condition Variables
*/

struct kCond
{
	struct kMutex   condMutex; /**< Lock */
	struct kList	queue;     /**< Waiting queue */
};
#endif
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
 *\brief Memory Pool Control Block
 */

struct kMem
{
	BYTE*  			freeListPtr;   /**< Free list pointer */
    BYTE    		blkSize;       /**< Block size */
    BYTE    		nMaxBlocks;    /**< Maximum number of blocks */
    BYTE    		nFreeBlocks;   /**< Number of free blocks */
    struct kMutex	poolMutex;	   /**< pool mutex; */

}__attribute__((aligned));
#if ((K_CONF_MSG_QUEUE==ON) || (K_CONF_MAILBOX == ON))

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

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
#endif

#if (K_CONF_MSG_QUEUE==ON)
/**
*\brief Message Queue
*/
struct kMesgQ
{
	struct kList		 	 mesgList;   /**< Linked list of messages */
	struct kSema     		 semaItem;	 /**< Semaphore indicating a new message */
	struct kMutex			 mutex;		 /**< Mutex for accessing message queue */
	struct kSema			 semaPool;   /**< Counter Semaphore for associated the mesg pool */
	struct kSema			 semaRoom;   /**< Counter Semaphore for items in the queue */
	struct kMem				 mesgMemCtrlBlk; /**< Mem Ctrl Block of the associated mesg pool */
};
#endif
#if (K_CONF_MAILBOX==ON)

/**
 *\brief Mail within a mailbox
 */
struct kMail
{
	TID					  senderTid; /**< Sender TID */
	ADDR  				  mesgPtr;   /**< Pointer to message contents */
	SIZE 	   			  mesgSize;  /**< Message size */
}__attribute__((aligned));

/**
 * \brief Mailbox
 */
struct kMailbox
{
#if (K_CONF_MAILBOX_ACK == ON)
	struct kSema		 semaAck; /**< Semaphore to ACK sender*/
#endif
	struct kSema  	 	 semaEmpty; /**< Signal/Wait producer/consumer */
	struct kMutex  	 	 mutex;     /**< Lock */
	struct kSema  	 	 semaFull;  /**< Signal/Wait consumer/producer */
	struct kMail	 	 mail;      /**< Message struct */
} __attribute__((aligned));

#endif
#if (K_CONF_COND_VAR== ON)


#endif

#if (K_CONF_PIPES==ON)
/**
 *\brief  Pipes
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
	BYTE		 	 buffer[PIPE_SIZE]; /**< Data buffer */
};
#endif

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
	BYTE				buffer[FIFO_SIZE];
}__attribute__((aligned)) ;

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

#if (K_CONF_TRACE==ON)
/**
 *\brief  Tracer Data
 */
struct kTraceData
{
	TICK 		timeStamp; /**< Current ticks when logged */
	K_TRACEID 	event;     /**< Event id */
	STRING	    info;      /**< Info */
	PID			pid;       /**< Which task was running */

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



#endif /* INC_KOBJS_H_ */
