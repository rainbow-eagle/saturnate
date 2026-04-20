#ifndef TT_POOL_MODEL_LOADER_H
#define TT_POOL_MODEL_LOADER_H

#include "jo/sgl_prototypes.h"
#include "../core/model_loader.h"
#include "animation.h" //because I need the define MAX_ANIMATED_MODEL_COUNT because this code needs better architecture

#define MAX_POINT_COUNT 2000
#define MAX_POLYGON_COUNT 1000
#define MAX_MESH_COUNT 100

typedef struct {
    Uint16 meshID;
    Uint8 meshCount;
    Uint8 forAlignementClarity; //will become something else I guess.
} TTModelSpec;

extern TEXTURE texturePool[MAX_POLYGON_COUNT];
extern PDATA meshPool[MAX_MESH_COUNT] __attribute__((aligned(4)));
extern PDATA* nextFreeMesh;
extern Uint32 textureCountByModelPool[MAX_MESH_COUNT] __attribute__((aligned(4)));
extern Uint32* nextFreeTextureCountByModel;
extern TTModelSpec modelSpecPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4)));
extern TTModelSpec* nextFreeModelSpec;

void ttResetModelPools();
Sint32 ttLoadNYAMeshesInPool(char* fileName);

#endif //TT_POOL_MODEL_LOADER_H
