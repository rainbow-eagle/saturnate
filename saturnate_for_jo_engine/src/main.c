#include <stdio.h>
#include <string.h>
#include "jo/sgl_prototypes.h"
#include "core/memory.h"
#include "pool_engine/file.h"
#include "core/model_loader.h"
#include "pool_engine/model_loader.h"
#include "core/animation.h"
#include "pool_engine/animation.h"
#include "pool_engine/file.h"
#include "pool_engine/debug.h"

char toBePrinted[20];
Uint8 nyaFiles[MAX_FILE_COUNT_PER_FOLDER-2]; // indexes of the nya files in gfsDirNames
Uint32 modelFileCount;
Uint32 currentModelFileID;
const Uint32 modelCount = 1; // Always 1 since we only load one model at a time
const Uint32 currentModelID = 0; // Always 0 since we only load one model at a time
const Uint32 currentInstanceID = 0; //Always 0 since we only have a single instance on screen
Uint32 currentModelMeshCount;
Uint32 currentModelAnimationCount;
Uint32 currentAnimationID;
Sint8 currentlyPausedPose;
Bool isHelpEnabled = FALSE;

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
    currentModelFileID = 0;
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

char* loadModel(Uint32 nyaFileIndex) {
    char* nyaFileName = GFS_IdToName(nyaFiles[nyaFileIndex]);
    ttLoadNYAMeshesInPool(nyaFileName);
    Sint32 motFileGFSID = getCorrespondingMOTFileGFSID(nyaFileName);
    if(motFileGFSID > 0) {
        ttLoadAnimationInPool(GFS_IdToName(motFileGFSID));
    }
}

void playAnimation(Uint32 animationID) {
    char msg[15];
    snprintf(msg, sizeof(msg), "Anim  %2d/%-2d", currentAnimationID+1, currentModelAnimationCount);
    ttDebugMsgXY(msg, 0, 1);

    currentAnimationID = animationID;
    ttStartAnimationInPool(currentModelID, currentInstanceID, currentAnimationID);
}

void playNextAnimation() {
    Uint32 animationID = (currentAnimationID + 1) % currentModelAnimationCount;
    playAnimation(animationID);
}

void playPreviousAnimation() {
    Uint32 animationID = ((currentAnimationID <= 0)? currentModelAnimationCount : currentAnimationID) - 1;
    playAnimation(animationID);
}

void pauseAnimation() {
    if(animationStatePool[currentInstanceID].currentPoseID != -1) {
        currentlyPausedPose = animationStatePool[currentInstanceID].currentPoseID;
        animationStatePool[currentInstanceID].currentPoseID = -1;
    } else
        animationStatePool[currentInstanceID].currentPoseID = currentlyPausedPose;
}

void playModel(Uint32 modelID) {
    char msg[26];
    snprintf(msg, sizeof(msg), "Model %2d/%-2d %s", currentModelFileID+1, modelFileCount, gfsDirNames[nyaFiles[currentModelFileID]].fname);
    ttDebugMsgXY(msg, 0, 0);

    ttResetModelPools();
    ttResetAnimationPools();
    loadModel(currentModelFileID);
    currentModelAnimationCount = (nextFreeAnimationSetSpec-1)->animationCount;
    ttSetAnimatedInstanceCountInPool(modelCount, (Uint8[]){1});// Only 1 animated model, and one instance of it.
    ttSetAnimatedInstancePositionInPool(currentModelID, currentInstanceID, (TTInstancePosition) {toFIXED(0),toFIXED(0),toFIXED(150), DEGtoANG(0), DEGtoANG(0), DEGtoANG(0), 0}); //sets the position of the instance;
    playAnimation(0);
}

void playPreviousModel() {
    currentModelFileID = ((currentModelFileID <= 0)? modelFileCount : currentModelFileID) - 1;
    playModel(currentModelFileID);
}

void playNextModel() {
    currentModelFileID = (currentModelFileID + 1) % modelFileCount;
    playModel(currentModelFileID);
}

