#include <string.h>
#include "sl_def.h"
#include "debug/menu.h"
#include "debug/print.h"
#include "core/memory.h"
#include "core/model_loader.h"
#include "pool_engine/animation.h"
#include "pool_engine/model_loader.h"
#include "model_viewer.h"
#include "stress_test_settings.h"

extern int snprintf(char *str, size_t size, const char *format, ...);

#define MENU_Y 5

extern uint32_t __dtors_end;

bool isSettingsMenuEnabled;
uint32_t reservedHRAMUsage, reservedLRAMUsage;

TTNYAFileMetadata nyaMetaDataPerModel[MAX_MODEL_FILE_COUNT];
TTMOTFileMetadata motMetaDataPerModel[MAX_MODEL_FILE_COUNT];

TTMenu settingsMenu = {
    .y = MENU_Y,
    .menuItemCount = 0, //set to modelFileCount at runtime
    .currentItem = 0,
    .maxKeyLength = 17,
    .menuItems = (TTMenuItem[MAX_MODEL_FILE_COUNT + 3]){0}
};

// You can make it return "error" if RAM above 100%
int32_t getCurrentSettings(Settings* settings) {
    uint32_t modelCount = 0;
    uint8_t* nyaFileIDPtr = settings->nyaFileIDs;
    uint8_t* instanceCountPerModelPtr = settings->instanceCountPerModel;
    TTMenuItem* menuItemPtr = settingsMenu.menuItems;
    for(uint32_t modelID = 0; modelID < modelFileCount; modelID++, menuItemPtr++) {
        if(menuItemPtr->value.integer > 0) {
            modelCount++;
            *nyaFileIDPtr = modelID;
            nyaFileIDPtr++;
            *instanceCountPerModelPtr = menuItemPtr->value.integer;
            instanceCountPerModelPtr++;
        }
    }
    return settings->modelCount = modelCount; // should rather return a bad code when 0 or too many models are selected
}

void initSettingsStatistics() {
    ttDebugMsgXY("", 0, 1);
    char msg[40];

    TTNYAFileMetadata* nyaMetaDataPtr = nyaMetaDataPerModel;
    TTMOTFileMetadata* motMetaDataPtr = motMetaDataPerModel;
    for(int modelID = 0; modelID < modelFileCount; nyaMetaDataPtr++, motMetaDataPtr++, modelID++) {
                                                    // vramCGCountUsagePtr++, modelHRAMUsagePtr++, modelLRAMUsagePtr++) {
        snprintf(msg, sizeof(msg), "Loading %-2d/%-39d", modelID, modelFileCount);
        slPrint(msg, slLocate(0, 0));
        char* nyaFileName = GFS_IdToName(nyaFiles[modelID]);
        ttLoadNYAMetadata(nyaFileName, nyaMetaDataPtr);
        int32_t motFileGFSID = getCorrespondingMOTFileGFSID(nyaFileName);
        if(motFileGFSID > 0) {
            ttLoadMOTMetadata(GFS_IdToName(motFileGFSID), motMetaDataPtr);
        }
    }
    ttGetReservedModelRAMUsage(&modelPools, &reservedHRAMUsage, &reservedLRAMUsage);
    uint32_t animationReservedHRAM, animationReservedLRAM;
    ttGetReservedAnimationRAMUsage(&animationPools, &animationReservedHRAM, &animationReservedLRAM);
    reservedHRAMUsage += animationReservedHRAM;
    reservedLRAMUsage += animationReservedLRAM;
}

void initSettingsMenu() {
    initSettingsStatistics();

    settingsMenu.menuItemCount = modelFileCount + 3;
    TTMenuItem* menuItem = settingsMenu.menuItems;
    for(uint32_t i = 0; i < modelFileCount; i++, menuItem++) {
        menuItem->valueCount = -1; //means we select a uint8_t
        menuItem->key = gfsDirNames[nyaFiles[i]].fname;
    }
    *menuItem++ = (TTMenuItem) {.valueCount = 0, .currentValue = 0, .key = "Reset", .value.texts = NULL};
    *menuItem++ = (TTMenuItem) {.valueCount = 0, .currentValue = 0, .key = "Start stress test", .value.texts = NULL};
    *menuItem = (TTMenuItem) {.valueCount = 0, .currentValue = 0, .key = "Return to model viewer", .value.texts = NULL};
}

void resetStressTestSettings() {
    TTMenuItem* menuItem = settingsMenu.menuItems;
    for(uint32_t i = 0; i < modelFileCount; i++, menuItem++)
        menuItem->value.integer = 0;
}

void toggleSettingsMenu() {
    isSettingsMenuEnabled = !isSettingsMenuEnabled;
    if(!isSettingsMenuEnabled) {
        ttClearMenu(&settingsMenu);
        char emptyMsg[40];
        snprintf(emptyMsg, sizeof(emptyMsg), "%-39s", "");
        for(int32_t y = 39; y >= 0; y--)
            slPrint(emptyMsg, slLocate(0,y));
    }
    else {
        settingsMenu.currentItem = 0;
        ttDebugMsgXY("Adjust instance counts per model", 0, 0);
        ttDebugMsgXY("then select \"Start stress test\"", 0, 1);
        ttDebugMsgXY(" Model                   Instance Count", 0, MENU_Y-2);
    }
}

