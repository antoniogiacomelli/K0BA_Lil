/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 					o Block Pool Control Block
 *					o Byte Pool Control Block
 *
 *****************************************************************************/

#define K_CODE
#include "kapi.h"

/*******************************************************************************
 *
 * BLOCK POOL CONTROL BLOCK
 *
 ******************************************************************************/
//let me kick some old simplistic embeddded systems programming sht
 
K_ERR kBlockPoolInit(K_BLOCKPOOL* const self, ADDR const memPoolPtr, \
		BYTE blkSize, const BYTE numBlocks)
{
	K_CR_AREA;
	K_ENTER_CR;

	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		return K_ERR_MEM_INIT;
	}
#if (K_DEF_MEMBLOCK_ALIGN_4==ON)
	/*round up to next value multiple of 4 (if not a multiple)*/
	blkSize = (blkSize + 0x03) & 0xFC;
#endif
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
	kMutexInit(&self->poolMutex);
	K_EXIT_CR;
	return K_SUCCESS;
}

ADDR kBlockPoolAlloc(K_BLOCKPOOL* const self)
{
	K_CR_AREA;
	K_ENTER_CR;
    if (self->nFreeBlocks==0)
    {
    	K_ENTER_CR;
        return NULL;
    }
    ADDR allocPtr = self->freeListPtr;
    self->freeListPtr = *(ADDR*)allocPtr;
    if (allocPtr != NULL)
    	self->nFreeBlocks -= 1;
    K_EXIT_CR;
    return allocPtr;
}

K_ERR kBlockPoolFree(K_BLOCKPOOL* const self, ADDR const blockPtr)
{

	if ((self == NULL) || (blockPtr == NULL))
	{
		return K_ERR_MEM_FREE;
	}
	*(ADDR*)blockPtr = self->freeListPtr;
    self->freeListPtr = blockPtr;
    self->nFreeBlocks += 1U;
    return K_SUCCESS;
}

/*******************************************************************************
 *
 * BYTE POOL CONTROL BLOCK
 *
 ******************************************************************************
 *
 *
 * About BYTE POOLS.
 *
 * It is hard to find a trade-off for byte pools - a random chunk of bytes the
 * application can allocate and deallocate.
 * A  safer version, with meta-data is counter-productive,  given you need
 * a record (a struct) to keep track of every itsy bitsy weenie BYTE.
 * It iss also a bit (or a byte) hard to think of  a situation
 * that cannot be circumvented either by a fixed-size pool or by -
 * the safest choice by far - a static memory allocation.
 *
 * So, it is up to application programmer to diminish the hazards:
 *
 *  o Do Not allocate and deallocate randomly. You got no UNIX here, son.
 *    It's hardcore emBedded <3 <goosebumps>
 *
 *  o TAKE THE OATH BEFORE THIS KERNEL:
 *    - to allocate, use, and free the EXACTLY size for each chunk you
 *      requested. In this order.
 *
 *  o If you write out of the boundaries you have allocated - too sad, too bad
 *    end of story.
 *
 *  o Check for NULL returns. Take a look on the pool. If it is too fragmented
 *    so you won't find a contiguous chunk the size you need, you are
 *    better off reinitialising the pool.
 *
 *  o The IDEAL use:
 *    - allocate and deallocate *multiples* of the pool size.
 *    - do it on unidirectional manner, just as you would do with
 *      synchronisation to avoid a 'deadlock'.
 *
 *
 *******************************************************************************
 *
 * o Inititialisation Depiction:
 *
 *
 * - allocated byte
 * | free byte
 * x sentinel
 *
 *
 * [||||||||||||||||||||||||||||||||||||||x]
 *  ^freelist
 * [------------- poolSize-1--------------)
 *
 ******************************************************************************/
K_ERR kBytePoolInit(K_BYTEPOOL* const self, BYTE* memPool, BYTE const poolSize)
{
    self->memPoolPtr = memPool;
    self->poolSize = poolSize;
    self->nFreeBytes = poolSize;
    self->freeList= 0;

    /* 0->1->2... */
    for (BYTE i = 0; i < poolSize - 1; i++)
    {
        memPool[i] = i + 1;
    }
    /* ... a dummy or a sentinel. it is a matter of soft skills. */
    memPool[poolSize - 1] = 0xFF;
    return K_SUCCESS;
}

/******************************************************************************
 * Alloc Depiction:
 *
 * - allocated byte
 * | free byte
 * x sentinel
 *
 * Pool before allocation::
 * [-----|||||||||||||||||||||||||||||||||x]
 *       ^freelist
 *
 * Size Requested: ||||||
 *
 * Pool after allocation:
 *
 * [----------||||||||||||||||||||||||||||x]
 *       -----^freeList updated
 *       ^
 *       returned address
 *******************************************************************************
 */

ADDR kBytePoolAlloc(K_BYTEPOOL* const self, BYTE size)
{
	if (self->nFreeBytes < size)
	{
		return NULL;
	}

	/*  first free block */
	BYTE startIdx = (self->freeList >> 8) & 0xFF;
	BYTE currIndex = startIdx;

	/* find a chunk */
	for (BYTE i = 1; i < size; i++) 
	{
		if (currIndex == 0xFF || self->memPoolPtr[currIndex] != currIndex + 1) {
			return NULL;  /* oh.. wait */
		}
		currIndex = self->memPoolPtr[currIndex];
	}

	/* the address of the starting block */
	ADDR retAddr = (ADDR)(self->memPoolPtr + startIdx);

	/*  next block after the allocated chunk */
	BYTE nextFreeIndex = self->memPoolPtr[currIndex];
	self->freeList = (nextFreeIndex << 8) | self->memPoolPtr[nextFreeIndex];
	self->nFreeBytes -= size; 

	/*yo got that chunk for yo trunk*/
	return retAddr;
}

/*******************************************************************************
* Free (returns to the pool) a chunk of bytes
*
* Symbols:
*   - : Allocated byte
*   | : Free byte
*   x : Sentinel (end of the pool)
*
* Initial Pool State:
*   [-------||---||||||||||||||||||||||||||x]
*      ^ Application asks to free: ||||
*
* After Freeing:
*   [--||||-||---||||||||||||||||||||||||||x]
*      ^..·.^·...^
*      freeList
*
* After Merging Adjacent Free Blocks:
*   [--||||||----||||||||||||||||||||||||||x]
*      ^....·....^
*      freeList
*******************************************************************************/

K_ERR kBytePoolFree(K_BYTEPOOL* const self, BYTE* const chunkPtr, BYTE const size)
{
	if (chunkPtr == NULL || size == 0)
	{
		return K_ERR_MEM_FREE;
	}
	BYTE chunkIdx = chunkPtr - self->memPoolPtr;
	BYTE nextIdx = chunkIdx + size;
	if (chunkIdx >= self->poolSize)
	{
		return K_ERR_MEM_INVALID_ADDR;
	}
	if (nextIdx < self->poolSize && nextIdx == (self->freeList >> 8))
	{
		self->freeList = (self->freeList & 0x00FF) | (chunkIdx << 8);
	}
	else
	{
		self->memPoolPtr[chunkIdx] = self->freeList >> 8;
		self->freeList = (UINT16)(chunkIdx << 8) | (self->freeList & 0x00FF);
	}
	self->nFreeBytes += size; 

	if (self->nFreeBytes > self->poolSize)
	{
		self->nFreeBytes = self->poolSize;
	}
	return K_SUCCESS;
}

