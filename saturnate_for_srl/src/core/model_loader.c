#include <sgl.h>
#include "model_loader.h"
#include "file.h"
#include "memory.h"
#include "../pool_engine/debug.h"

#define IMG_SIZE_PX(pixelCount, colorMode) (((pixelCount) * 4) >> colorMode)
#define IMG_SIZE_WH(width, height, colorMode) IMG_SIZE_PX((width) * (height))

typedef struct {
    uint32_t type;
    uint32_t meshCount;
    uint32_t textureCount;
} TTModelFileHeader;

typedef struct {
    uint32_t pointCount;
    uint32_t polygonCount;
} TTModelFileMeshHeader;

typedef struct {
    uint8_t hasTexture      : 1;
    uint8_t hasMeshEffect   : 1;
    uint8_t isDoubleSided   : 1;
    uint8_t hasTransparency : 1;
    uint8_t hasFlatShading   : 1;
    uint8_t hasHalfBrightness: 1;
    uint8_t sortMode        : 2;
    uint8_t isWireframe     : 1;
    uint8_t reserved        : 7;
    uint16_t baseColor;
    int32_t texture;
} TTModelFileAttribute;

typedef struct {
    uint16_t width;
    uint16_t height;
} TTModelFileTextureHeader;

int32_t ttLoadNYAMeshes(char* fileName, PDATA* meshes, POINT* points, POLYGON* polygons, ATTR* attributes,
                        uint32_t textureOffset, uint32_t* textureCountPtr, TEXTURE* textures, uint32_t cgBlockIndex) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);

    TTModelFileHeader header;
    ttReadBytes(&stream, sizeof(TTModelFileHeader), (uint8_t*) &header);
    for(uint32_t i = header.meshCount; i > 0; i--, meshes++) {
        TTModelFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTModelFileMeshHeader), (uint8_t*) &meshHeader);
        uint32_t size = sizeof(POINT) * meshHeader.pointCount;
        ttReadBytes(&stream, size, (uint8_t*) points);
        size = sizeof(POLYGON) * meshHeader.polygonCount;
        ttReadBytes(&stream, size, (uint8_t*) polygons);
        meshes->pntbl = points;
        meshes->nbPoint = meshHeader.pointCount;
        meshes->pltbl = polygons;
        meshes->nbPolygon = meshHeader.polygonCount;
        meshes->attbl = attributes;

        points += meshHeader.pointCount;
        polygons += meshHeader.polygonCount;

        // The file Attribute format doesn't match the ATTR format, gotta convert
        for(uint32_t j = meshHeader.polygonCount; j > 0; j--, attributes++) {
            TTModelFileAttribute currentFileAttribute;
            ttReadBytes(&stream, sizeof(TTModelFileAttribute), (uint8_t*) &currentFileAttribute);
            *attributes = (ATTR) ATTRIBUTE(
                currentFileAttribute.isDoubleSided? Dual_Plane : Single_Plane,
                SORT_CEN - currentFileAttribute.sortMode,
                textureOffset + currentFileAttribute.texture, //should be texture + index of the first texture belonging to this model
                currentFileAttribute.hasTexture? No_Palet : currentFileAttribute.baseColor,
                No_Gouraud, //gouraud support to be added
                CL32KRGB | ECdis |
                    (currentFileAttribute.hasMeshEffect != 0 ? MESHon : MESHoff) |
                    (currentFileAttribute.hasFlatShading != 0 ? 0 : CL_Gouraud) |
                    (currentFileAttribute.hasTransparency != 0 ? CL_Trans : 0) |
                    (currentFileAttribute.hasHalfBrightness != 0 ? CL_Half : 0),
                currentFileAttribute.isWireframe != 0 ?
                    sprPolyLine : (currentFileAttribute.hasTexture != 0 ? sprNoflip : sprPolygon),
                (currentFileAttribute.hasFlatShading != 0 ? UseLight : UseGouraud));
        }
    }

    //It'd be better if all texture headers were stored together in a block, and then all textures next to one another in another block.
    //That way we would copy everything at once and be much faster. In case you want to define your own file format..
    //Well actually the same can be said about all the POINTS and all the POLYGONS.
    *textureCountPtr = header.textureCount;
    uint32_t pixelCount = 0;
    for(uint32_t i = header.textureCount; i > 0; i--, textures++) {
        TTModelFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTModelFileTextureHeader), (uint8_t*) &textureHeader);
        *textures = (TEXTURE) TEXDEF(textureHeader.height, textureHeader.width, cgBlockIndex);
        pixelCount = textureHeader.height * textureHeader.width;
        cgBlockIndex += pixelCount;
        const uint32_t imageSize = IMG_SIZE_PX(pixelCount, COL_32K);
        #ifdef TT_DEBUG_MODE
        if(SpriteVRAM + CGADDR_TO_GLOBAL_ADDR(textures->CGadr) + imageSize > 0x25C7FFFF) {
            ttDebugMsgLn("Texture load stopped before overflow");
            break;
        }
        #endif
        ttReadBytes(&stream, imageSize, (uint8_t*) SpriteVRAM + CGADDR_TO_GLOBAL_ADDR(textures->CGadr));
    }
    ttCloseFileStream(&stream);
    return header.meshCount;
}