void toggleHelp() {
    isHelpEnabled = !isHelpEnabled;
    if(isHelpEnabled) {
        int lineNb = 20;
        ttDebugMsgXY("L/R previous/next model", 0, lineNb++);
        ttDebugMsgXY("X/Y previous/next animation", 0, lineNb++);
        ttDebugMsgXY("DPAD rotate", 0, lineNb++);
        ttDebugMsgXY("B + DPAD move around", 0, lineNb++);
        ttDebugMsgXY("C + up/down move up and down", 0, lineNb++);
        ttDebugMsgXY("C + right/left useless rotation", 0, lineNb++);
        ttDebugMsgXY("Z pause animation", 0, lineNb++);
    }
    else {
        for(int i = 20; i < 30; i++)
        ttDebugMsgXY("", 0, i);
    }
}

void checkInput() {
    if(Per_Connect1) {
        Uint16 data = Smpc_Peripheral[0].push;
        if(!(data & PER_DGT_TX))
            playPreviousAnimation();
        if(!(data & PER_DGT_TY))
            playNextAnimation();
        if(!(data & PER_DGT_TR))
            playNextModel();
        if(!(data & PER_DGT_TL))
            playPreviousModel();
        if(!(data & PER_DGT_TZ))
            pauseAnimation();
        if(!(data & PER_DGT_ST))
            toggleHelp();

        data = Smpc_Peripheral[0].data;
        if(!(data & PER_DGT_TC)) {
            if(!(data & PER_DGT_KU))
                animatedInstanceCurrentPositionPool[currentInstanceID].yOffset -= toFIXED(3);
            else if(!(data & PER_DGT_KD))
                animatedInstanceCurrentPositionPool[currentInstanceID].yOffset += toFIXED(3);
            if(!(data & PER_DGT_KR))
                animatedInstanceCurrentPositionPool[currentInstanceID].zAngle += DEGtoANG(3);
            else if(!(data & PER_DGT_KL))
                animatedInstanceCurrentPositionPool[currentInstanceID].zAngle -= DEGtoANG(3);
        } else if(!(data & PER_DGT_TB)) {
            if(!(data & PER_DGT_KR))
                animatedInstanceCurrentPositionPool[currentInstanceID].xOffset += toFIXED(3);
            else if(!(data & PER_DGT_KL))
                animatedInstanceCurrentPositionPool[currentInstanceID].xOffset -= toFIXED(3);
            if(!(data & PER_DGT_KU))
                animatedInstanceCurrentPositionPool[currentInstanceID].zOffset += toFIXED(3);
            else if(!(data & PER_DGT_KD))
                animatedInstanceCurrentPositionPool[currentInstanceID].zOffset -= toFIXED(3);
        } else {
            if(!(data & PER_DGT_KR))
                animatedInstanceCurrentPositionPool[currentInstanceID].yAngle -= DEGtoANG(3);
            else if(!(data & PER_DGT_KL))
                animatedInstanceCurrentPositionPool[currentInstanceID].yAngle += DEGtoANG(3);
            if(!(data & PER_DGT_KU))
                animatedInstanceCurrentPositionPool[currentInstanceID].xAngle -= DEGtoANG(3);
            else if(!(data & PER_DGT_KD))
                animatedInstanceCurrentPositionPool[currentInstanceID].xAngle += DEGtoANG(3);
        }
    }
}

//tmp function, should be improved and stored better (and deal with framerate correction)
void initSystem() {
    volatile Uint8* areaCodePtr = (Uint8*) AREA_CODE_REGISTER;
    enum tvsz resolution;
    if(*areaCodePtr == SMPC_AREA_EU_PAL || *areaCodePtr == SMPC_AREA_AJ_PAL)
        resolution = TV_320x256;
    else
        resolution = TV_320x240;
    slInitSystem(resolution, texturePool, 1);
    ttDebugMsgXY(resolution==TV_320x240? "Region: NTSC" : "Region: PAL", 0, 30);
}

void ss_main(void) {
	initSystem();
	slZdspLevel(3);
	ttInitFileSystem();

	ttSetDir("ASSETS");
	listNYAFiles();
	slBack1ColSet((void*) VDP2_VRAM_A1, CD_DarkWhite);

    if(modelFileCount < 1) {
        ttDebugMsgLn("No model found !");
        ttDebugMsgLn("Add NYA+MOT files to asset dir !");
        while(1);
    }

	playModel(0);

	while(-1) {
		checkInput();
		ttUpdateAnimationsInPool();
		slPushMatrix();
		{
			ttDrawAnimatedModelsFromPool();
		}
		slPopMatrix();
		slSynch();
	}
}

