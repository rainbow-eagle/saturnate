#include "jo/sgl_prototypes.h"
#include "model_loader.h"
#include "file.h"
#include "memory.h"
#include "../debug/print.h"

#define IMG_SIZE_PX(pixelCount, colorMode) (((pixelCount) * 4) >> colorMode)
#define IMG_SIZE_WH(width, height, colorMode) IMG_SIZE_PX((width) * (height), colorMode)

typedef struct {
    Uint32 type;
    Uint32 meshCount;
    Uint32 textureCount;
} TTNYAFileHeader;

typedef struct {
    Uint32 pointCount;
    Uint32 polygonCount;
} TTNYAFileMeshHeader;

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
} TTNYAFileAttribute;

typedef struct {
    Uint16 width;
    Uint16 height;
} TTNYAFileTextureHeader;

void ttLoadNYAMetadata(char* fileName, TTNYAFileMetadata* metadata) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);

    Uint32 pointCount = 0;
    Uint32 polygonCount = 0;
    TTNYAFileHeader header;
    ttReadBytes(&stream, sizeof(TTNYAFileHeader), (Uint8*) &header);
    for(Uint32 i = header.meshCount; i > 0; i--) {
        TTNYAFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (Uint8*) &meshHeader);
        pointCount += meshHeader.pointCount;
        Uint32 size = sizeof(POINT) * meshHeader.pointCount;
        ttSkipBytes(&stream, size);
        polygonCount += meshHeader.polygonCount;
        size = sizeof(POLYGON) * meshHeader.polygonCount + sizeof(TTNYAFileAttribute) * meshHeader.polygonCount;
        ttSkipBytes(&stream, size);
    }
    metadata->pointCount = pointCount;
    metadata->polygonCount = polygonCount;
    metadata->meshCount = header.meshCount;

    metadata->textureCount = header.textureCount;
    Uint32 textureDataAddr = 0;
    for(Uint32 i = header.textureCount; i > 0; i--) {
        TTNYAFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (Uint8*) &textureHeader);
        const Uint32 imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);
        ttSkipBytes(&stream, imageSize);
        textureDataAddr = AdjCG(textureDataAddr, textureHeader.width, textureHeader.height, COL_32K);
    }
    metadata->vramCGCount = ADDR_OFFSET_TO_CG_COUNT(textureDataAddr);

    ttCloseFileStream(&stream);
}

TTMeshLoadStatus ttLoadNYAMeshes(char* fileName, TTMeshBuffer* meshBuffer) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);

    TTNYAFileHeader header;
    ttReadBytes(&stream, sizeof(TTNYAFileHeader), (Uint8*) &header);
    #ifdef TT_DEBUG_MODE
        if(__builtin_expect(header.meshCount > meshBuffer->maxMeshCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_MESH_OVERFLOW;
        }
        if(__builtin_expect(header.textureCount > meshBuffer->maxTextureCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_TEXTURE_OVERFLOW;
        }
    #endif
    *(meshBuffer->meshCountPtr) = header.meshCount;
    PDATA* meshes = meshBuffer->meshes;
    POINT* points = meshBuffer->points;
    POLYGON* polygons = meshBuffer->polygons;
    ATTR* attributes = meshBuffer->attributes;
    #ifdef TT_DEBUG_MODE
        Uint32 totalPointCount = 0;
        Uint32 totalPolygonCount = 0;
    #endif
    for(Uint32 i = header.meshCount; i > 0; i--, meshes++) {
        TTNYAFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (Uint8*) &meshHeader);
        #ifdef TT_DEBUG_MODE
            totalPointCount += meshHeader.pointCount;
            if(__builtin_expect(totalPointCount > meshBuffer->maxPointCount, 0)) {
                ttCloseFileStream(&stream);
                return TT_POINT_OVERFLOW;
            }
            totalPolygonCount += meshHeader.polygonCount;
            if(__builtin_expect(totalPolygonCount > meshBuffer->maxPolygonCount, 0)) {
                ttCloseFileStream(&stream);
                return TT_POLYGON_OVERFLOW;
            }
        #endif
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
            TTNYAFileAttribute currentFileAttribute;
            ttReadBytes(&stream, sizeof(TTNYAFileAttribute), (Uint8*) &currentFileAttribute);
            *attributes = (ATTR) ATTRIBUTE(
                currentFileAttribute.isDoubleSided? Dual_Plane : Single_Plane,
                SORT_CEN - currentFileAttribute.sortMode,
                meshBuffer->textureOffset + currentFileAttribute.texture, //should be texture + index of the first texture belonging to this model
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
    *(meshBuffer->textureCountPtr) = header.textureCount;
    TEXTURE* textures = meshBuffer->textures;
    Uint16 pixelCount = 0;
    for(Uint32 i = header.textureCount; i > 0; i--, textures++) {
        TTNYAFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (Uint8*) &textureHeader);
        *textures = (TEXTURE) TEXTBL(textureHeader.width, textureHeader.height, *meshBuffer->textureDataAddrPtr - TT_VDP1_VRAM);
        const Uint32 imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);

        #ifdef TT_DEBUG_MODE
            if(__builtin_expect(CG_COUNT_TO_ADDR_OFFSET(textures->CGadr) + imageSize > GouraudRAM, 0)) {
                //The actual texture count is less than previously thought
                *(meshBuffer->textureCountPtr) = header.textureCount - i;
                //Remove all references to textures that couldn't be loaded
                Uint16 lastValidTextureIndex = header.textureCount - i - 1;
                meshes--;
                for(Uint32 i = header.meshCount; i > 0; i--, meshes--) {
                    attributes--;
                    for(Uint32 j = meshes->nbPolygon; j > 0; j--, attributes--)
                        if(attributes->texno > lastValidTextureIndex) {
                            attributes->texno = No_Texture;
                        }
                }
                ttCloseFileStream(&stream);
                return TT_VRAM_FULL;
            }
        #endif

        ttReadBytes(&stream, imageSize, (Uint8*) TT_VDP1_VRAM + CG_COUNT_TO_ADDR_OFFSET(textures->CGadr));
        *meshBuffer->textureDataAddrPtr = AdjCG(*meshBuffer->textureDataAddrPtr, textureHeader.width, textureHeader.height, COL_32K);
    }
    ttCloseFileStream(&stream);

    return OK;
}

