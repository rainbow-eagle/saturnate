#include <sgl.h>
#include "model_loader.h"
#include "file.h"
#include "memory.h"
#include "../debug/print.h"

#define IMG_SIZE_PX(pixelCount, colorMode) (((pixelCount) * 4) >> colorMode)
#define IMG_SIZE_WH(width, height, colorMode) IMG_SIZE_PX((width) * (height), colorMode)

typedef struct {
    uint32_t type;
    uint32_t meshCount;
    uint32_t textureCount;
} TTNYAFileHeader;

typedef struct {
    uint32_t pointCount;
    uint32_t polygonCount;
} TTNYAFileMeshHeader;

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
} TTNYAFileAttribute;

typedef struct {
    uint16_t width;
    uint16_t height;
} TTNYAFileTextureHeader;

void ttLoadNYAMetadata(char* fileName, TTNYAFileMetadata* metadata) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);

    uint32_t pointCount = 0;
    uint32_t polygonCount = 0;
    TTNYAFileHeader header;
    ttReadBytes(&stream, sizeof(TTNYAFileHeader), (uint8_t*) &header);
    for(uint32_t i = header.meshCount; i > 0; i--) {
        TTNYAFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (uint8_t*) &meshHeader);
        pointCount += meshHeader.pointCount;
        uint32_t size = sizeof(POINT) * meshHeader.pointCount;
        ttSkipBytes(&stream, size);
        polygonCount += meshHeader.polygonCount;
        size = sizeof(POLYGON) * meshHeader.polygonCount + sizeof(TTNYAFileAttribute) * meshHeader.polygonCount;
        ttSkipBytes(&stream, size);
    }
    metadata->pointCount = pointCount;
    metadata->polygonCount = polygonCount;
    metadata->meshCount = header.meshCount;

    metadata->textureCount = header.textureCount;
    uint32_t textureDataAddr = 0;
    for(uint32_t i = header.textureCount; i > 0; i--) {
        TTNYAFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (uint8_t*) &textureHeader);
        const uint32_t imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);
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
    ttReadBytes(&stream, sizeof(TTNYAFileHeader), (uint8_t*) &header);
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
        uint32_t totalPointCount = 0;
        uint32_t totalPolygonCount = 0;
    #endif
    for(uint32_t i = header.meshCount; i > 0; i--, meshes++) {
        TTNYAFileMeshHeader meshHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (uint8_t*) &meshHeader);
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
            TTNYAFileAttribute currentFileAttribute;
            ttReadBytes(&stream, sizeof(TTNYAFileAttribute), (uint8_t*) &currentFileAttribute);
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
    uint16_t pixelCount = 0;
    for(uint32_t i = header.textureCount; i > 0; i--, textures++) {
        TTNYAFileTextureHeader textureHeader;
        ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (uint8_t*) &textureHeader);
        *textures = (TEXTURE) TEXTBL(textureHeader.width, textureHeader.height, *meshBuffer->textureDataAddrPtr - TT_VDP1_VRAM);
        const uint32_t imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);

        #ifdef TT_DEBUG_MODE
            if(__builtin_expect(CG_COUNT_TO_ADDR_OFFSET(textures->CGadr) + imageSize > GouraudRAM, 0)) {
                //The actual texture count is less than previously thought
                *(meshBuffer->textureCountPtr) = header.textureCount - i;
                //Remove all references to textures that couldn't be loaded
                uint16_t lastValidTextureIndex = header.textureCount - i - 1;
                meshes--;
                for(uint32_t i = header.meshCount; i > 0; i--, meshes--) {
                    attributes--;
                    for(uint32_t j = meshes->nbPolygon; j > 0; j--, attributes--)
                        if(attributes->texno > lastValidTextureIndex) {
                            attributes->texno = No_Texture;
                        }
                }
                ttCloseFileStream(&stream);
                return TT_VRAM_FULL;
            }
        #endif

        ttReadBytes(&stream, imageSize, (uint8_t*) TT_VDP1_VRAM + CG_COUNT_TO_ADDR_OFFSET(textures->CGadr));
        *meshBuffer->textureDataAddrPtr = AdjCG(*meshBuffer->textureDataAddrPtr, textureHeader.width, textureHeader.height, COL_32K);
    }
    ttCloseFileStream(&stream);

    return OK;
}

