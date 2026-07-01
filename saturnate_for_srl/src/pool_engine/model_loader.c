#include <sl_def.h>
#include "../core/model_loader.h"
#include "../core/memory.h"
#include "model_loader.h"
#include "../debug/print.h"

void ttInitModelPools(TTModelPools* pools, uint8_t** nextFreeBytePtr, uint32_t maxModelCount, uint32_t maxMeshCount,
                        uint32_t maxPointCount, uint32_t maxPolygonCount, uint32_t maxTextureCount) {
    pools->meshPool = TT_LALLOC(*nextFreeBytePtr, maxMeshCount * sizeof(PDATA));
    pools->textureCountByModelPool = TT_LALLOC(*nextFreeBytePtr, maxModelCount * sizeof(uint32_t));
    pools->modelSpecPool = TT_LALLOC(*nextFreeBytePtr, maxModelCount * sizeof(TTModelSpec));
    pools->pointPool = TT_LALLOC(*nextFreeBytePtr, maxPointCount * sizeof(POINT));
    pools->polygonPool = TT_LALLOC(*nextFreeBytePtr, maxPolygonCount * sizeof(POLYGON));
    pools->attributePool = TT_LALLOC(*nextFreeBytePtr, maxPolygonCount * sizeof(ATTR));
    pools->texturePool = TT_LALLOC(*nextFreeBytePtr, maxTextureCount * sizeof(TEXTURE));

    pools->maxTextureCount = maxTextureCount;
    pools->maxPolygonCount = maxPolygonCount;
    pools->maxPointCount = maxPointCount;
    pools->maxMeshCount = maxMeshCount;
    pools->maxModelCount = maxModelCount;

    ttResetModelPools(pools);
}

void ttResetModelPools(TTModelPools* pools) {
    pools->nextFreePoint = pools->pointPool;
    pools->nextFreePolygon = pools->polygonPool;
    pools->nextFreeAttribute = pools->attributePool;
    pools->nextFreeMesh = pools->meshPool;
    pools->nextFreeModelSpec = pools->modelSpecPool;
    pools->nextFreeTexture = pools->texturePool;
    pools->nextFreeTextureID = 0;
    pools->nextFreeTextureDataAddr = TT_VDP1_TEXTURE_POOL;
    pools->nextFreeTextureCountByModel = pools->textureCountByModelPool;
}

//depends on ttInitFile
//return code see core/loadNyaMeshes
TTMeshPoolLoadStatus ttLoadNYAMeshesInPool(TTModelPools* pools, char* fileName) {
    #ifdef TT_DEBUG_MODE
        uint32_t modelCount = pools->nextFreeModelSpec - pools->modelSpecPool;
        if(__builtin_expect(modelCount >= pools->maxModelCount, 0))
            return TT_MODEL_POOL_OVERFLOW;
    #endif

    // uint32_t textureOffset = 0;
    // if(pools->nextFreeTextureCountByModel > pools->textureCountByModelPool)
    //     for(uint32_t* textureCountByModel = pools->nextFreeTextureCountByModel - 1;
    //         textureCountByModel >= pools->textureCountByModelPool; textureCountByModel--)
    //         textureOffset += *textureCountByModel;

    TTMeshBuffer meshBuffer = {
        &(pools->nextFreeModelSpec->meshCount),
        pools->nextFreeMesh, pools->maxMeshCount - (pools->nextFreeMesh - pools->meshPool),
        pools->nextFreePoint, pools->maxPointCount - (pools->nextFreePoint - pools->pointPool),
        pools->nextFreePolygon, pools->maxPolygonCount - (pools->nextFreePolygon - pools->polygonPool),
        pools->nextFreeAttribute,
        pools->nextFreeTexture, pools->maxTextureCount - (pools->nextFreeTexture - pools->texturePool),
        pools->nextFreeTextureID, pools->nextFreeTextureCountByModel, &(pools->nextFreeTextureDataAddr)
    };
    TTMeshPoolLoadStatus returnCode = ttLoadNYAMeshes(fileName, &meshBuffer);//the return only works because I manually mapped the error code from core to pool
    if(__builtin_expect(returnCode == OK || returnCode == TT_VRAM_FULL, 1)) {
        pools->nextFreeModelSpec->meshID = pools->nextFreeMesh - pools->meshPool;
        pools->nextFreeMesh += pools->nextFreeModelSpec->meshCount;
        pools->nextFreeModelSpec++;

        PDATA* lastMesh = pools->nextFreeMesh - 1;
        pools->nextFreePoint = lastMesh->pntbl + lastMesh->nbPoint;
        pools->nextFreePolygon = lastMesh->pltbl + lastMesh->nbPolygon;
        pools->nextFreeAttribute = lastMesh->attbl + lastMesh->nbPolygon;

        pools->nextFreeTexture += *(pools->nextFreeTextureCountByModel);
        pools->nextFreeTextureID += *(pools->nextFreeTextureCountByModel);
        pools->nextFreeTextureCountByModel++;
    }

    return returnCode;
}

void ttGetModelRAMUsage(TTModelPools* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                        uint32_t modelCount, uint32_t meshCount, uint32_t pointCount,
                        uint32_t polygonCount, uint32_t textureCount) {
    uint32_t hRAMAccumulator = 0;
    uint32_t lRAMAccumulator = 0;

    _TT_LOCAL_ACCUMULATE_(modelSpecPool, modelCount, sizeof(TTModelSpec))
    _TT_LOCAL_ACCUMULATE_(textureCountByModelPool, modelCount, sizeof(uint32_t))
    _TT_LOCAL_ACCUMULATE_(meshPool, meshCount, sizeof(PDATA))

    _TT_LOCAL_ACCUMULATE_(pointPool, pointCount, sizeof(POINT))
    _TT_LOCAL_ACCUMULATE_(polygonPool, polygonCount, sizeof(POLYGON))
    _TT_LOCAL_ACCUMULATE_(attributePool, polygonCount, sizeof(ATTR))
    _TT_LOCAL_ACCUMULATE_(texturePool, textureCount, sizeof(TEXTURE))

    *hRAMUsage = hRAMAccumulator;
    *lRAMUsage = lRAMAccumulator;
}

void ttGetCurrentModelRAMUsage(TTModelPools* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    const uint32_t modelCount = pools->nextFreeModelSpec - pools->modelSpecPool;
    const uint32_t meshCount = pools->nextFreeMesh - pools->meshPool;
    const uint32_t pointCount = pools->nextFreePoint - pools->pointPool;
    const uint32_t polygonCount = pools->nextFreePolygon - pools->polygonPool;
    const uint32_t textureCount = pools->nextFreeTexture - pools->texturePool;
    ttGetModelRAMUsage(pools, hRAMUsage, lRAMUsage, modelCount, meshCount, pointCount, polygonCount, textureCount);
}
