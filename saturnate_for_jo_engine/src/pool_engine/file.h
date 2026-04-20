#ifndef TT_POOL_FILE_H
#define TT_POOL_FILE_H

#include "../core/file.h"

#define MAX_FILE_COUNT_PER_FOLDER 32
#define MAX_BACKGROUND_JOBS 1
extern GfsDirName gfsDirNames[MAX_FILE_COUNT_PER_FOLDER];
extern GfsDirTbl gfsDirTbl;

/**
 * @brief Initializes the File System for CD access for name-based file access and maximum retries.
 *
 * It must be called once before any file operations can be performed.
 * Everything is hardcoded for now but a different version with parameters could be implemented in the future.
 *
 * @return Sint32 The result code from GFS_Init. 0 indicates success, non-zero indicates failure.
 * @retval 0    Success.
 * @retval != 0 Initialization failed. Check GFS library documentation for specific error codes.
 *
 * @see GFS_Init()
 */
extern Sint32 ttInitFileSystem();

/**
 * @brief Changes the current directory on the CD file hierarchy.
 * 
 * @param[in] dirName Name of the directory relatively to the current directory.
 * @see ttInitFileSystem()
 */
void ttSetDir(char* dirName);


Bool ttFileExists(char* fileName);

// extern Uint8 loadedPool[22000] __attribute__((aligned(2048)));
// Sint32 ttLoadFileInPool(char* fileName);

#endif //TT_POOL_FILE_H