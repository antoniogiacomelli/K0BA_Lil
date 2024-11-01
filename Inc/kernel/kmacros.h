/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o System Macros/Macros functions
 *
 *****************************************************************************/


#ifndef K_MACROS_H
#define K_MACROS_H

/*
 * brief This is the offset w.r.t the top of a stack frame
 * The numbers are unsigned.
 * */

#define PSR_OFFSET  1 /* Program Status Register offset */
#define PC_OFFSET   2 /* Program Counter offset */
#define LR_OFFSET   3 /* Link Register offset */
#define R12_OFFSET  4 /* R12 Register offset */
#define R3_OFFSET   5 /* R3 Register offset */
#define R2_OFFSET   6 /* R2 Register offset */
#define R1_OFFSET   7 /* R1 Register offset */
#define R0_OFFSET   8 /* R0 Register offset */
#define R11_OFFSET  9 /* R11 Register offset */
#define R10_OFFSET  10 /* R10 Register offset */
#define R9_OFFSET   11 /* R9 Register offset */
#define R8_OFFSET   12 /* R8 Register offset */
#define R7_OFFSET   13 /* R7 Register offset */
#define R6_OFFSET   14 /* R6 Register offset */
#define R5_OFFSET   15 /* R5 Register offset */
#define R4_OFFSET   16 /* R4 Register offset */

#define IDLE_STACKSIZE 		   32
#define TIMHANDLER_STACKSIZE  128
#define TIMHANDLER_TID		  255
#define IDLETASK_TID		  0

#define MSGBUFF_SIZE sizeof(K_MESGBUFF)
#define TIMER_SIZE 	 sizeof(K_TIMER)

#define _N_SYSTASKS      	 2 /*idle task + tim handler*/
#define NTHREADS			 (K_DEF_N_USRTASKS + _N_SYSTASKS)
#define TICK_10MS        	(SystemCoreClock/1000)  /**<  Tick period of 10ms */
#define TICK_5MS        	(SystemCoreClock/2000)  /**< Tick period of 5ms */
#define TICK_1MS         	(SystemCoreClock/10000) /**<  Tick period of 1ms */
#define NPRIO      			(K_DEF_N_PRIO + 1)

/*
 * brief Macro to get the address of the container structure
 * param memberPtr Pointer to the member
 * param containerType The type of the container structure
 * param memberName The name of the member inside the structure
 */

#define K_GET_CONTAINER_ADDR(memberPtr, containerType, memberName) \
    ((containerType *)((unsigned char *)(memberPtr) - \
     offsetof(containerType, memberName)))

/* brief Critical region macro to declare a variable for critical
 * region state */
#define K_CR_AREA  \
    UINT32 crState

/* brief Macro to enter critical region */
#define K_ENTER_CR  \
    do { crState = kEnterCR(); } while(0U)

/* brief Macro to exit critical region */
#define K_EXIT_CR   \
    do { kExitCR(crState); } while(0U)
/*
 * brief Macro for unused variables
 */
#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

/* brief Trigger Context Switch */
#define K_PEND_CTXTSWTCH K_TRAP_PENDSV;

#define READY_HIGHER_PRIO(ptr) ((ptr->priority < runPtr->priority) ? 1 : 0)

/*
 * brief Max value for tick type
 */
#define K_TICK_TYPE_MAX ((1ULL << (8 * sizeof(typeof(TICK)))) - 1)

/*
 * brief Max value for priority type
 */
#define K_PRIO_TYPE_MAX ((1ULL << (8 * sizeof(typeof(PRIO)))) - 1)


/*
 * brief Gets the size of the list
 * param listPtr Pointer to the list
 * return Size of the list
 */
#define K_LIST_SIZE(listPtr) \
    listPtr->size

/*
 * brief Gets the head of the list
 * param listPtr Pointer to the list
 */
#define K_LIST_HEAD(listPtr) \
    listPtr->listDummy.nextPtr

/*
 * brief Gets the dummy node of the list
 * param listPtr Pointer to the list
 */
#define K_LIST_DUMMY(listPtr) \
    &(listPtr->listDummy)

/*
 * brief Gets the tail of the list
 * param listPtr Pointer to the list
 */
#define K_LIST_TAIL(listPtr) \
    listPtr->listDummy.prevPtr

/*
 * brief Checks if the list is empty
 * param listPtr Pointer to the list
 * return TRUE if the list is empty, otherwise FALSE
 */
#define K_LIST_IS_EMPTY(listPtr) \
		((((listPtr->listDummy.nextPtr) == K_LIST_DUMMY(listPtr)) \
		&& (K_LIST_SIZE(listPtr) == 0)))
/*
 * brief Gets the address of the container structure of a node
 * param nodePtr Pointer to the node
 * param containerType Type of the container
 */
#define K_LIST_GET_TCB_NODE(nodePtr, containerType) \
    K_GET_CONTAINER_ADDR(nodePtr, containerType, tcbNode)



/*brief Helper macro to get a message buffer from a mesg list */
#define K_LIST_GET_MESGBUFFER_NODE(nodePtr) \
    K_GET_CONTAINER_ADDR(nodePtr, K_MESGBUFF, mesgNode)


#define IS_NULL_PTR(ptr) ((ptr) == NULL ? 1 : 0)


/*
 * brief Software interrupt (trap) for PendSV
 */
#define K_TRAP_PENDSV  \
    do { SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; } while(0U)

/*
 * brief Software interrupt (trap) for Supervisor Call
 * param N Supervisor call number
 */

#define K_TRAP_SVC(N)  \
    do { asm volatile ("svc %0" :: "i" (N)); } while(0U)


#define K_START_APPLICATION \
	do { asm volatile ("svc #0"); } while(0U)
/*
 * brief Enables the system tick
 */
#define K_TICK_EN  do { SysTick->CTRL |= 0xFFFFFFFF; } while(0U)

/*
 * brief Disables the system tick
 */
#define K_TICK_DIS do { SysTick->CTRL &= 0xFFFFFFFE; } while(0U)

/*
 * brief Macro to convert version numbers to string.
 *
*/
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/*
 * brief Macro to get the version as a string.
 */
#define VERSION_STRING \
    TOSTRING(VERSION.major) "." \
    TOSTRING(VERSION.minor) "." \
    TOSTRING(VERSION.patch)

/* Timer Reload / Oneshot options */
#define RELOAD 		1
#define ONESHOT		0

#if		(K_DEF_TRACE_NO_INFO==ON)
#define K_TRACE(event) kTrace(event, NULL)
#else
#define K_TRACE(event, info) kTrace(event, info)
#endif

#endif /*K_MACROS_H*/
