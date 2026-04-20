#ifndef TT_CORE_MODEL_LOADER_H
#define TT_CORE_MODEL_LOADER_H

#include "jo/sgl_prototypes.h"

/**
 * @defgroup ModelLoader Model Loader
 * @brief Functions to load models from a file into memory.
 */

/**
 * @addtogroup ModelLoader
 * @{
 */

/**
 * @brief Loads meshes from a NYA format file and transfers texture data to VDP1 VRAM.
 * 
 * This function parses a .NYA file to populate user-allocated data structures. 
 * It handles the conversion from the NYA attribute format to SGL-compliant 
 * ATTR structures and uploads pixel data directly to the VDP1 CG RAM.
 * 
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @param[in] fileName Name of the the model file to be opened in the current directory.
 * @param[out] meshes Array of PDATA where meshes are to be stored (one per mesh).
 * @param[out] points Vertex array (POINT) to store all mesh coordinates.
 * @param[out] polygons Polygon array (POLYGON) defining face indices.
 * @param[out] attributes Attribute array (ATTR) where attributes are to be stored (one per polygon).
 * @param[in] textureOffset Index of the next free texture slot in SGL's texture pool (the one expected by slInitSystem).
 * @param[out] textureCountPtr The total number of loaded textures.
 * @param[out] textures Texture array (TEXTURE). Metadata is stored in this array,
 * while raw pixel data is streamed directly to VRAM.
 * @param[out] cgBlockIndex Index of the first block in VRAM where the texture are to be stored.
 * @return Sint32 The total number of meshes successfully loaded.
 * 
 * @warning Output arrays must be pre-allocated with sufficient capacity to hold 
 * the entire file content to avoid overflow.
 * @see GFS_Init()
 */

Sint32 ttLoadNYAMeshes(char* fileName, PDATA* meshes, POINT* vertices, POLYGON* polygons, ATTR* attributes,
                        Uint32 textureOffset, Uint32* textureCountPtr, TEXTURE* textures, Uint32 cgBlockIndex);

/** @} */

#endif // TT_CORE_MODEL_LOADER_H
