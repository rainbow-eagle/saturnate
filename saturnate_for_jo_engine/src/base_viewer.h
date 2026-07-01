#ifndef BASE_VIEWER_H
#define BASE_VIEWER_H

#include <jo/sgl_prototypes.h>
#include "pool_engine/file.h"
#include "pool_engine/model_loader.h"
#include "pool_engine/animation.h"

#define MAX_MODEL_FILE_COUNT (MAX_FILE_COUNT_PER_FOLDER-2)

#define MAX_ANIMATION_COUNT 200

#if TT_MOT == 1
#define MAX_TRANSFORM_COUNT 110000
// #define MAX_TRANSFORM_COUNT 50000 //max if everything in HRAM (needs to be even less than that actually)
#elif TT_MOT == 0
#define MAX_TRANSFORM_COUNT 52200 //max if MOT0
// #define MAX_TRANSFORM_COUNT 25000 //max if everything in HRAM (needs to be even less than that actually)
#endif //TT_MOT == 0

#define MAX_POSE_COUNT (MAX_TRANSFORM_COUNT / MAX_ANIMATED_MODEL_COUNT) //This size isn't mathematicaly absolutely accurate, just a good guess of what could be POSE_COUNT
#define MAX_ANIMATED_INSTANCE_COUNT 30

// #define MAX_POINT_COUNT 5500
// #define MAX_POLYGON_COUNT 4800
// #define MAX_TEXTURE_COUNT 4800

#define MAX_POINT_COUNT 5000
#define MAX_POLYGON_COUNT 4000
#define MAX_TEXTURE_COUNT 4000

// #define MAX_POINT_COUNT 5500
// #define MAX_POLYGON_COUNT 3750
// #define MAX_TEXTURE_COUNT 3750

// #define MAX_POINT_COUNT 5300
// #define MAX_POLYGON_COUNT 3600
// #define MAX_TEXTURE_COUNT 3600

// #define MAX_POINT_COUNT 2500
// #define MAX_POLYGON_COUNT 1700
// #define MAX_TEXTURE_COUNT 1700

#define MAX_MESH_COUNT 150
#define MAX_ANIMATED_MODEL_COUNT 10

extern TTAnimationPools animationPools; //including this file gives access to the pools
extern TTModelPools modelPools;

typedef struct {
    void (*playModel)(Uint32 modelID);
    void (*pressA)();
} ModelPlayer;

typedef struct {
    Uint8 modelCount;
    Uint8 nyaFileIDs[MAX_MODEL_FILE_COUNT]; // indexes of the nya files in gfsDirNames
    Uint8 instanceCountPerModel[MAX_MODEL_FILE_COUNT];
} Settings;

extern ModelPlayer *currentModelPlayer;

extern Uint8 nyaFiles[MAX_MODEL_FILE_COUNT];
// extern Uint8* nyaFiles;
extern Uint32 modelFileCount;

extern Uint32 currentModelIDInPools;
extern Uint32 currentModelIDInSettings;
extern Uint32 currentInstanceID;
extern Uint32 currentModelAnimationCount;
extern Uint32 currentAnimationID;

extern Settings settings;
extern Uint8 remainingInstanceCountPerModel[MAX_MODEL_FILE_COUNT];
extern Bool isAllInstanceSet;

TEXTURE* initPoolsAndGetTexturePtr();
Uint32 endsWith(const char *str, const char *suffix);
void listNYAFiles();
Sint32 getCorrespondingMOTFileGFSID(char* nyaFileName);
Sint32 loadModel(Uint32 nyaFileIndex);
void playAnimation(Uint32 animationID);
void playNextAnimation();
void playPreviousAnimation();
void toggleAnimationPause();
void resetRemainingInstanceCount();
void playFirstFreeInstanceForModel(Uint32 modelID);
void playPreviousModel();
void playNextModel();
void checkViewerInput();

#endif //BASE_VIEWER_H