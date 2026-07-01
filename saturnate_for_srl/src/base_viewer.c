#include <stdio.h>
#include <string.h>
#include <sgl.h>
#include "base_viewer.h"
#include "menu.h"
#include "pool_engine/model_loader.h"
#include "debug/print.h"
#include "pool_engine/model_loader.h"
#include "pool_engine/animation.h"

extern int snprintf(char *str, size_t size, const char *format, ...);

// TT_ALLOC_MODEL_POOLS(modelPools, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POINT_COUNT, MAX_POLYGON_COUNT,
//                         MAX_TEXTURE_COUNT, TT_HIGH_RAM_ATTRIBUTE)
// TT_ALLOC_ANIMATION_POOLS(animationPools, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POSE_COUNT,
//                         MAX_ANIMATION_COUNT, TT_HIGH_RAM_ATTRIBUTE, MAX_TRANSFORM_COUNT, TT_HIGH_RAM_ATTRIBUTE,
//                         MAX_ANIMATED_INSTANCE_COUNT, TT_HIGH_RAM_ATTRIBUTE)
TTModelPools modelPools;
TTAnimationPools animationPools;

ModelPlayer *currentModelPlayer;

uint8_t nyaFiles[MAX_MODEL_FILE_COUNT]; // indexes of the nya files in gfsDirNames
// uint8_t* nyaFiles = (uint8_t*) 0x00200000;
uint32_t modelFileCount;

uint32_t currentModelIDInPools;
uint32_t currentModelIDInSettings;
uint32_t currentInstanceID;
uint32_t currentModelAnimationCount;
uint32_t currentAnimationID;

Settings settings;
uint8_t remainingInstanceCountPerModel[MAX_MODEL_FILE_COUNT];
bool isAllInstanceSet;

TEXTURE* initPoolsAndGetTexturePtr() {
    uint8_t* poolsHRAMPtr = (uint8_t*) TT_HRAM_HEAP_START;
    uint8_t* poolsLRAMPtr = (uint8_t*) TT_LRAM_HEAP_START;
    ttInitModelPools(&modelPools, &poolsHRAMPtr, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT,
                    MAX_POINT_COUNT, MAX_POLYGON_COUNT, MAX_TEXTURE_COUNT);
    ttInitAnimationPools(&animationPools, &poolsHRAMPtr, &poolsLRAMPtr, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POSE_COUNT,
                        MAX_ANIMATION_COUNT, TT_HRAM, MAX_TRANSFORM_COUNT, TT_LRAM, MAX_ANIMATED_INSTANCE_COUNT, TT_HRAM);
    return modelPools.texturePool;
}

uint32_t endsWith(const char *str, const char *suffix) {
    uint32_t strLength = strlen(str);
    uint32_t suffixLength = strlen(suffix);

    if(suffixLength > strLength)
        return 0;

    return strcmp(&str[strLength - suffixLength], suffix) == 0;
}

void listNYAFiles() {
    modelFileCount = 0;
    for(uint32_t i = 2; i < gfsDirTbl.ndir; i++)
        if(endsWith(gfsDirNames[i].fname, ".NYA"))
            nyaFiles[modelFileCount++] = i;
    currentModelIDInSettings = 0;
}

int32_t getCorrespondingMOTFileGFSID(char* nyaFileName) {
    char motFileName[12];
    char *extension = strrchr(nyaFileName, '.');
    uint32_t lengthWithoutExtension = extension - nyaFileName;
    strncpy(motFileName, nyaFileName, lengthWithoutExtension);
    motFileName[lengthWithoutExtension] = '\0';
    strcat(motFileName, ".MOT");
    return GFS_NameToId(motFileName);
}

//returnValue = 2 means "model without an animation" - this case is not handled yet though
int32_t loadModel(uint32_t nyaFileIndex) {
    char msg[40];
    char* nyaFileName = GFS_IdToName(nyaFiles[nyaFileIndex]);
    int32_t returnValue = ttLoadNYAMeshesInPool(&modelPools, nyaFileName);
    switch(returnValue) {
        case TT_VRAM_FULL:
            snprintf(msg, sizeof(msg), "%s: VRAM full, texture incomplete%39s", nyaFileName, "");
            ttDebugMsgLn(msg);
        case OK:;//weird semicolon that I need to add to have it run on Jo Engine's compiler
            int32_t motFileGFSID = getCorrespondingMOTFileGFSID(nyaFileName);
            if(motFileGFSID > 0) {
                returnValue = ttLoadAnimationInPool(&modelPools, &animationPools, GFS_IdToName(motFileGFSID));//TBD
                if(returnValue) {
                    char* statusMessage;
                    switch(returnValue) {
                        case TT_ANIMATION_POOL_OVERFLOW: statusMessage = "animation"; break;
                        case TT_BONE_POOL_OVERFLOW: statusMessage = "bone"; break;
                        case TT_POSE_POOL_OVERFLOW: statusMessage = "pose"; break;
                        case TT_TRANSFORM_POOL_OVERFLOW: statusMessage = "transform"; break;
                        case TT_ANIMATION_SET_POOL_OVERFLOW: statusMessage = "animation set"; break;
                    }
                    snprintf(msg, sizeof(msg), "%s failed: %s overflow %-39s", nyaFileName, statusMessage, "");
                    ttDebugMsgLn(msg);
                }
            } else
                returnValue = 2;
            break;
        default:;//weird semicolon that I need to add to have it run on Jo Engine's compiler
            char* statusMessage = NULL;
            switch(returnValue) {
                case TT_MESH_POOL_OVERFLOW: statusMessage = "mesh"; break;
                case TT_POINT_POOL_OVERFLOW: statusMessage = "vertex"; break;
                case TT_POLYGON_POOL_OVERFLOW: statusMessage = "polygon"; break;
                case TT_TEXTURE_POOL_OVERFLOW: statusMessage = "texture (not VRAM)"; break;
                case TT_MODEL_POOL_OVERFLOW: statusMessage = "model"; break;
            }
            snprintf(msg, sizeof(msg), "%s failed: %s overflow %-39s", nyaFileName, statusMessage, "");
            ttDebugMsgLn(msg);
    }

    return returnValue;
}

