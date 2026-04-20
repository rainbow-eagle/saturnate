#include <sega_gfs.h>
#include "file.h"

char gfsWork[GFS_WORK_SIZE(MAX_BACKGROUND_JOBS)] __attribute__((aligned(4)));
GfsDirName gfsDirNames[MAX_FILE_COUNT_PER_FOLDER];
GfsDirTbl gfsDirTbl;

int32_t ttInitFileSystem() {
    gfsDirTbl.type = GFS_DIR_NAME;
    gfsDirTbl.ndir = MAX_FILE_COUNT_PER_FOLDER;
    gfsDirTbl.dir.dir_n = gfsDirNames;
    int32_t result = GFS_Init(MAX_BACKGROUND_JOBS, gfsWork, &gfsDirTbl);
    CDC_CdInit(0, 0, 5, 15);  // Max ECC (5) and retries (15) (optional line)
    
    return result;
}

void ttSetDir(char* dirName) {
    GFS_LoadDir(GFS_NameToId(dirName), &gfsDirTbl);
    GFS_SetDir(&gfsDirTbl);
}

bool ttFileExists(char* fileName) {
    return GFS_NameToId(fileName) >= 0;
}

// int32_t ttLoadFileInPool(char* fileName) {
//     return ttLoadFileInBuffer(fileName, loadedPool);
// }

// #include <sgl_cd.h>
// #include "debug.h"

// int32_t dirWork[SLCD_WORK_SIZE(MAX_FILE_COUNT_PER_FOLDER)];

// int32_t ppInitFileSystem() {
//     int32_t ret = slCdInit(MAX_FILE_COUNT_PER_FOLDER, dirWork);
//     ttDebugIntLn(ret);
// }

// uint8_t loadedPool[22000] __attribute__((aligned(2048)));
// void ttLoadFileInBuffer(char* fileName) {
//     CDKEY cdKeys[2];
//     CDBUF cdBuf[2];

//     cdKeys[0].cn = cdKeys[0].sm = cdKeys[0].ci = CDKEY_NONE;
//     cdKeys[1].cn = CDKEY_TERM;
//     CDHN fileHandle = slCdOpen("ASSETS/AGUY.MOT", cdKeys);

//     cdBuf[0].type = CDBUF_COPY;
//     cdBuf[0].trans.copy.addr = loadedPool;
//     cdBuf[0].trans.copy.unit = CDBUF_FORM1; //means 2048 bytes per sector
//     cdBuf[0].trans.copy.size = 1; //means one instance of the above unit
//     cdBuf[1].type = CDBUF_TERM;

//     slCdLoadFile(fileHandle, cdBuf);


// }