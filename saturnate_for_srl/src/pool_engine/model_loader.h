#ifndef TT_POOL_MODEL_LOADER_H
#define TT_POOL_MODEL_LOADER_H

#include <sl_def.h>
#include "../core/model_loader.h"
#include "animation.h" //because I need the define MAX_ANIMATED_MODEL_COUNT because this code needs better architecture

#define MAX_POINT_COUNT 2000
#define MAX_POLYGON_COUNT 1000
#define MAX_MESH_COUNT 100

typedef struct {
    uint16_t meshID;
    uint8_t meshCount;
    uint8_t forAlignementClarity; //will become something else I guess.
} TTModelSpec;

extern TEXTURE texturePool[MAX_POLYGON_COUNT];
extern PDATA meshPool[MAX_MESH_COUNT] __attribute__((aligned(4)));
extern PDATA* nextFreeMesh;
extern uint32_t textureCountByModelPool[MAX_MESH_COUNT] __attribute__((aligned(4)));
extern uint32_t* nextFreeTextureCountByModel;
extern TTModelSpec modelSpecPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4)));
extern TTModelSpec* nextFreeModelSpec;

void ttResetModelPools();
int32_t ttLoadNYAMeshesInPool(char* fileName);

#endif //TT_POOL_MODEL_LOADER_H