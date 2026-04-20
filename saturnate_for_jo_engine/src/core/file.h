#ifndef TT_CORE_FILE_H
#define TT_CORE_FILE_H
#include "jo/sgl_prototypes.h"

/**
 * @defgroup FileSystem File
 * @brief Functions to read files.
 * 
 * GFS_Init() is a necessary initialization step to use the API.
 * ttOpenFileStream(), ttReadBytes() and ttCloseFileStream() is the goto approach to reading files.
 */

/**
 * @addtogroup FileSystem
 * @{
 */

#ifndef TT_SECTOR_SIZE
#define TT_SECTOR_SIZE 2048  /**< Number of usable bytes present in each sector - either 2048 or 2324. Can be set as a compilation parameter.*/
#endif //TT_SECTOR_SIZE

 /**
 * @brief Structure representing a filestream.
 *
 * Contains the inner state of the stream. Is to be allocated by the user
 * and initialized with ttOpenFileStream().
 * Contains the data from the lastly read CD sector as to implement
 * an abstraction layer enabling effective streaming of the file.
 * 
 * @warning Do not edit manually.
 */
typedef struct {
    Sint32 tmpBufferedByteCount; /**Number of bytes currently buffered (sector size - number of already read bytes)*/
    GfsHn fileHandle; /**GFS file handle to the opened file*/
    Uint8 tmpBuffer[TT_SECTOR_SIZE]; /**Buffer where the lastly read sector is stored*/
} TTFileStream;

/**
 * @brief Opens a stream to read a file.
 * An opened stream must be closed with ttCloseFileStream().
 * 
 * @param[in] fileName The name of the file in the current directory.
 * @param[out] stream Pointer to a user-allocated stream.
 * @see GFS_Init(), ttCloseFileStream(), ttReadBytes()
 */
void ttOpenFileStream(char* fileName, TTFileStream* stream);

/**
 * @brief Closes an opened filestream.
 * 
 * @param stream[in] The stream to be closed.
 * @see GFS_Init(), ttOpenFileStream()
 */
void ttCloseFileStream(TTFileStream* stream);

/**
 * @brief Reads bytes from an opened stream to a file.
 * The stream must be opened with ttOpenFileStream.
 * After usage, it must be closed with ttCloseFileStream().
 * 
 * @param[in,out] stream The opened stream from which bytes are read.
 * @param[in] byteCount The amound of bytes to read.
 * @param[out] buffer The user-allocated buffer where bytes are written.
 * @see GFS_Init(), ttOpenFileStream(), ttCloseFileStream()
 */
void ttReadBytes(TTFileStream* stream, Uint32 byteCount, Uint8* buffer);

/**
 * @brief Load the entire content of a file into a given buffer.
 * Does not require the user to open nor close the file.
 * 
 * @param[in] fileName The file to be loaded.
 * @param[out] buffer The user-allocated buffer where the content of the file is loaded.
 */
void ttLoadFileInBuffer(char* fileName, Uint8* buffer);

/** @} */

#endif // TT_CORE_FILE_H
