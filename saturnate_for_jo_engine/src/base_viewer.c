#include <stdio.h>
#include <string.h>
#include "jo/sgl_prototypes.h"
#include "base_viewer.h"
#include "menu.h"
#include "pool_engine/model_loader.h"
#include "debug/print.h"
#include "pool_engine/model_loader.h"
#include "pool_engine/animation.h"

// TT_ALLOC_MODEL_POOLS(modelPools, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POINT_COUNT, MAX_POLYGON_COUNT,
//                         MAX_TEXTURE_COUNT, TT_HIGH_RAM_ATTRIBUTE)
// TT_ALLOC_ANIMATION_POOLS(animationPools, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POSE_COUNT,
//                         MAX_ANIMATION_COUNT, TT_HIGH_RAM_ATTRIBUTE, MAX_TRANSFORM_COUNT, TT_HIGH_RAM_ATTRIBUTE,
//                         MAX_ANIMATED_INSTANCE_COUNT, TT_HIGH_RAM_ATTRIBUTE)
TTModelPools modelPools;
TTAnimationPools animationPools;

ModelPlayer *currentModelPlayer;

Uint8 nyaFiles[MAX_MODEL_FILE_COUNT]; // indexes of the nya files in gfsDirNames
// Uint8* nyaFiles = (Uint8*) 0x00200000;
Uint32 modelFileCount;

Uint32 currentModelIDInPools;
Uint32 currentModelIDInSettings;
Uint32 currentInstanceID;
Uint32 currentModelAnimationCount;
Uint32 currentAnimationID;

Settings settings;
Uint8 remainingInstanceCountPerModel[MAX_MODEL_FILE_COUNT];
Bool isAllInstanceSet;

TEXTURE* initPoolsAndGetTexturePtr() {
    Uint8* poolsHRAMPtr = (Uint8*) TT_HRAM_HEAP_START;
    Uint8* poolsLRAMPtr = (Uint8*) TT_LRAM_HEAP_START;
    ttInitModelPools(&modelPools, &poolsHRAMPtr, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT,
                    MAX_POINT_COUNT, MAX_POLYGON_COUNT, MAX_TEXTURE_COUNT);
    ttInitAnimationPools(&animationPools, &poolsHRAMPtr, &poolsLRAMPtr, MAX_ANIMATED_MODEL_COUNT, MAX_MESH_COUNT, MAX_POSE_COUNT,
                        MAX_ANIMATION_COUNT, TT_HRAM, MAX_TRANSFORM_COUNT, TT_LRAM, MAX_ANIMATED_INSTANCE_COUNT, TT_HRAM);
    return modelPools.texturePool;
}

Uint32 endsWith(const char *str, const char *suffix) {
    Uint32 strLength = strlen(str);
    Uint32 suffixLength = strlen(suffix);

    if(suffixLength > strLength)
        return 0;

    return strcmp(&str[strLength - suffixLength], suffix) == 0;
}

void listNYAFiles() {
    modelFileCount = 0;
    for(Uint32 i = 2; i < gfsDirTbl.ndir; i++)
        if(endsWith(gfsDirNames[i].fname, ".NYA"))
            nyaFiles[modelFileCount++] = i;
    currentModelIDInSettings = 0;
}

Sint32 getCorrespondingMOTFileGFSID(char* nyaFileName) {
    char motFileName[12];
    char *extension = strrchr(nyaFileName, '.');
    Uint32 lengthWithoutExtension = extension - nyaFileName;
    strncpy(motFileName, nyaFileName, lengthWithoutExtension);
    motFileName[lengthWithoutExtension] = '\0';
    strcat(motFileName, ".MOT");
    return GFS_NameToId(motFileName);
}

//returnValue = 2 means "model without an animation" - this case is not handled yet though
Sint32 loadModel(Uint32 nyaFileIndex) {
    char msg[40];
    char* nyaFileName = GFS_IdToName(nyaFiles[nyaFileIndex]);
    Sint32 returnValue = ttLoadNYAMeshesInPool(&modelPools, nyaFileName);
    switch(returnValue) {
        case TT_VRAM_FULL:
            snprintf(msg, sizeof(msg), "%s: VRAM full, texture incomplete%39s", nyaFileName, "");
            ttDebugMsgLn(msg);
        case OK:;
            Sint32 motFileGFSID = getCorrespondingMOTFileGFSID(nyaFileName);
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
        default:;
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

void updateAnimationHUD(Uint32 animationID) {
    char msg[15];
    snprintf(msg, sizeof(msg), "Anim  %2d/%-2d", animationID+1, currentModelAnimationCount);
    ttDebugMsgXY(msg, 0, 1);
}

void playAnimation(Uint32 animationID) {
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
    for(Uint32 modelID = 0; modelID < settings.modelCount; modelID++)
        remainingInstanceCountPerModel[modelID] = settings.instanceCountPerModel[modelID];
}

void playFirstFreeInstanceForModel(Uint32 modelID) {
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
    Uint32 modelID = currentModelIDInSettings;
    Uint32 i = settings.modelCount;
    do {
        modelID = ((modelID <= 0)? settings.modelCount : modelID) - 1;
    } while(i-- > 0 && remainingInstanceCountPerModel[modelID] == 0);

    currentModelPlayer->playModel(modelID);
}

void playNextModel() {
    Uint32 modelID = currentModelIDInSettings;
    Sint32 i = settings.modelCount;
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
        Uint16 data = Smpc_Peripheral[0].push;
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

