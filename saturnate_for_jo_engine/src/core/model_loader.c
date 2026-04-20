#include "jo/sgl_prototypes.h"
#include "model_loader.h"
#include "file.h"
#include "memory.h"
#include "../pool_engine/debug.h"

#define IMG_SIZE_PX(pixelCount, colorMode) (((pixelCount) * 4) >> colorMode)
#define IMG_SIZE_WH(width, height, colorMode) IMG_SIZE_PX((width) * (height))

typedef struct {
    Uint32 type;
    Uint32 meshCount;
    Uint32 textureCount;
} TTModelFileHeader;

typedef struct {
    Uint32 pointCount;
    Uint32 polygonCount;
} TTModelFileMeshHeader;

typedef struct {
    Uint8 hasTexture      : 1;
    Uint8 hasMeshEffect   : 1;
    Uint8 isDoubleSided   : 1;
    Uint8 hasTransparency : 1;
    Uint8 hasFlatShading   : 1;
    Uint8 hasHalfBrightness: 1;
    Uint8 sortMode        : 2;
    Uint8 isWireframe     : 1;
    Uint8 reserved        : 7;
    Uint16 baseColor;
    Sint32 texture;
} TTModelFileAttribute;

typedef struct {
    Uint16 width;
    Uint16 height;
} TTModelFileTextureHeader;

Sint32 ttLoadNYAMeshes(char* fileName, PDATA* meshes, POINT* points, POLYGON* polygons, ATTR* attributes,
                        Uint32 textureOffset, Uint32* textureCountPtr, TEXTURE* textures, Uint32 cgBlockIndex) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);

    TTModelFileHeader header;
    ttReadBytes(&stream, sizeof(TTModelFileHeader), (Uint8*) &header);
    for(Uint32 i = header.meshCount; i > 0; i--, meshes++) {
        TTModelFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTModelFileMeshHeader), (Uint8*) &meshHeader);
        Uint32 size = sizeof(POINT) * meshHeader.pointCount;
        ttReadBytes(&stream, size, (Uint8*) points);
        size = sizeof(POLYGON) * meshHeader.polygonCount;
        ttReadBytes(&stream, size, (Uint8*) polygons);
        meshes->pntbl = points;
        meshes->nbPoint = meshHeader.pointCount;
        meshes->pltbl = polygons;
        meshes->nbPolygon = meshHeader.polygonCount;
        meshes->attbl = attributes;

        points += meshHeader.pointCount;
        polygons += meshHeader.polygonCount;

        // The file Attribute format doesn't match the ATTR format, gotta convert
        for(Uint32 j = meshHeader.polygonCount; j > 0; j--, attributes++) {
            TTModelFileAttribute currentFileAttribute;
            ttReadBytes(&stream, sizeof(TTModelFileAttribute), (Uint8*) &currentFileAttribute);
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
    Uint32 pixelCount = 0;
    for(Uint32 i = header.textureCount; i > 0; i--, textures++) {
        TTModelFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTModelFileTextureHeader), (Uint8*) &textureHeader);
        *textures = (TEXTURE) TEXDEF(textureHeader.height, textureHeader.width, cgBlockIndex);
        pixelCount = textureHeader.height * textureHeader.width;
        cgBlockIndex += pixelCount;
        const Uint32 imageSize = IMG_SIZE_PX(pixelCount, COL_32K);
        #ifdef TT_DEBUG_MODE
        if(SpriteVRAM + CGADDR_TO_GLOBAL_ADDR(textures->CGadr) + imageSize > 0x25C7FFFF) {
            ttDebugMsgLn("Texture load stopped before overflow");
            break;
        }
        #endif
        ttReadBytes(&stream, imageSize, (Uint8*) SpriteVRAM + CGADDR_TO_GLOBAL_ADDR(textures->CGadr));
    }
    ttCloseFileStream(&stream);
    return header.meshCount;
}
