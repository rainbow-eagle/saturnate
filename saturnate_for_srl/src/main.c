#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sgl.h>
#include <sega_sys.h>
#include "core/memory.h"
#include "pool_engine/file.h"
#include "core/model_loader.h"
#include "pool_engine/model_loader.h"
#include "core/animation.h"
#include "pool_engine/animation.h"
#include "pool_engine/file.h"
#include "pool_engine/debug.h"

extern int snprintf(char *str, size_t size, const char *format, ...);

char toBePrinted[20];
uint8_t nyaFiles[MAX_FILE_COUNT_PER_FOLDER-2]; // indexes of the nya files in gfsDirNames
uint32_t modelFileCount;
uint32_t currentModelFileID;
const uint32_t modelCount = 1; // Always 1 since we only load one model at a time
const uint32_t currentModelID = 0; // Always 0 since we only load one model at a time
const uint32_t currentInstanceID = 0; //Always 0 since we only have a single instance on screen
uint32_t currentModelMeshCount;
uint32_t currentModelAnimationCount;
uint32_t currentAnimationID;
int8_t currentlyPausedPose;
bool isHelpEnabled = FALSE;

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
    currentModelFileID = 0;
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

char* loadModel(uint32_t nyaFileIndex) {
    char* nyaFileName = GFS_IdToName(nyaFiles[nyaFileIndex]);
    ttLoadNYAMeshesInPool(nyaFileName);
    int32_t motFileGFSID = getCorrespondingMOTFileGFSID(nyaFileName);
    if(motFileGFSID > 0) {
        ttLoadAnimationInPool(GFS_IdToName(motFileGFSID));
    }
}

void playAnimation(uint32_t animationID) {
    char msg[15];
    snprintf(msg, sizeof(msg), "Anim  %2d/%-2d", currentAnimationID+1, currentModelAnimationCount);
    ttDebugMsgXY(msg, 0, 1);

    currentAnimationID = animationID;
    ttStartAnimationInPool(currentModelID, currentInstanceID, currentAnimationID);
}

void playNextAnimation() {
    uint32_t animationID = (currentAnimationID + 1) % currentModelAnimationCount;
    playAnimation(animationID);
}

void playPreviousAnimation() {
    uint32_t animationID = ((currentAnimationID <= 0)? currentModelAnimationCount : currentAnimationID) - 1;
    playAnimation(animationID);
}

void pauseAnimation() {
    if(animationStatePool[currentInstanceID].currentPoseID != -1) {
        currentlyPausedPose = animationStatePool[currentInstanceID].currentPoseID;
        animationStatePool[currentInstanceID].currentPoseID = -1;
    } else
        animationStatePool[currentInstanceID].currentPoseID = currentlyPausedPose;
}

