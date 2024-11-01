/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Private API: List ADT
 *
 *****************************************************************************/


#ifndef KLIST_H
#define KLIST_H

/*
 *  Initialize a linked list
 * param self Pointer to the list
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListInit(K_LIST* const self, STRING listName);

/*
 *  Remove a node after the specified reference node
 * param self Pointer to the list
 * param refNodePtr Reference node
 * param remNodePPtr Pointer to the node to be removed
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListRemAfter(K_LIST* const self, K_LISTNODE* const refNodePtr,
		K_LISTNODE** const remNodePPtr);

/*
 *  Insert a new node after the specified reference node
 * param self Pointer to the list
 * param refNodePtr Reference node
 * param newNodePtr New node to insert
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListInsertAfter(K_LIST* const self, K_LISTNODE* const refNodePtr,
		K_LISTNODE* const newNodePtr);

/*
 *  Get the tail node of the list
 * param self Pointer to the list
 * return Pointer to the tail node
 */
K_LISTNODE* kListGetTail(K_LIST* const self);

/*
 *  Remove a node from the list
 * param selfPtr Pointer to the list
 * param remNodePtr Pointer to the node to remove
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListRemove(K_LIST* const selfPtr, K_LISTNODE* const remNodePtr);

/*
 *  Remove the head node of the list
 * param self Pointer to the list
 * param remNodePPtr Pointer to the node to be removed
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListRemoveHead(K_LIST* const self, K_LISTNODE** const remNodePPtr);
/*
 *  Add to the tail node of the list
 * param self Pointer to the list
 * param remNodePPtr Pointer to the node to be removed
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kListAddTail(K_LIST* const self, K_LISTNODE* const newNodePtr);
/*****************************************************************************/
/*																			  */
/* TCB QUEUE MANAGEMENT														  */
/*																			  */
/*****************************************************************************/

/*
 *  Initialize a TCB queue
 * param self Pointer to the queue
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kTCBQInit(K_LIST* const self, STRING listName);

/*
 *  Enqueue a TCB into a TCB queue
 * param self Pointer to the queue
 * param tcbPtr Pointer to the TCB to enqueue
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kTCBQEnq(K_LIST* const self, K_TCB* const tcbPtr);

/*
 *  Dequeue a TCB from a TCB queue
 * param self Pointer to the queue
 * param tcbPPtr Pointer to the dequeued TCB
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kTCBQDeq(K_TCBQ* const self, K_TCB** const tcbPPtr);

/*
 *  Removes a specific TCB from a TCB queue
 * param self Pointer to the queue
 * param tcbPPtr Pointer to the TCB address (double pointer) \
 *        (e.g.: K_TCB** tcbPPtr = &tcbs[2])
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kTCBQRem(K_TCBQ* const self, K_TCB** const tcbPPtr);
/*
 *  Enqueue a task into the ready queue
 * param tcbPtr Pointer to the task TCB
 * return none
 */
K_ERR kReadyQEnq(K_TCB* const tcbPtr);

/*
 *  Dequeue a task from the ready queue
 * param tcbPPtr Pointer to the dequeued task TCB
 * param priority Priority of the task to dequeue
 * return K_SUCCESS on success, K_ERROR on failure
 */
K_ERR kReadyQDeq(K_TCB** const tcbPPtr, PRIO priority);


#endif /* KLIST_H */
