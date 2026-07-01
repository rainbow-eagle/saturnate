#ifndef TT_CORE_MEMORY_H
#define TT_CORE_MEMORY_H

#include <sl_def.h>

/**
 * @defgroup Memory Memory management
 * @brief Utilities to help manage memory.
 * 
 * ttInitFileSystem() is a necessary initialization step to use the API.
 * ttOpenFileStream(), ttReadBytes() and ttCloseFileStream() is the goto approach to reading files.
 */

/**
 * @addtogroup Memory
 * @{
 */

extern const void end; //not defined in sl_def but it exists in sglarea.c (or workarea.c)

#define TT_AREA_CODE_REGISTER (0x20100033)

#define TT_HRAM_HEAP_START (&end)
#define TT_HRAM_HEAP_END (SortList)

#define TT_LRAM_HEAP_START 0x00200000
#define TT_LRAM_HEAP_END 0x002FFFFF

#define TT_MASTER_CPU_STACK_START (MasterStack)
#define TT_MASTER_CPU_STACK_END AdjWork(CommandBuf, sizeof(uint32_t) * 8, MaxPolygons)

#define TT_SLAVE_CPU_STACK_START (SlaveStack)
#define TT_SLAVE_CPU_STACK_END (0x06000000)

#define TT_VDP1_VRAM (0x25C00000)
#define TT_VDP1_TEXTURE_POOL (TT_VDP1_VRAM + CGADDRESS)

/**
 * @brief Identifies a type of memory.
 */
typedef enum {
    TT_HRAM, /**< High RAM. */
    TT_LRAM /**< Low RAM. */
} TT_RAM_TYPE;

// /**//TBDelected wit hthe next one
//  * @def TT_HIGH_RAM_ATTRIBUTE
//  * @brief Add after the end of the declaration of a global variable to explicitely put it in high RAM.
//  * Actually it's just an empty string because everything goes to high RAM by default.
//  */
#define TT_HIGH_RAM_ATTRIBUTE //(empty)

/**
 * @def TT_LOW_RAM_ATTRIBUTE
 * @brief Add after the end of the declaration of a global variable to explicitely put it in low RAM (doesn't work).
 */
#define TT_LOW_RAM_ATTRIBUTE  __attribute__((section(".low_ram")))

/**
 * @def TT_IS_LRAM
 * @brief Check whether an address is located in low RAM.
 * @return True if the given address is located in low RAM. False otherwise.
 */
#define TT_IS_LRAM(memoryAddress) (((unsigned int)(memoryAddress) >= TT_LRAM_HEAP_START) && ((unsigned int)(memoryAddress) <= TT_LRAM_HEAP_END))

/**
 * @def TT_IS_HRAM
 * @brief Check whether an address is located in high RAM.
 * @return True if the given address is located in high RAM. False otherwise.
 */
#define TT_IS_HRAM(memoryAddress) (((unsigned int)(memoryAddress) >= 0x06000000) && ((unsigned int)(memoryAddress) < 0x06100000))

#define TO_UNCACHED_ADDR(cachedMemoryAddress) (uint8_t*) ((uint32_t) (cachedMemoryAddress) | 0x20000000) /**< Convert a memory address to its direct acces, non-cached equivalent.*/
#define TO_CACHED_ADDR(uncachedMemoryAddress) (uint8_t*) ((uint32_t) (uncachedMemoryAddress) & 0x0FFFFFFF) /**< Convert a direct access memory address to its cached equivalent.*/
#define CG_COUNT_TO_ADDR_OFFSET(vramCGCount) (vramCGCount << 3) /**< Convert a VDP1 CGRAM cell index to a memory address offset equivalent.*/
#define ADDR_OFFSET_TO_CG_COUNT(vramAddrOffset) (vramAddrOffset >> 3)

#define		AdjWork(pt,sz,ct)	(pt + (sz) * (ct)) //directly copied from SGL's workarea.c

/**
 * @def TT_LALLOC
 * @brief Linearly allocates memory from a user-managed pointer with 4-byte alignment.
 * 
 * This macro rounds the current pointer up to the next 32-bit (4-byte) boundary,
 * advances the tracking pointer by the requested size, and returns the starting 
 * address of the allocated space.
 * 
 * @param[in,out] nextFreeByte The pointer variable tracking the current free space. Will be updated.
 * @param[in] size         The number of bytes to allocate.
 * 
 * @return A void pointer to the start of the allocated, 4-byte aligned memory block.
 * 
 * @note To start allocating memory, assign your tracking pointer to free space in memory,
 *       usually, TT_LRAM_HEAP_START or TT_HRAM_HEAP_START.
 * @note To free all allocated memory, simply reassign your tracking pointer back to 
 *       the start of your RAM pool (e.g., `nextFree = (uint8_t*) TT_LRAM_HEAP_START;`).
 * @note The MACRO doesn't check whether the allocated memory lies within bounds of
 *       an available memory space. You might want to check that by verifying that
 *       nextFreeByte is still below TT_HRAM_HEAP_END or TT_LRAM_HEAP_END (or any other limit you have)
 *       after use.
 * 
 * @par Example:
 * @code
 * uint8_t* nextFree = (uint8_t*) TT_LRAM_HEAP_START; // Start of Low RAM
 * 
 * // Allocate 400 bytes for integers (aligned to 4 bytes)
 * uint32_t* myHundredIntegers = TT_LALLOC(nextFree, 100 * sizeof(uint32_t));
 * 
 * // Allocate 16 bytes for text strings
 * char* fileName = TT_LALLOC(nextFree, 16);
 * 
 * // 'nextFree' has now safely advanced past both allocations automatically.
 * @endcode
 */
#define TT_LALLOC(nextFreeByte, size) \
    ({ \
        uint8_t* __aligned = (uint8_t*)(((uint32_t)(nextFreeByte) + 3) & ~3); \
        (nextFreeByte) = __aligned + (size); \
        (void*)__aligned; \
    })


/**
 * @brief Copies given amount of bytes from src to dest.
 * 
 * @param[out] dest Pointer to the destination buffer.
 * @param[in] src Pointer to the source buffer.
 * @param[in] byteCount The number of bytes to copy.
 */
void ttCopyBytes(uint8_t* dest, uint8_t* src, uint32_t byteCount);

/** @} */ //close group Memory

//Technical stuff, library user should ignore this one
#define _TT_LOCAL_ACCUMULATE_(poolName, count, sizeOfType) \
if(TT_IS_HRAM(pools->poolName)) \
    hRAMAccumulator += (count) * (sizeOfType); \
else \
    lRAMAccumulator += (count) * (sizeOfType);

#endif //TT_CORE_MEMORY_H
