#include <jo/sgl_prototypes.h>
#include <jo/sgl_prototypes.h>
#include "model_viewer.h"
#include "core/memory.h"
#include "pool_engine/model_loader.h"
#include "debug/print.h"
#include "game_state.h"
#include "base_viewer.h"
#include "menu.h"
#include "pool_engine/animation.h"//normal?

GameState modelViewerState = {initModelViewer, updateModelViewer, drawModelViewer};
ModelPlayer modelViewerPlayer = {playModelViewerModel, doNothing};

static const Uint32 modelCount = 1; // Always 1 since we only load one model at a time

//those 2 are from the "parent class" base_viewer
extern TTAnimationPools animationPools;
extern Settings settings;

void playModelViewerModel(Uint32 modelID) {
    currentModelIDInSettings = modelID;
    char msg[26];
    snprintf(msg, sizeof(msg), "Model %2d/%-2d %s", currentModelIDInSettings+1, modelFileCount, gfsDirNames[nyaFiles[currentModelIDInSettings]].fname);
    ttDebugMsgXY(msg, 0, 0);

    ttResetModelPools(&modelPools);
    ttResetAnimationPools(&animationPools);
    loadModel(currentModelIDInSettings);
    currentModelAnimationCount = ttGetModelAnimationCount(animationPools, currentModelIDInPools);
    ttSetAnimatedInstanceCountInPool(&animationPools, modelCount, (Uint8[]){1});// Only 1 animated model, and one instance of it.
    ttSetAnimatedInstancePositionInPool(&animationPools, currentModelIDInPools, currentInstanceID, ((TTInstancePosition) {toFIXED(0),toFIXED(90),toFIXED(300), DEGtoANG(0), DEGtoANG(0), DEGtoANG(0), 0})); //sets the position of the instance;
    playAnimation(0);
}

//that's what happens when you press A
void doNothing(){}

void initModelViewer() {
	ttSetDir("ASSETS");
	listNYAFiles();
	slBack1ColSet((void*) VDP2_VRAM_A1, CD_Black);

    if(modelFileCount < 1) {
        ttDebugMsgLn("No model found !");
        ttDebugMsgLn("Add NYA+MOT files to asset dir !");
        while(1);
    }
    settings.modelCount = modelFileCount;
    for(int i = 0; i < modelFileCount; i++)
        settings.instanceCountPerModel[i] = 1;

    currentModelIDInPools = 0; // Always 0 since we only load one model at a time
    currentInstanceID = 0;
    currentModelPlayer = &modelViewerPlayer;
    setMenuModelViewerMode();
    currentModelPlayer->playModel(0);
}

#include "stress_test.h"
GameStatus updateModelViewer() {
    if(!isMenuEnabled)
        checkViewerInput();
    switch(updateMenu()) {
        case MENU_GO_TO_NEXT_MODE:
            return GAME_NEXT;
    }
    ttUpdateAnimationsInPool(&animationPools); //TBD

    return GAME_CONTINUE;
}

void drawModelViewer() {
    slPushMatrix();
    {
        ttDrawAnimatedModelsFromPool(&modelPools, &animationPools);//TBD
    }
    slPopMatrix();
}
