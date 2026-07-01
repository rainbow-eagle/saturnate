#ifndef TT_POOL_MODEL_LOADER_H
#define TT_POOL_MODEL_LOADER_H

#include <jo/sgl_prototypes.h>
#include "../core/model_loader.h"

typedef struct {
    Uint16 meshID;
    Uint8 meshCount;
    Uint8 forAlignementClarity; //will become something else I guess.
} TTModelSpec;

typedef struct {
    PDATA* meshPool;
    PDATA* nextFreeMesh;
    Uint32* textureCountByModelPool;
    Uint32* nextFreeTextureCountByModel;
    TTModelSpec* modelSpecPool;
    TTModelSpec* nextFreeModelSpec;

    POINT* pointPool;
    POINT* nextFreePoint;
    POLYGON* polygonPool;
    POLYGON* nextFreePolygon;
    ATTR* attributePool;
    ATTR* nextFreeAttribute;
    TEXTURE* texturePool;
    TEXTURE* nextFreeTexture;
    Uint32 nextFreeTextureID;
    Uint32 nextFreeTextureDataAddr;

    Sint32 maxTextureCount;
    Sint32 maxPolygonCount;
    Sint32 maxPointCount;
    Sint32 maxMeshCount;
    Sint32 maxModelCount;
} TTModelPools;

typedef enum {
    TT_VRAM_POOL_FULL = TT_VRAM_FULL,
    TT_MESH_POOL_OVERFLOW = TT_MESH_OVERFLOW,
    TT_POINT_POOL_OVERFLOW = TT_POINT_OVERFLOW,
    TT_POLYGON_POOL_OVERFLOW = TT_POLYGON_OVERFLOW,
    TT_TEXTURE_POOL_OVERFLOW = TT_TEXTURE_OVERFLOW,
    TT_MODEL_POOL_OVERFLOW = -5
} TTMeshPoolLoadStatus;


void ttInitModelPools(TTModelPools* pools, Uint8** nextFreeBytePtr, Uint32 maxModelCount, Uint32 maxMeshCount,
                        Uint32 maxPointCount, Uint32 maxPolygonCount, Uint32 maxTextureCount);
void ttResetModelPools(TTModelPools* pools);
TTMeshPoolLoadStatus ttLoadNYAMeshesInPool(TTModelPools* pools, char* fileName);
void ttGetModelRAMUsage(TTModelPools* pools, Uint32* hRAMUsage, Uint32* lRAMUsage,
                        Uint32 modelCount, Uint32 meshCount, Uint32 pointCount,
                        Uint32 polygonCount, Uint32 textureCount);
void ttGetCurrentModelRAMUsage(TTModelPools* pools, Uint32* hRAMUsage, Uint32* lRAMUsage);

static inline __attribute__((always_inline)) void ttGetModelRAMUsageFromMetaData(TTModelPools* pools, Uint32* hRAMUsage,
                                                                                Uint32* lRAMUsage, TTNYAFileMetadata* metaData) {
    ttGetModelRAMUsage(pools, hRAMUsage, lRAMUsage, 1, metaData->meshCount,
                        metaData->pointCount, metaData->polygonCount, metaData->textureCount);
}

static inline __attribute__((always_inline)) void ttGetReservedModelRAMUsage(TTModelPools* pools, Uint32* hRAMUsage, Uint32* lRAMUsage) {
    ttGetModelRAMUsage(pools, hRAMUsage, lRAMUsage, pools->maxModelCount, pools->maxMeshCount,
                        pools->maxPointCount, pools->maxPolygonCount, pools->maxTextureCount);
}

#define TT_ALLOC_MODEL_POOLS(variableName, maxModelCnt, maxMeshCnt, maxPointCnt, maxPolygonCnt, maxTextureCnt, modelAttribute) \
    POINT variableName##_pointPool[maxPointCnt] __attribute__((aligned(4))) modelAttribute; \
    POLYGON variableName##_polygonPool[maxPolygonCnt] __attribute__((aligned(4))) modelAttribute; \
    ATTR variableName##_attributePool[maxPolygonCnt] __attribute__((aligned(4))) modelAttribute; \
    TEXTURE variableName##_texturePool[maxTextureCnt] __attribute__((aligned(4))) modelAttribute; \
    PDATA variableName##_meshPool[maxMeshCnt] __attribute__((aligned(4))) modelAttribute; \
    Uint32 variableName##_textureCountByModelPool[maxModelCnt] __attribute__((aligned(4))) modelAttribute; \
    TTModelSpec variableName##_modelSpecPool[maxModelCnt] __attribute__((aligned(4))) modelAttribute; \
    TTModelPools variableName = { \
        .meshPool = variableName##_meshPool, \
        .nextFreeMesh = variableName##_meshPool, \
        .textureCountByModelPool = variableName##_textureCountByModelPool, \
        .nextFreeTextureCountByModel = variableName##_textureCountByModelPool, \
        .modelSpecPool = variableName##_modelSpecPool, \
        .nextFreeModelSpec = variableName##_modelSpecPool, \
        .pointPool = variableName##_pointPool, \
        .nextFreePoint = variableName##_pointPool, \
        .polygonPool = variableName##_polygonPool, \
        .nextFreePolygon = variableName##_polygonPool, \
        .attributePool = variableName##_attributePool, \
        .nextFreeAttribute = variableName##_attributePool, \
        .texturePool = variableName##_texturePool, \
        .nextFreeTexture = variableName##_texturePool, \
        .nextFreeTextureID = 0, \
        .nextFreeTextureDataAddr = TT_VDP1_TEXTURE_POOL, \
        .maxTextureCount = maxTextureCnt, \
        .maxPolygonCount = maxPolygonCnt, \
        .maxPointCount = maxPointCnt, \
        .maxMeshCount = maxMeshCnt, \
        .maxModelCount = maxModelCnt \
    };

#endif //TT_POOL_MODEL_LOADER_H