// int32_t ttLoadNYAMeshes1(char* fileName, PDATA* meshes, POINT* points, POLYGON* polygons, ATTR* attributes,
//                         uint32_t textureOffset, uint32_t* textureCountPtr, TEXTURE* textures, uint32_t* textureDataAddrPtr) {
//     TTFileStream stream;
//     ttOpenFileStream(fileName, &stream);

//     TTNYAFileHeader header;
//     ttReadBytes(&stream, sizeof(TTNYAFileHeader), (uint8_t*) &header);
//     for(uint32_t i = header.meshCount; i > 0; i--, meshes++) {
//         TTNYAFileMeshHeader meshHeader;
//         ttReadBytes(&stream, sizeof(TTNYAFileMeshHeader), (uint8_t*) &meshHeader);
//         uint32_t size = sizeof(POINT) * meshHeader.pointCount;
//         ttReadBytes(&stream, size, (uint8_t*) points);
//         size = sizeof(POLYGON) * meshHeader.polygonCount;
//         ttReadBytes(&stream, size, (uint8_t*) polygons);
//         meshes->pntbl = points;
//         meshes->nbPoint = meshHeader.pointCount;
//         meshes->pltbl = polygons;
//         meshes->nbPolygon = meshHeader.polygonCount;
//         meshes->attbl = attributes;

//         points += meshHeader.pointCount;
//         polygons += meshHeader.polygonCount;

//         // The file Attribute format doesn't match the ATTR format, gotta convert
//         for(uint32_t j = meshHeader.polygonCount; j > 0; j--, attributes++) {
//             TTNYAFileAttribute currentFileAttribute;
//             ttReadBytes(&stream, sizeof(TTNYAFileAttribute), (uint8_t*) &currentFileAttribute);
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
//     uint16_t pixelCount = 0;
//     for(uint32_t i = header.textureCount; i > 0; i--, textures++) {
//         TTNYAFileTextureHeader textureHeader;
//         ttReadBytes(&stream, sizeof(TTNYAFileTextureHeader), (uint8_t*) &textureHeader);
//         *textures = (TEXTURE) TEXTBL(textureHeader.width, textureHeader.height, *textureDataAddrPtr - TT_VDP1_VRAM);
//         const uint32_t imageSize = IMG_SIZE_WH(textureHeader.width, textureHeader.height, COL_32K);

//         #ifdef TT_DEBUG_MODE
//             if(CG_COUNT_TO_ADDR_OFFSET(textures->CGadr) + imageSize > GouraudRAM) {
//                 ttDebugMsgLn("Texture load stopped before overflow");
//                 //Remove all references to textures that couldn't be loaded
//                 uint16_t lastValidTextureIndex = header.textureCount - i - 1;
//                 meshes--;
//                 for(uint32_t i = header.meshCount; i > 0; i--, meshes--) {
//                     attributes--;
//                     for(uint32_t j = meshes->nbPolygon; j > 0; j--, attributes--)
//                         if(attributes->texno > lastValidTextureIndex) {
//                             attributes->texno = No_Texture;
//                         }
//                 }
//                 break;
//             }
//         #endif

//         ttReadBytes(&stream, imageSize, (uint8_t*) TT_VDP1_VRAM + CG_COUNT_TO_ADDR_OFFSET(textures->CGadr));
//         *textureDataAddrPtr = AdjCG(*textureDataAddrPtr, textureHeader.width, textureHeader.height, COL_32K);
//     }
//     ttCloseFileStream(&stream);
//     return header.meshCount;
// }