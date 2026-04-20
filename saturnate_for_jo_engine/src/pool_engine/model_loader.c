#include "jo/sgl_prototypes.h"
#include "../core/model_loader.h"
#include "model_loader.h"

POINT   pointPool[MAX_POINT_COUNT]  __attribute__((aligned(4)));
POLYGON polygonPool[MAX_POLYGON_COUNT]  __attribute__((aligned(4)));
ATTR    attributePool[MAX_POLYGON_COUNT]  __attribute__((aligned(4)));
TEXTURE texturePool[MAX_POLYGON_COUNT];
POINT* nextFreePoint = pointPool;
POLYGON* nextFreePolygon = polygonPool;
ATTR* nextFreeAttribute = attributePool;
TEXTURE* nextFreeTexture = texturePool;
Uint32 nextFreeCGBlockIndex = 0;

TEXTURE texturePool[MAX_POLYGON_COUNT];
PDATA meshPool[MAX_MESH_COUNT] __attribute__((aligned(4)));
PDATA* nextFreeMesh;
Uint32 textureCountByModelPool[MAX_MESH_COUNT] __attribute__((aligned(4))); // same 100 than meshPool
Uint32* nextFreeTextureCountByModel;
TTModelSpec modelSpecPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); //same 20 than animationSetSpecPool
TTModelSpec* nextFreeModelSpec;

void ttResetModelPools() {
    nextFreePoint = pointPool;
    nextFreePolygon = polygonPool;
    nextFreeAttribute = attributePool;
    nextFreeMesh = meshPool;
    nextFreeModelSpec = modelSpecPool;
    nextFreeTexture = texturePool;
    nextFreeCGBlockIndex = 0;
    nextFreeTextureCountByModel = textureCountByModelPool;
}

//depends on ttInitFile
//returns the number of meshes in the model
Sint32 ttLoadNYAMeshesInPool(char* fileName) {
    Uint32 textureOffset = 0;
    if(nextFreeTextureCountByModel > textureCountByModelPool)
        for(Uint32* textureCountByModel = nextFreeTextureCountByModel - 1; textureCountByModel >= textureCountByModelPool; textureCountByModel--)
            textureOffset += *textureCountByModel;
    Sint32 meshCount = ttLoadNYAMeshes(fileName, nextFreeMesh, nextFreePoint, nextFreePolygon, nextFreeAttribute,
                                        // (void*) 0, nextFreeTexture, nextFreeCGBlockIndex);
                                         textureOffset, nextFreeTextureCountByModel, nextFreeTexture, nextFreeCGBlockIndex);
    nextFreeModelSpec->meshID = nextFreeMesh - meshPool;
    nextFreeModelSpec->meshCount = meshCount;
    nextFreeMesh += meshCount;
    nextFreeModelSpec++;

    PDATA* lastMesh = nextFreeMesh - 1;
    nextFreePoint = lastMesh->pntbl + lastMesh->nbPoint;
    nextFreePolygon = lastMesh->pltbl + lastMesh->nbPolygon;
    nextFreeAttribute = lastMesh->attbl + lastMesh->nbPolygon;

    nextFreeTexture += *nextFreeTextureCountByModel;
    TEXTURE* lastTexture = nextFreeTexture - 1;
    nextFreeCGBlockIndex += lastTexture->Vsize * lastTexture->Hsize;
    nextFreeTextureCountByModel++;

    return meshCount;
}
