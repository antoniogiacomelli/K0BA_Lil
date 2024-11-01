/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 *					o ADT for Circular Doubly Linked List
 *
 ******************************************************************************

 This Linked List has a sentinel (dummy node), whose addres
 is fixed on initialisation. Thus, every node in the list
 has an anchored reference, making it easier to handle edge
 cases.
 This ADT is aggregated to other system structures whenever
 an ordered list is required.

				  HEAD                                TAIL
	   _____     _____     _____     _____           _____
	  |     |-->|     |-->|     |-->|     |-->   <--|     |
	  |DUMMY|   |  H  |   | H+1 |   | H+2 |  . . .  |  T  |
	<-|_____|<--|_____|<--|_____|<--|_____|         |_____|-->
	|________________________________________________________|


	 - INITIALISE

	 The list is initialised by declaring a node, and assigning its previous
	 and next pointers to itself. This is the anchored reference.
			  _____
		   __|__   |
		  |     |-->
		  |DUMMY|
		<-|_____|
		|____|


	 - INSERT AFTER

		When list is empty we are inserting the head (that is also the tail).
		If not empty:
		 To insert on the head, reference node is the dummy.
		 To insert on the tail, reference node is the current tail
		 (dummy's previous node).
				 _____     _____     _____
				| ref |-> | new |-> | old |->
		   . . .|     |   |next |   | next| . . .
			  <-|_____| <-|_____| <-|_____|

	 - REMOVE A NODE

	  	 To remove a node we "cut off" its next and previous links, rearranging
	  	 as: node.prev.next = node.next; node.next.prev = node.prev;

					     _______________
				 _____  |  _____     ___|_
				|     |-> |     x   |     |->
		   . . .|prev |   |node |   | next| . . .
			  <-|_____|   x_____| <-|_____|
				   |______________|


	   To remove the head, we remove the dummy's next node
	   To remove the tail, we remove the dummy's previous node
	   In both cases, the list adjusts itself
******************************************************************************/

#define K_CODE
#include "kapi.h"
#include "kglobals.h"

static inline void kListNodeDel_(K_LISTNODE* nodePtr)
{
    nodePtr->nextPtr->prevPtr = nodePtr->prevPtr;
    nodePtr->prevPtr->nextPtr = nodePtr->nextPtr;
    nodePtr->prevPtr=NULL;
    nodePtr->nextPtr=NULL;
}


/* Initialize the list by setting head pointers and size */
K_ERR kListInit(K_LIST* const self, STRING listName)
{
    if (self == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    /* Initialize the list to be circular by pointing to itself */
    self->listDummy.nextPtr = K_LIST_DUMMY(self);
    self->listDummy.prevPtr = K_LIST_DUMMY(self);
    self->listName = listName;
    self->size = 0U;

    return K_SUCCESS;
}

K_ERR kListInsertAfter(K_LIST* const self, K_LISTNODE* const refNodePtr,
		K_LISTNODE* const newNodePtr)
{
    if (self == NULL || newNodePtr == NULL || refNodePtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    newNodePtr->nextPtr = refNodePtr->nextPtr;
    refNodePtr->nextPtr->prevPtr = newNodePtr;
    newNodePtr->prevPtr = refNodePtr;
    refNodePtr->nextPtr = newNodePtr;

    /* Increment the list size */
    self->size += 1;
    __DMB();
    return K_SUCCESS;
}

/* Remove a node from the list */
K_ERR kListRemove(K_LIST* const self, K_LISTNODE* const remNodePtr)
{
    if (self == NULL || remNodePtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    if (self->size==0)
    {
    	assert(0);
        return K_ERR_LIST_EMPTY;
    }
    kListNodeDel_(remNodePtr);
    self->size -= 1;
    __DMB();
    return K_SUCCESS;
}


/* Remove the head node from the list */
K_ERR kListRemoveHead(K_LIST* const self, K_LISTNODE** const remNodePPtr)
{
    if (self == NULL || remNodePPtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }

    /* If the list is empty */
    if (K_LIST_IS_EMPTY(self))
    {
        return K_ERR_LIST_EMPTY;
    }

    K_LISTNODE* currHeadPtr = K_LIST_HEAD(self);
    *remNodePPtr = currHeadPtr;
    /* Use list_del to unlink the head node */
    kListNodeDel_(currHeadPtr);

    /* Decrement the list size */
    self->size -= 1;
    __DMB();
    return K_SUCCESS;
}

K_ERR kListAddTail(K_LIST* const self, K_LISTNODE* const newNodePtr)
{
    if (self == NULL || newNodePtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    return kListInsertAfter(self, self->listDummy.prevPtr, newNodePtr);
}

K_ERR kListRemoveTail(K_LIST* const self, K_LISTNODE** remNodePPtr)
{
    if (self == NULL || remNodePPtr == NULL)
    {
        return K_ERR_NULL_OBJ;
    }
    if (K_LIST_IS_EMPTY(self))
    {
        return K_ERR_LIST_EMPTY;
    }

    K_LISTNODE* currTailPtr = K_LIST_TAIL(self);
    *remNodePPtr = currTailPtr;
    kListNodeDel_(currTailPtr);
    /* Decrement the list size */
    self->size -= 1;
    __DMB();
    return K_SUCCESS;
}
