#ifndef TT_CORE_MODEL_LOADER_H
#define TT_CORE_MODEL_LOADER_H

#include <sgl.h>

/**
 * @defgroup ModelLoader Model Loader
 * @brief Functions to load models from a file into memory.
 *
 * @addtogroup ModelLoader
 * @{
 */

 /**
 * @brief Container describing where to load a 3D model.
 * 
 * A Model is described by meshes, vertices, polygons, attributes, texture metadata and the texture data itself.
 * Data is to be loaded to the pointers the user provides, respecting the containers' size thank to the provided "maxCount" parameters.
 * The number of meshes and textures loaded will be respectively placed in meshCountPtr and textureCountPtr at load time.
 */
typedef struct {
    uint8_t* meshCountPtr; ///< [out] Pointer to the total number of loaded meshes.
    PDATA* meshes; ///< [out] Array of PDATA where meshes are to be stored (one per mesh).
    uint32_t maxMeshCount; ///< [in] Maximum amount of PDATA that can be added to meshes.
    POINT* points; ///< [out] Vertex array (POINT) to store all mesh coordinates.
    uint32_t maxPointCount; ///< [in] Maximum amount of POINT that can be added to points.
    POLYGON* polygons; ///< [out] Polygon array (POLYGON) defining face indices.
    uint32_t maxPolygonCount; ///< [in] Maximum amount of POLYGON that can be added to polygons.
    ATTR* attributes; ///< [out] Attribute array (ATTR) where attributes are to be stored (one per polygon).
    TEXTURE* textures; ///< [out] Texture array (TEXTURE). Metadata is stored in this array, while raw pixel data is streamed directly to VRAM.
    uint32_t maxTextureCount; ///< [in] Maximum amount of TEXTURE that can be added to textures.
    uint32_t textureOffset; ///< [in] Index of the next free texture slot in SGL's texture pool (the one expected by slInitSystem).
    uint32_t* textureCountPtr; ///< [out] Pointer to the total number of loaded textures.
    uint32_t* textureDataAddrPtr; ///< [out] Pointer to the where the texture data (actual pixels) is to be stored in VRAM.
} TTMeshBuffer;

/**
 * @brief Metadata describing a 3D model from a .NYA file.
 */
typedef struct {
    uint32_t pointCount; ///< Total number of vertices in the file.
    uint32_t polygonCount; ///< Total number of polygons in the file.
    uint32_t textureCount; ///< Total number of textures in the file.
    uint16_t vramCGCount; ///< Total number of VRAM cells that would be used by the textures in the file.
    uint8_t meshCount; ///< Total number of meshes in the file.
    uint8_t reservedSpace;
} TTNYAFileMetadata;

/**
 * @brief Return codes for model loading operations.
 * 
 * SGL's OK value (zero) means success.
 * A negative value indicates a critical error preventing the use of the model.
 * A full VRAM indicates that the model can be used, but at least one texture wasn't loaded because of a lack of space in VRAM. The polygons using this texture will appear without texture.
 */
typedef enum {
    TT_VRAM_FULL = 1, /**< The texture data exceeds the capacity of the VRAM; the meshes could be loaded but some polygons will lack their texture. */
    TT_MESH_OVERFLOW = -1, /**< The number of PDATA exceeds the capacity of the provided container; the meshes couldn't be loaded. */
    TT_POINT_OVERFLOW = -2, /**< The number of POINT exceeds the capacity of the provided container; the meshes couldn't be loaded. */
    TT_POLYGON_OVERFLOW = -3, /**< The number of POLYGON exceeds the capacity of the provided container; the meshes couldn't be loaded. */
    TT_TEXTURE_OVERFLOW = -4 /**< The number of TEXTURE exceeds the capacity of the provided container; the meshes couldn't be loaded. */
} TTMeshLoadStatus;

/**
 * @brief Loads the metadata of the model described in a .NYA file into the given struct.
 * 
 * @note Requires GFS_Init() to be called prior to use.
 * @note This function is almost as slow as loading the file with ttLoadNYAMeshes(). A workflow based on checking the size
 * and then opening the file (this function and then ttLoadNYAMeshes()) should be avoided to optimize loading speed.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[out] metadata The structure where the metadata are to be loaded.
 *
 * @see GFS_Init()
 */
void ttLoadNYAMetadata(char* fileName, TTNYAFileMetadata* metadata);

/**
 * @brief Loads meshes from a NYA format file into the given user-allocated arrays and transfers texture data to VDP1 VRAM.
 * 
 * This function parses a .NYA file to populate user-allocated data structures. 
 * It handles the conversion from the NYA attribute format to SGL-compliant 
 * ATTR structures and uploads pixel data directly to the VDP1 CG RAM.
 * 
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @return TTMeshLoadStatus success code, only used if TT_DEBUG_MODE is defined, otherwise always return TT_SUCCESS.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[in,out] animationBuffer structure providing the pointers to where the data will be loaded.
 * 
 * @warning Output arrays must be pre-allocated with sufficient capacity to hold 
 * the entire file content to avoid returning an error code.
 * @see GFS_Init()
 */
TTMeshLoadStatus ttLoadNYAMeshes(char* fileName, TTMeshBuffer* meshBuffer);

/** @} */ //end of group ModelLoader

#endif // TT_CORE_MODEL_LOADER_H
