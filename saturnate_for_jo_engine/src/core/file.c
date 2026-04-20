#include "jo/sgl_prototypes.h"
#include "file.h"
#include "memory.h"
#include "../pool_engine/debug.h"

void ttOpenFileStream(char* fileName, TTFileStream* stream) {
    stream->tmpBufferedByteCount = 0;
    Sint32 fileID = GFS_NameToId(fileName);
    stream->fileHandle = GFS_Open(fileID);

    //Delete that shit
    // Sint32 sctSize, numSct, lastSize;
    // GFS_GetFileSize(stream->fileHandle, &sctSize, &numSct, &lastSize);
    // ttDebugMsgLn("Sector size :");
    // ttDebugIntLn(sctSize);
}

void ttCloseFileStream(TTFileStream* stream) {
    GFS_Close(stream->fileHandle);
}

void ttReadBytes(TTFileStream* stream, Uint32 byteCount, Uint8* buffer) {
    Uint8* bufferPtr = buffer;
    if(stream->tmpBufferedByteCount) { // if we still have pre-read stuff
        Uint8* tmpBufferPtr = stream->tmpBuffer +(TT_SECTOR_SIZE - stream->tmpBufferedByteCount); //the pre-read stuff occupies the end of the buffer
        if(byteCount > stream->tmpBufferedByteCount) {
            ttCopyBytes(bufferPtr, tmpBufferPtr, stream->tmpBufferedByteCount);
            bufferPtr += stream->tmpBufferedByteCount;
            byteCount -= stream->tmpBufferedByteCount;
        }
        else {
            ttCopyBytes(bufferPtr, tmpBufferPtr, byteCount); // Copy only the start of the preread stuff to the destination
            stream->tmpBufferedByteCount -= byteCount;
            tmpBufferPtr+=byteCount;
            return;
        }
    }

    Sint32 startSectorCount = ((TT_SECTOR_SIZE == 2048)? byteCount >> 11 : byteCount / TT_SECTOR_SIZE);
    if(startSectorCount > 0) {// Read the first sectors directly to the final destination...
        GFS_Fread(stream->fileHandle, startSectorCount, bufferPtr, byteCount);
        bufferPtr += TT_SECTOR_SIZE * startSectorCount;
    }
    //... and read the last sector in the buffer, to then copy only the requested part to the final destination...
    GFS_Fread(stream->fileHandle, 1, stream->tmpBuffer, TT_SECTOR_SIZE);
    Uint32 lastSectorByteCount = byteCount % TT_SECTOR_SIZE;
    stream->tmpBufferedByteCount = TT_SECTOR_SIZE - lastSectorByteCount;
    ttCopyBytes(bufferPtr, stream->tmpBuffer, lastSectorByteCount);

    return;
}

//Doesn't need to close anything after that
void ttLoadFileInBuffer(char* fileName, Uint8* buffer) {
    GFS_Load(GFS_NameToId(fileName), 0, buffer, GFS_BUFSIZ_INF);
}
