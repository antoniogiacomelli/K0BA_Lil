/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Memory Pool Control Block
 *
 *****************************************************************************/
#include <kapi.h>

K_ERR kMemInit(K_MEM* const self, ADDR const memPoolPtr, BYTE blkSize, const BYTE numBlocks)
{
	K_CR_AREA;
	K_ENTER_CR;

	/*round up to next value multiple of 4 (if not a multiple)*/
	blkSize = (blkSize + 0x03) & 0xFC;

	/* initialise freelist of blocks */

	BYTE* blockPtr = (BYTE*)memPoolPtr;    /* first byte address in the pool  */
	ADDR* nextAddrPtr = (ADDR*)memPoolPtr; /* next block address */

	for (BYTE i = 0; i < numBlocks - 1; i++)
	{
		/* init pool by advancing each block addr by blkSize bytes */
		blockPtr +=  blkSize;
		/* save blockPtr addr as the next */
		*nextAddrPtr = (ADDR)blockPtr;
		/* update  */
		nextAddrPtr = (ADDR*)(blockPtr);

	}
	*nextAddrPtr = NULL;

	/* Initialize the control block */
	self->blkSize     = blkSize;
	self->nMaxBlocks  = numBlocks;
	self->nFreeBlocks = numBlocks;
	self->freeListPtr = memPoolPtr;
	K_EXIT_CR;
	return K_SUCCESS;
}

ADDR kMemAlloc(K_MEM* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
    if (self->nFreeBlocks==0)
    {
    	K_ENTER_CR;
        return NULL;
    }
    // Allocate the first free block

    ADDR allocPtr = self->freeListPtr;
    self->freeListPtr = *(ADDR*)allocPtr;  // Update the free list to the next free block
    if (allocPtr != NULL)
    	self->nFreeBlocks -= 1;
    K_EXIT_CR;
    return allocPtr;  // Return the allocated block
}

K_ERR kMemFree(K_MEM* const self, ADDR const blockPtr)
{

	if ((self == NULL) || (blockPtr == NULL))
	{
		return K_ERROR;
	}
	*(ADDR*)blockPtr = self->freeListPtr;
    self->freeListPtr = blockPtr;
    self->nFreeBlocks += 1U;
    return K_SUCCESS;
}