void updateAnimationHUD(uint32_t animationID) {
    char msg[15];
    snprintf(msg, sizeof(msg), "Anim  %2d/%-2d", animationID+1, currentModelAnimationCount);
    ttDebugMsgXY(msg, 0, 1);
}

void playAnimation(uint32_t animationID) {
    updateAnimationHUD(animationID);

    currentAnimationID = animationID;
    ttStartAnimationInPool(&animationPools, currentModelIDInPools, currentInstanceID, animationID);//TBD
}

void playNextAnimation() {
    playAnimation((currentAnimationID + 1) % currentModelAnimationCount);
}

void playPreviousAnimation() {
    playAnimation(((currentAnimationID <= 0)? currentModelAnimationCount : currentAnimationID) - 1);
}

void toggleAnimationPause() {
    if(ttIsAnimationPausedInPool(&animationPools, currentModelIDInPools, currentInstanceID))
        ttUnpauseAnimationInPool(&animationPools, currentModelIDInPools, currentInstanceID);
    else
        ttPauseAnimationInPool(&animationPools, currentModelIDInPools, currentInstanceID);
}

void resetRemainingInstanceCount() {
    for(uint32_t modelID = 0; modelID < settings.modelCount; modelID++)
        remainingInstanceCountPerModel[modelID] = settings.instanceCountPerModel[modelID];
}

void playFirstFreeInstanceForModel(uint32_t modelID) {
    char msg[17];
    currentInstanceID = settings.instanceCountPerModel[modelID] - remainingInstanceCountPerModel[modelID];
    snprintf(msg, sizeof(msg), "Instance %3d/%-3d", currentInstanceID + 1, settings.instanceCountPerModel[modelID]);
    ttDebugMsgXY(msg, 24, 0);

    if(!isAllInstanceSet)
        playAnimation(0);
    else
        updateAnimationHUD(currentAnimationID);
}

void playPreviousModel() {
    uint32_t modelID = currentModelIDInSettings;
    uint32_t i = settings.modelCount;
    do {
        modelID = ((modelID <= 0)? settings.modelCount : modelID) - 1;
    } while(i-- > 0 && remainingInstanceCountPerModel[modelID] == 0);

    currentModelPlayer->playModel(modelID);
}

void playNextModel() {
    uint32_t modelID = currentModelIDInSettings;
    int32_t i = settings.modelCount;
    do {
        modelID = (modelID + 1) % settings.modelCount;
    } while(i-- > 0 && remainingInstanceCountPerModel[modelID] == 0);
    if(i <= 0 && remainingInstanceCountPerModel[modelID] == 0) {
        isAllInstanceSet = TRUE;
        resetRemainingInstanceCount();
        currentModelPlayer->playModel(0);
    }

    currentModelPlayer->playModel(modelID);
}

void checkViewerInput() {
    if(Per_Connect1) {
        uint16_t data = Smpc_Peripheral[0].push;
        if(!(data & PER_DGT_TA))
            currentModelPlayer->pressA();
            // pasteCurrentInstance();
        if(!(data & PER_DGT_TX))
            playPreviousAnimation();	
        if(!(data & PER_DGT_TY))
            playNextAnimation();
        if(!(data & PER_DGT_TR))
            playNextModel();
        if(!(data & PER_DGT_TL))
            playPreviousModel();
        if(!(data & PER_DGT_TZ))
            toggleAnimationPause();
        if(!(data & PER_DGT_ST))
            toggleMenu();

        data = Smpc_Peripheral[0].data;

        if(!(data & PER_DGT_TC)) {
            if(!(data & PER_DGT_KU))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).yOffset -= toFIXED(3);
            else if(!(data & PER_DGT_KD))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).yOffset += toFIXED(3);
            if(!(data & PER_DGT_KR))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).zAngle += DEGtoANG(3);
            else if(!(data & PER_DGT_KL))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).zAngle -= DEGtoANG(3);
        } else if(!(data & PER_DGT_TB)) {
            if(!(data & PER_DGT_KR))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).xOffset += toFIXED(3);
            else if(!(data & PER_DGT_KL))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).xOffset -= toFIXED(3);
            if(!(data & PER_DGT_KU))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).zOffset += toFIXED(3);
            else if(!(data & PER_DGT_KD))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).zOffset -= toFIXED(3);
        } else {
            if(!(data & PER_DGT_KR))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).yAngle -= DEGtoANG(3);
            else if(!(data & PER_DGT_KL))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).yAngle += DEGtoANG(3);
            if(!(data & PER_DGT_KU))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).xAngle -= DEGtoANG(3);
            else if(!(data & PER_DGT_KD))
                ttGetIntanceCurrentPosition(animationPools, currentModelIDInPools, currentInstanceID).xAngle += DEGtoANG(3);
        }
    }
}
