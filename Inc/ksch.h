/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
 *
 ******************************************************************************
 ******************************************************************************
 *  In this header:
 *                  o High-level scheduler kernel module.
 *
 *****************************************************************************/
#ifndef KSCH_H
#define KSCH_H
#ifdef __cplusplus
{
#endif

/* shared data */
extern K_TCB* runPtr; /* Pointer to the running TCB */
extern K_TCB tcbs[NTHREADS]; /* Pool of TCBs */
extern volatile K_FAULT faultID; /* Fault ID */
extern INT idleStack[IDLE_STACKSIZE]; /* Stack for idle task */
extern INT timerHandlerStack[TIMHANDLER_STACKSIZE];
extern K_TCBQ readyQueue[K_DEF_MIN_PRIO + 2]; /* Table of ready queues */
extern K_TCBQ timeOutQueue;
extern volatile UINT startup;

BOOL kSchNeedReschedule(K_TCB*);
VOID kSchSwtch(VOID);
UINT kEnterCR(VOID);
VOID kExitCR(UINT);
VOID kInit(VOID);
VOID kYield(VOID);
VOID kApplicationInit(VOID);
extern unsigned __getReadyPrio(unsigned);
K_ERR kTCBQInit(K_LIST* const, STRING);
K_ERR kTCBQEnq(K_LIST* const, K_TCB* const);
K_ERR kTCBQDeq(K_TCBQ* const, K_TCB** const);
K_ERR kTCBQRem(K_TCBQ* const, K_TCB** const);
K_ERR kReadyCtxtSwtch(K_TCB* const);
K_ERR kReadyQDeq(K_TCB** const, PRIO);
K_TCB* kTCBQPeek(K_TCBQ* const);
K_ERR kTCBQEnqByPrio(K_TCBQ* const, K_TCB* const);

/* Doubly Linked List ADT */

#define K_LIST_GET_TCB_NODE(nodePtr, containerType) \
    K_GET_CONTAINER_ADDR(nodePtr, containerType, tcbNode)

/*brief Helper macro to get a message buffer from a mesg list */
#define K_LIST_GET_SYSMESG_NODE(nodePtr) \
    K_GET_CONTAINER_ADDR(nodePtr, K_MESG, mesgNode)

#define KLISTNODEDEL(nodePtr) \
    do \
    \
    { \
    nodePtr->nextPtr->prevPtr = nodePtr->prevPtr; \
    nodePtr->prevPtr->nextPtr = nodePtr->nextPtr; \
    nodePtr->prevPtr=NULL; \
    nodePtr->nextPtr=NULL; \
    \
    }while(0U)

__attribute__((always_inline)) static inline K_ERR kListInit(K_LIST* const kobj,
        STRING listName)
{
    if (kobj == NULL)
    {
        return (K_ERR_OBJ_NULL);
    }
    kobj->listDummy.nextPtr = & (kobj->listDummy);
    kobj->listDummy.prevPtr = & (kobj->listDummy);
    kobj->listName = listName;
    kobj->size = 0U;
    kobj->init=TRUE;
    DMB/*guarantee data is updated before going*/
    return (K_SUCCESS);
}
__attribute__((always_inline))    static inline K_ERR kListInsertAfter(
        K_LIST* const kobj, K_NODE* const refNodePtr,
        K_NODE* const newNodePtr)
{
    if (kobj == NULL || newNodePtr == NULL || refNodePtr == NULL)
    {
        return (K_ERR_OBJ_NULL);
    }
    newNodePtr->nextPtr = refNodePtr->nextPtr;
    refNodePtr->nextPtr->prevPtr = newNodePtr;
    newNodePtr->prevPtr = refNodePtr;
    refNodePtr->nextPtr = newNodePtr;
    kobj->size += 1U;

    DMB
    return (K_SUCCESS);
}
__attribute__((always_inline))    static inline K_ERR kListRemove(K_LIST* const kobj,
        K_NODE* const remNodePtr)
{
    if (kobj == NULL || remNodePtr == NULL)
    {
        return (K_ERR_OBJ_NULL);
    }
    if (kobj->size == 0)
    {
        return (K_ERR_LIST_EMPTY);
    }
    KLISTNODEDEL(remNodePtr);
    kobj->size -= 1U;
    DMB
    return (K_SUCCESS);
}

__attribute__((always_inline))    static inline K_ERR kListRemoveHead(
        K_LIST* const kobj, K_NODE** const remNodePPtr)
{

    if (kobj->size == 0)
    {
        return (K_ERR_LIST_EMPTY);
    }

    K_NODE* currHeadPtr = kobj->listDummy.nextPtr;
    *remNodePPtr = currHeadPtr;
    KLISTNODEDEL(currHeadPtr);
    kobj->size -= 1U;
    DMB
    return (K_SUCCESS);
}
__attribute__((always_inline))    static inline K_ERR kListAddTail(
        K_LIST* const kobj, K_NODE* const newNodePtr)
{
    return (kListInsertAfter(kobj, kobj->listDummy.prevPtr, newNodePtr));
}

__attribute__((always_inline))    static inline K_ERR kListAddHead(
        K_LIST* const kobj, K_NODE* const newNodePtr)
{

    return (kListInsertAfter(kobj, &kobj->listDummy, newNodePtr));
}

__attribute__((always_inline))

static inline K_ERR kListRemoveTail(K_LIST* const kobj, K_NODE** remNodePPtr)
{
    if (kobj->size == 0)
    {
        return (K_ERR_LIST_EMPTY);
    }

    K_NODE* currTailPtr = kobj->listDummy.prevPtr;
    if (currTailPtr != NULL)
    {
        *remNodePPtr = currTailPtr;
        KLISTNODEDEL(currTailPtr);
        kobj->size -= 1U;
        DMB
        return (K_SUCCESS);
    }
    return (K_ERR_OBJ_NULL);
}

#ifdef __cplusplus
}
#endif

#endif /* KSCH_H */