void updateStats() {
    //calculate everything first, then display
    uint32_t currentModelVRAMCellUsage;
    uint32_t currentModelTotalRAMUsage;
    uint32_t currentAnimationTotalRAMUsage;

    uint32_t totalHRAMUsage = 0;
    uint32_t totalLRAMUsage = 0;
    uint32_t totalVRAMCellUsage = 0;

    TTMenuItem* menuItem = settingsMenu.menuItems;
    TTNYAFileMetadata* nyaMetaDataPtr = nyaMetaDataPerModel;
    TTMOTFileMetadata* motMetaDataPtr = motMetaDataPerModel;
    for(uint32_t modelID = 0; modelID < modelFileCount; modelID++, menuItem++, nyaMetaDataPtr++, motMetaDataPtr++) {
        uint32_t hRAMModelUsage, lRAMModelUsage;
        ttGetModelRAMUsageFromMetaData(&modelPools, &hRAMModelUsage, &lRAMModelUsage, nyaMetaDataPtr);
        if(menuItem->value.integer) {
            totalVRAMCellUsage += nyaMetaDataPtr->vramCGCount;
            uint32_t hRAMAnimationUsage, lRAMAnimationUsage;
            totalHRAMUsage += hRAMModelUsage;
            totalLRAMUsage += lRAMModelUsage;
            ttGetAnimationRAMUsageFromMetaData(&animationPools, &hRAMAnimationUsage, &lRAMAnimationUsage, motMetaDataPtr, menuItem->value.integer);
            totalHRAMUsage += hRAMAnimationUsage;
            totalLRAMUsage += lRAMAnimationUsage;

            if(modelID == settingsMenu.currentItem) {//this model is currently selected
                currentModelVRAMCellUsage = nyaMetaDataPtr->vramCGCount;
                currentModelTotalRAMUsage = hRAMModelUsage + lRAMModelUsage;
                currentAnimationTotalRAMUsage = hRAMAnimationUsage + lRAMAnimationUsage;
            }
        } else if(modelID == settingsMenu.currentItem) {//this model is currently selected
            currentModelVRAMCellUsage = nyaMetaDataPtr->vramCGCount;
            currentModelTotalRAMUsage = hRAMModelUsage + lRAMModelUsage;
            uint32_t hRAMAnimationUsage, lRAMAnimationUsage;
            ttGetAnimationRAMUsageFromMetaData(&animationPools, &hRAMAnimationUsage, &lRAMAnimationUsage, motMetaDataPtr, 1);
            currentAnimationTotalRAMUsage = hRAMAnimationUsage + lRAMAnimationUsage;
        }
    }

    //currently selected line stats
    char msg[40];
    if(settingsMenu.currentItem < modelFileCount) {
        snprintf(msg, sizeof(msg), "Model cost: %6d  VRAM cost: %6d", currentModelTotalRAMUsage, currentModelVRAMCellUsage);
        slPrint(msg, slLocate(0, 24));
        snprintf(msg, sizeof(msg), "Animation cost: %6d", currentAnimationTotalRAMUsage);
        slPrint(msg, slLocate(0, 25));
    }
    else {
        snprintf(msg, sizeof(msg), "Model cost: %6s  VRAM cost: %6s", "", "");
        slPrint(msg, slLocate(0, 24));
        snprintf(msg, sizeof(msg), "Animation cost: %6s", "");
        slPrint(msg, slLocate(0, 25));
    }

    //total stats
    snprintf(msg, sizeof(msg), "Total HRAM: %8d/%-8d (%2d%%)", totalHRAMUsage, reservedHRAMUsage, 100 * totalHRAMUsage / reservedHRAMUsage);
    slPrint(msg, slLocate(0, 27));
    snprintf(msg, sizeof(msg), "Total LRAM: %8d/%-8d (%2d%%)%10s", totalLRAMUsage, reservedLRAMUsage, 100 * totalLRAMUsage / reservedLRAMUsage, "");
    slPrint(msg, slLocate(0, 28));
    const uint32_t usableCells = ADDR_OFFSET_TO_CG_COUNT(GouraudRAM-CGADDRESS);
    snprintf(msg, sizeof(msg), "Total VRAM: %8d/%-8d (%2d%%)", totalVRAMCellUsage, usableCells, 100 * totalVRAMCellUsage / usableCells);
    slPrint(msg, slLocate(0, 29));
}

SettingsMenuStatus updateSettingsMenu() {
    if(isSettingsMenuEnabled) {
        ttFocusMenu(&settingsMenu);
        updateStats();

        if(settingsMenu.menuItems[modelFileCount].currentValue) { //Reset
            settingsMenu.menuItems[modelFileCount].currentValue = FALSE;
            resetStressTestSettings();
        }
        if(settingsMenu.menuItems[modelFileCount+1].currentValue) { //Start stress test
            settingsMenu.menuItems[modelFileCount+1].currentValue = FALSE;
            toggleSettingsMenu();
            return SETTINGS_MENU_START;
        }
        if(settingsMenu.menuItems[modelFileCount+2].currentValue) { //Exit stress test
            settingsMenu.menuItems[modelFileCount+2].currentValue = FALSE;
            toggleSettingsMenu();
            return SETTINGS_MENU_EXIT;
        }
    }

    return SETTINGS_MENU_CONTINUE;
}
