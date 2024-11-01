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
#include "ksys.h"
#include "kapi.h"

/*******************************************************************************
 *
 * BLOCK POOL CONTROL BLOCK
 *
 ******************************************************************************/
/*let me kick some old simplistic embeddded systems programming sht
 */
K_ERR kBlockPoolInit(K_BLOCKPOOL *const self, const ADDR memPoolPtr,
		BYTE blkSize, const BYTE numBlocks)
{
	K_CR_AREA;
	K_ENTER_CR
	;

	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return K_ERR_MEM_INIT;
	}
#if (K_DEF_MEMBLOCK_ALIGN_4==ON)
	/*round up to next value multiple of 4 (if not a multiple)*/
	blkSize = (blkSize + 0x03) & 0xFC;
#endif
	/* initialise freelist of blocks */

	BYTE *blockPtr = (BYTE*) memPoolPtr; /* first byte address in the pool  */
	ADDR *nextAddrPtr = (ADDR*) memPoolPtr; /* next block address */

	for (BYTE i = 0; i < numBlocks - 1; i++)
	{
		/* init pool by advancing each block addr by blkSize bytes */
		blockPtr += blkSize;
		/* save blockPtr addr as the next */
		*nextAddrPtr = (ADDR) blockPtr;
		/* update  */
		nextAddrPtr = (ADDR*) (blockPtr);

	}
	*nextAddrPtr = NULL;

	/* Initialize the control block */
	self->blkSize = blkSize;
	self->nMaxBlocks = numBlocks;
	self->nFreeBlocks = numBlocks;
	self->freeListPtr = memPoolPtr;
	kMutexInit(&self->poolMutex);
	K_EXIT_CR
	;
	return K_SUCCESS;
}

ADDR kBlockPoolAlloc(K_BLOCKPOOL *const self)
{
	K_CR_AREA;
	K_ENTER_CR
	;
	if (self->nFreeBlocks == 0)
	{
		K_EXIT_CR
		;
		return NULL;
	}
	ADDR allocPtr = self->freeListPtr;
	self->freeListPtr = *(ADDR*) allocPtr;
	if (allocPtr != NULL)
		self->nFreeBlocks -= 1;
	K_EXIT_CR
	;
	return allocPtr;
}

K_ERR kBlockPoolFree(K_BLOCKPOOL *const self, const ADDR blockPtr)
{

	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(self) || IS_NULL_PTR(blockPtr))
	{
		K_EXIT_CR
		;
		return K_ERR_MEM_FREE;
	}
	*(ADDR*) blockPtr = self->freeListPtr;
	self->freeListPtr = blockPtr;
	self->nFreeBlocks += 1;
	K_EXIT_CR
	;
	return K_SUCCESS;
}

/*******************************************************************************
 *
 * BYTE POOL CONTROL BLOCK
 *
 ******************************************************************************
 *
 * About BYTE POOLS.
 *
 * It is hard to find a trade-off for byte pools - a random chunk of bytes the
 * application can allocate and deallocate.
 * A safer version, with meta-data is counter-productive,  given you need
 * a record (a struct) to keep track of every itsy bitsy weenie BYTE.
 * In very constrained applications, it is also a bit hard to deem a situation
 * that cannot be circumvented either by a fixed-size pool or by -
 * the safest choice by far - a static memory allocation.
 *
 * So, it is up to the application programmer to diminish the hazards:
 *
 *  o Do Not allocate and deallocate randomly. You got no UNIX here, son.
 *    It's hardcore emBedded
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
 *    - do it on a unidirectional manner, just as you would do with
 *      synchronisation to avoid a 'deadlock'.
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
K_ERR kBytePoolInit(K_BYTEPOOL *const self, BYTE *memPool, const BYTE poolSize)
{
	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(self) || IS_NULL_PTR(memPool))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return K_ERR_MEM_INIT;
	}
	if (poolSize == 0)
	{
		K_EXIT_CR
		;
		return K_ERR_MEM_INIT;
	}
	self->memPoolPtr = memPool;
	self->poolSize = poolSize;
	self->nFreeBytes = poolSize;
	self->freeList = (0 << 8) | 0; /* this number is also called '0'.
	 wrote like this to be clear about
	 how the indexing works */

	/* link each byte to the next */
	for (BYTE i = 0; i < poolSize - 1; i++)
	{
		memPool[i] = i + 1;
	}
	/* a sentinel. or a soft-skilled dummy */
	memPool[poolSize - 1] = 0xFF;
	K_EXIT_CR
	;
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

ADDR kBytePoolAlloc(K_BYTEPOOL *const self, const BYTE size)
{

	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(self))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return NULL;
	}
	if (self->nFreeBytes < size)
	{
		K_EXIT_CR
		;
		return NULL;
	}
	/* start from the first free block */
	BYTE startIdx = (self->freeList >> 8) & 0xFF;
	BYTE currIdx = startIdx;

	/* grab contiguous chunk */
	for (BYTE i = 1; i < size; i++)
	{
		if (currIdx == 0xFF || self->memPoolPtr[currIdx] != currIdx + 1)
		{
			K_EXIT_CR
			;
			return NULL; /* too fragmented or not contiguous */
		}
		currIdx = self->memPoolPtr[currIdx];
	}
	/* get the address of the starting block */
	ADDR retAddr = (ADDR) (self->memPoolPtr + startIdx);
	/* now freelist points to the next block after the allocated chunk */
	BYTE nextFreeIdx = self->memPoolPtr[currIdx];
	self->freeList = (nextFreeIdx << 8) | self->memPoolPtr[nextFreeIdx];
	self->nFreeBytes -= size;
	K_EXIT_CR
	;
	/* yo got that chunk for yo trunk */
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
 ******************************************************************************/

K_ERR kBytePoolFree(K_BYTEPOOL *const self, BYTE *const chunkPtr,
		const BYTE size)
{
	K_CR_AREA;
	K_ENTER_CR
	;
	if (IS_NULL_PTR(self) || IS_NULL_PTR(chunkPtr))
	{
		kErrHandler(FAULT_NULL_OBJ);
		K_EXIT_CR
		;
		return K_ERR_MEM_INIT;
	}

	BYTE chunkIdx = chunkPtr - self->memPoolPtr;
	BYTE nextIdx = chunkIdx + size;
	if (chunkIdx >= self->poolSize)
	{
		K_EXIT_CR
		;
		return K_ERR_MEM_INVALID_ADDR;
	}
	/*try to merge  */
	if (nextIdx < self->poolSize && nextIdx == (self->freeList >> 8))
	{
		self->freeList = (self->freeList & 0x00FF) | (chunkIdx << 8);
	}
	else
	{
		self->memPoolPtr[chunkIdx] = self->freeList >> 8;
		self->freeList = (UINT16) (chunkIdx << 8) | (self->freeList & 0x00FF);
	}
	self->nFreeBytes += size;

	if (self->nFreeBytes > self->poolSize)
	{
		self->nFreeBytes = self->poolSize;
	}
	K_EXIT_CR
	;
	return K_SUCCESS;
}