void playModel(uint32_t modelID) {
    char msg[26];
    snprintf(msg, sizeof(msg), "Model %2d/%-2d %s", currentModelFileID+1, modelFileCount, gfsDirNames[nyaFiles[currentModelFileID]].fname);
    ttDebugMsgXY(msg, 0, 0);

    ttResetModelPools();
    ttResetAnimationPools();
    loadModel(currentModelFileID);
    currentModelAnimationCount = (nextFreeAnimationSetSpec-1)->animationCount;
    ttSetAnimatedInstanceCountInPool(modelCount, (uint8_t[]){1});// Only 1 animated model, and one instance of it.
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
        uint16_t data = Smpc_Peripheral[0].push;
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
    volatile uint8_t* areaCodePtr = (uint8_t*) AREA_CODE_REGISTER;
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

	ttSetDir("data");
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

// void ss_main(void) {
// 	slInitSystem(TV_320x224,texturePool,1);
// 	slZdspLevel(3);
// 	ttInitFileSystem();
// 	ttSetDir("ASSETS");
// 	slPrint("Shinobi" , slLocate(6,2));

// 	uint32_t returnedValue = 0;
// 	ttLoadNYAMeshesInPool("JO.NYA");
// 	ttLoadAnimationInPool("JO.MOT");
// 	uint32_t animIndex = 0;

// 	sprintf(toBePrinted, "ret = %d", returnedValue);
// 	slPrint(toBePrinted, slLocate(2, 10));

// 	sprintf(toBePrinted, "anim = %d, mesh = %d", animationSetSpecPool[0].animationCount, animationSetSpecPool[0].meshCount);
// 	slPrint(toBePrinted, slLocate(2, 24));
// 	sprintf(toBePrinted, "frame = %d, pose = %d", animationSpecPool[0].totalFrameCountBetweenPoses, animationSpecPool[0].totalPoseCount);
// 	slPrint(toBePrinted, slLocate(2, 25));

// 	ttSetAnimatedInstanceCountInPool(1, (uint8_t[]){2}); // Only one animated model, and two instances of it
// 	ttStartAnimationInPool(0, 0, 0); // In order: animate for the model 0, the instance 0, with the animationSpec 0
// 	ttStartAnimationInPool(0, 1, 1); // In order: animate for the model 0, the instance 1, with the animationSpec 1
// 	ttSetAnimatedInstancePositionInPool(0, 0, (TTInstancePosition) {toFIXED(50),toFIXED(0),toFIXED(150), DEGtoANG(0), DEGtoANG(45), DEGtoANG(0), 0});
// 	ttSetAnimatedInstancePositionInPool(0, 1, (TTInstancePosition) {toFIXED(-50),toFIXED(0),toFIXED(250), DEGtoANG(0), DEGtoANG(-45), DEGtoANG(0), 0});

// 	sprintf(toBePrinted, "frame = %d, pose = %d", animationStatePool->animationSpec.totalFrameCountBetweenPoses, animationStatePool->animationSpec.totalPoseCount);
// 	slPrint(toBePrinted, slLocate(2, 1));
// 	sprintf(toBePrinted, "frame = %d, pose = %d", animationStatePool->frameCountBeforeNextPose, animationStatePool->currentPoseID);
// 	slPrint(toBePrinted, slLocate(2, 2));

// 	while(-1) {
// 		slPushMatrix();
// 		{
// 			ttDrawAnimatedModelsFromPool();
// 		}
// 		slPopMatrix();
// 		slSynch();
// 		ttUpdateAnimationsInPool();
// 	}
// }

// TTAnimationSpec loadedAnimationSpecs[2]; // the file contains 2 animations
// //TTTransform loadedTransforms[2][32][15]; // 2 animations on 32 frames, with 15 meshes
// TTTransform loadedTransforms[32*15+16*15]; //2 animations : 32 poses over 15 frames and 16 poses over 16 frames
// // TTTransformSC loadedTransformsSC[32][15];

// void ss_main(void) {
// 	slInitSystem(TV_320x224,getTextureBuffer(),1);
// 	slZdspLevel(3);
// 	ttInitFile();
// 	slPrint("Shinobi" , slLocate(6,2));

// 	PDATA loadedModel[15]; // the model contains 15 meshes (i.e. 15 PDATA)
// 	uint32_t returnedValue = ttLoadNYAMeshes("JO.NYA", loadedModel);
// 	returnedValue = loadAnimation1("JO.MOT", loadedAnimationSpecs, 15, (TTTransform*) loadedTransforms);
// 	uint32_t animIndex = 0;
// 	// loadAnimationSC1("JOSC.MOT", loadedAnimationSpecs, 15, loadedTransformsSC);
// 	slDMAWait();
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->xOffset, slLocate(2,6));
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->yOffset, slLocate(2,8));
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->zOffset, slLocate(2,10));
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->xAngle, slLocate(2,12));
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->yAngle, slLocate(2,14));
// 	slPrintFX((loadedTransforms+32*15+14*15+1)->zAngle, slLocate(2,16));
// 	sprintf(toBePrinted, "%d", (loadedTransforms+32*15+14*15+1)->popCount);
// 	slPrint(toBePrinted, slLocate(2, 18));
// 	sprintf(toBePrinted, "frame = %d, pose = %d", loadedAnimationSpecs[animIndex].totalFrameCountBetweenPoses, loadedAnimationSpecs[animIndex].totalPoseCount);
// 	slPrint(toBePrinted, slLocate(2, 1));
// 	// sprintf(toBePrinted, "returnedValue = %d", returnedValue);
// 	// slPrint(toBePrinted, slLocate(2, 22));

// 	TTAnimationSpec animationSpecs[1];
// 	animationSpecs[0].totalPoseCount=32;
// 	animationSpecs[0].totalFrameCountBetweenPoses=3;

// 	TTAnimationState animationStates[2];
// 	animationStates[0].currentPoseID = loadedAnimationSpecs[0].totalPoseCount-1;
// 	animationStates[0].frameCountBeforeNextPose = loadedAnimationSpecs[0].totalFrameCountBetweenPoses;
// 	animationStates[1].currentPoseID = loadedAnimationSpecs[1].totalPoseCount-1;
// 	animationStates[1].frameCountBeforeNextPose = loadedAnimationSpecs[1].totalFrameCountBetweenPoses;

// 	while(-1) {
// 		slUnitMatrix(NULL);
// 		slPushMatrix();
// 		{
// 			slTranslate(0,toFIXED(0),toFIXED(150));
// 			slRotX(DEGtoANG(0));
// 			slRotY(DEGtoANG(30));
// 			slRotZ(DEGtoANG(0));
// 			// drawFrame(loadedModel, loadedTransforms[animIndex][animationStates[animIndex].currentPoseID], 15);
// 			drawFrame(loadedModel, loadedTransforms+animationStates[animIndex].currentPoseID*15, 15);
// 			// drawFrame(loadedModel, loadedTransforms+32*15+animationStates[animIndex].currentPoseID*15, 15);
// 			// drawFrameSC(loadedModel, loadedTransformsSC[animationStates[0].currentPoseID], 15);
// 		}
// 		slPopMatrix();
// 		updateAnimations1(animationStates, loadedAnimationSpecs, loadedAnimationSpecs+1);
// 		slSynch();
// 	}
// }
