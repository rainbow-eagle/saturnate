#ifndef TT_POOL_MODEL_LOADER_H
#define TT_POOL_MODEL_LOADER_H

#include <sl_def.h>
#include "../core/model_loader.h"

typedef struct {
    uint16_t meshID;
    uint8_t meshCount;
    uint8_t forAlignementClarity; //will become something else I guess.
} TTModelSpec;

typedef struct {
    PDATA* meshPool;
    PDATA* nextFreeMesh;
    uint32_t* textureCountByModelPool;
    uint32_t* nextFreeTextureCountByModel;
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
    uint32_t nextFreeTextureID;
    uint32_t nextFreeTextureDataAddr;

    int32_t maxTextureCount;
    int32_t maxPolygonCount;
    int32_t maxPointCount;
    int32_t maxMeshCount;
    int32_t maxModelCount;
} TTModelPools;

typedef enum {
    TT_VRAM_POOL_FULL = TT_VRAM_FULL,
    TT_MESH_POOL_OVERFLOW = TT_MESH_OVERFLOW,
    TT_POINT_POOL_OVERFLOW = TT_POINT_OVERFLOW,
    TT_POLYGON_POOL_OVERFLOW = TT_POLYGON_OVERFLOW,
    TT_TEXTURE_POOL_OVERFLOW = TT_TEXTURE_OVERFLOW,
    TT_MODEL_POOL_OVERFLOW = -5
} TTMeshPoolLoadStatus;


void ttInitModelPools(TTModelPools* pools, uint8_t** nextFreeBytePtr, uint32_t maxModelCount, uint32_t maxMeshCount,
                        uint32_t maxPointCount, uint32_t maxPolygonCount, uint32_t maxTextureCount);
void ttResetModelPools(TTModelPools* pools);
TTMeshPoolLoadStatus ttLoadNYAMeshesInPool(TTModelPools* pools, char* fileName);
void ttGetModelRAMUsage(TTModelPools* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                        uint32_t modelCount, uint32_t meshCount, uint32_t pointCount,
                        uint32_t polygonCount, uint32_t textureCount);
void ttGetCurrentModelRAMUsage(TTModelPools* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage);

static inline __attribute__((always_inline)) void ttGetModelRAMUsageFromMetaData(TTModelPools* pools, uint32_t* hRAMUsage,
                                                                                uint32_t* lRAMUsage, TTNYAFileMetadata* metaData) {
    ttGetModelRAMUsage(pools, hRAMUsage, lRAMUsage, 1, metaData->meshCount,
                        metaData->pointCount, metaData->polygonCount, metaData->textureCount);
}

static inline __attribute__((always_inline)) void ttGetReservedModelRAMUsage(TTModelPools* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    ttGetModelRAMUsage(pools, hRAMUsage, lRAMUsage, pools->maxModelCount, pools->maxMeshCount,
                        pools->maxPointCount, pools->maxPolygonCount, pools->maxTextureCount);
}

#define TT_ALLOC_MODEL_POOLS(variableName, maxModelCnt, maxMeshCnt, maxPointCnt, maxPolygonCnt, maxTextureCnt, modelAttribute) \
    POINT variableName##_pointPool[maxPointCnt] __attribute__((aligned(4))) modelAttribute; \
    POLYGON variableName##_polygonPool[maxPolygonCnt] __attribute__((aligned(4))) modelAttribute; \
    ATTR variableName##_attributePool[maxPolygonCnt] __attribute__((aligned(4))) modelAttribute; \
    TEXTURE variableName##_texturePool[maxTextureCnt] __attribute__((aligned(4))) modelAttribute; \
    PDATA variableName##_meshPool[maxMeshCnt] __attribute__((aligned(4))) modelAttribute; \
    uint32_t variableName##_textureCountByModelPool[maxModelCnt] __attribute__((aligned(4))) modelAttribute; \
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