// Sint32 ttLoadNYAMeshes1(char* fileName, PDATA* meshes, POINT* points, POLYGON* polygons, ATTR* attributes,
//                         Uint32 textureOffset, Uint32* textureCountPtr, TEXTURE* textures, Uint32* textureDataAddrPtr) {
//     TTFileStream stream;
//     ttOpenFileStream(fileName, &stream);

//     TTNYAFileHeader header;
//     ttReadBytes(&stream, sizeof(TTNYAFileHeader), (Uint8*) &header);
//     for(Uint32 i = header.meshCount; i > 0; i--, meshes++) {
//         TTNYAFileMeshHeader meshHeader;
//         ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (Uint8*) &meshHeader);
//         Uint32 size = sizeof(POINT) * meshHeader.pointCount;
//         ttReadBytes(&stream, size, (Uint8*) points);
//         size = sizeof(POLYGON) * meshHeader.polygonCount;
//         ttReadBytes(&stream, size, (Uint8*) polygons);
//         meshes->pntbl = points;
//         meshes->nbPoint = meshHeader.pointCount;
//         meshes->pltbl = polygons;
//         meshes->nbPolygon = meshHeader.polygonCount;
//         meshes->attbl = attributes;

//         points += meshHeader.pointCount;
//         polygons += meshHeader.polygonCount;

//         // The file Attribute format doesn't match the ATTR format, gotta convert
//         for(Uint32 j = meshHeader.polygonCount; j > 0; j--, attributes++) {
//             TTNYAFileAttribute currentFileAttribute;
//             ttReadBytes(&stream, sizeof(TTNYAFileAttribute), (Uint8*) &currentFileAttribute);
//             *attributes = (ATTR) ATTRIBUTE(
//                 currentFileAttribute.isDoubleSided? Dual_Plane : Single_Plane,
//                 SORT_CEN - currentFileAttribute.sortMode,
//                 textureOffset + currentFileAttribute.texture, //should be texture + index of the first texture belonging to this model
//                 currentFileAttribute.hasTexture? No_Palet : currentFileAttribute.baseColor,
//                 No_Gouraud, //gouraud support to be added
//                 CL32KRGB | ECdis |
//                     (currentFileAttribute.hasMeshEffect != 0 ? MESHon : MESHoff) |
//                     (currentFileAttribute.hasFlatShading != 0 ? 0 : CL_Gouraud) |
//                     (currentFileAttribute.hasTransparency != 0 ? CL_Trans : 0) |
//                     (currentFileAttribute.hasHalfBrightness != 0 ? CL_Half : 0),
//                 currentFileAttribute.isWireframe != 0 ?
//                     sprPolyLine : (currentFileAttribute.hasTexture != 0 ? sprNoflip : sprPolygon),
//                 (currentFileAttribute.hasFlatShading != 0 ? UseLight : UseGouraud));
//         }
//     }

//     //It'd be better if all texture headers were stored together in a block, and then all textures next to one another in another block.
//     //That way we would copy everything at once and be much faster. In case you want to define your own file format..
//     //Well actually the same can be said about all the POINTS and all the POLYGONS.
//     *textureCountPtr = header.textureCount;
//     Uint16 pixelCount = 0;
//     for(Uint32 i = header.textureCount; i > 0; i--, textures++) {
//         TTNYAFileTextureHeader textureHeader;
//         ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (Uint8*) &textureHeader);
//         *textures = (TEXTURE) TEXTBL(textureHeader.width, textureHeader.height, *textureDataAddrPtr - TT_VDP1_VRAM);
//         const Uint32 imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);

//         #ifdef TT_DEBUG_MODE
//             if(CG_COUNT_TO_ADDR_OFFSET(textures->CGadr) + imageSize > GouraudRAM) {
//                 ttDebugMsgLn("Texture load stopped before overflow");
//                 //Remove all references to textures that couldn't be loaded
//                 Uint16 lastValidTextureIndex = header.textureCount - i - 1;
//                 meshes--;
//                 for(Uint32 i = header.meshCount; i > 0; i--, meshes--) {
//                     attributes--;
//                     for(Uint32 j = meshes->nbPolygon; j > 0; j--, attributes--)
//                         if(attributes->texno > lastValidTextureIndex) {
//                             attributes->texno = No_Texture;
//                         }
//                 }
//                 break;
//             }
//         #endif

//         ttReadBytes(&stream, imageSize, (Uint8*) TT_VDP1_VRAM + CG_COUNT_TO_ADDR_OFFSET(textures->CGadr));
//         *textureDataAddrPtr = AdjCG(*textureDataAddrPtr, textureHeader.width, textureHeader.height, COL_32K);
//     }
//     ttCloseFileStream(&stream);
//     return header.meshCount;
// }
