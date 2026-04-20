#ifndef MEMORY_H
#define MEMORY_H

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

#define AREA_CODE_REGISTER (0x20100033)

#define TO_UNCACHED_ADDR(cachedMemoryAddress) (Uint8*) ((Uint32) (cachedMemoryAddress) | 0x20000000) /**< Convert a memory address to its direct acces, non-cached equivalent.*/
#define TO_CACHED_ADDR(uncachedMemoryAddress) (Uint8*) ((Uint32) (uncachedMemoryAddress) & 0x0FFFFFFF) /**< Convert a direct access memory address to its cached equivalent.*/
#define CGADDR_TO_GLOBAL_ADDR(vramBlock) (vramBlock << 3) /**< Convert a VDP1 CGRAM block address to a memory address offset equivalent.*/

/**
 * @brief Copies given amount of bytes from src to dest.
 * 
 * @param[out] dest Pointer to the destination buffer.
 * @param[in] src Pointer to the source buffer.
 * @param[in] byteCount The number of bytes to copy.
 */
void ttCopyBytes(Uint8* dest, Uint8* src, Uint32 byteCount);

/** @} */

#endif //MEMORY_H