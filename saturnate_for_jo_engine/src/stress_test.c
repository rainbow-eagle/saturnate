#include "jo/sgl_prototypes.h"
#include "stress_test.h"
#include "pool_engine/model_loader.h"
#include "debug/print.h"
#include "game_state.h"
#include "menu.h"
#include "stress_test_settings.h"
#include "base_viewer.h"
#include "pool_engine/animation.h" //normal?

GameState stressTestState = {initStressTest, updateStressTest, drawStressTest};
ModelPlayer stressTestPlayer = {playStressTestModel, pasteCurrentInstance};

//those 2 are from the "parent class" base_viewer
extern TTAnimationPools animationPools;
extern Settings settings;

void resetStressTest() {
    isAllInstanceSet = FALSE;
    ttDebugClear();
    ttResetModelPools(&modelPools);
    ttResetAnimationPools(&animationPools);
    toggleSettingsMenu();
}

void pasteCurrentInstance() {
    if(!remainingInstanceCountPerModel[currentModelIDInPools])
        return;
    if(--remainingInstanceCountPerModel[currentModelIDInPools])
        playFirstFreeInstanceForModel(currentModelIDInPools);
    else
        playNextModel();
}

void playStressTestModel(Uint32 modelID) {
    char msg[26];
    snprintf(msg, sizeof(msg), "Model %2d/%-2d %s", modelID+1, settings.modelCount, gfsDirNames[nyaFiles[settings.nyaFileIDs[modelID]]].fname);
    ttDebugMsgXY(msg, 0, 0);

    if(remainingInstanceCountPerModel[currentModelIDInPools] && !isAllInstanceSet)
        ttDeactivateInstanceInPool(&animationPools, currentModelIDInPools, currentInstanceID);//TBD
    currentModelIDInSettings = currentModelIDInPools = modelID;
    currentModelAnimationCount = ttGetModelAnimationCount(animationPools, modelID); //TBD
    playFirstFreeInstanceForModel(modelID);
}

// return OK if ok, NG if not good
Sint32 startStressTest() {
    if(getCurrentSettings(&settings)) {
        char msg[40];
        Uint8* nyaFileIDPtr = settings.nyaFileIDs;
        for(Uint32 i = settings.modelCount; i > 0; i--, nyaFileIDPtr++) {
            snprintf(msg, sizeof(msg), "Loading %-2d/%2d", settings.modelCount - i, settings.modelCount);
            slPrint(msg, slLocate(0, 0));
            if(loadModel(*nyaFileIDPtr)) {
                ttDebugMsgLn("Error, select a different configuration");
                ttDebugMsgLn("Press A to continue");
                ttDebugPauseA();
                return NG;
            }
        }

        ttSetAnimatedInstanceCountInPool(&animationPools, settings.modelCount, settings.instanceCountPerModel);
        TTInstancePosition position = {toFIXED(0),toFIXED(90),toFIXED(300), DEGtoANG(0), DEGtoANG(0), DEGtoANG(0), 0};
        for(Uint32 modelID = 0; modelID < settings.modelCount; modelID++) {
            remainingInstanceCountPerModel[modelID] = settings.instanceCountPerModel[modelID];
            for(Uint32 instanceID = 0; instanceID < settings.instanceCountPerModel[modelID]; instanceID++)
                ttSetAnimatedInstancePositionInPool(&animationPools, modelID, instanceID, position);
        }
        currentModelPlayer->playModel(0);
    }

    return OK;
}

void initStressTest() {
    currentModelPlayer = &stressTestPlayer;
    setMenuStressTestMode();
    initSettingsMenu();
    resetStressTest();
}

GameStatus updateStressTest() {
    if(isSettingsMenuEnabled)
        switch(updateSettingsMenu()) {
            case SETTINGS_MENU_START:
                if(startStressTest())
                    resetStressTest();
                break;
            case SETTINGS_MENU_EXIT:
                return GAME_NEXT;
        }
    else {
        if(!isMenuEnabled)
            checkViewerInput();
        switch(updateMenu()) {
            case MENU_GO_TO_NEXT_MODE:
                return GAME_NEXT;
            case MENU_GO_TO_SETTINGS:
                resetStressTest();
        }
        ttUpdateAnimationsInPool(&animationPools);//TBD
    }

    return GAME_CONTINUE;
}

void drawStressTest() {
    slPushMatrix();
    ttDrawAnimatedModelsFromPool(&modelPools, &animationPools);//TBD
    slPopMatrix();
}
