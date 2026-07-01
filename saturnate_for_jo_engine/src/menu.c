#include <stdio.h>
#include "menu.h"
#include "debug/menu.h"
#include "core/memory.h"
#include "pool_engine/model_loader.h"
#include "pool_engine/animation.h"
#include "debug/print.h"
#include "game_state.h"
#include "stress_test.h"

#define MENU_Y 5
#define HELP_Y 15
#define STATISTICS_Y 24

extern Uint32 _bstart;
extern Uint16 DispPolygons;

TTMenu menu = {
    .y = MENU_Y,
    .menuItemCount = 6,
    .currentItem = 0,
    .maxKeyLength = 17,
    .menuItems = (TTMenuItem[]) {
        {.valueCount = 0, .currentValue = 0, .key = "Resume", .value.texts = NULL},
        {.valueCount = 2, .currentValue = 0, .key = "Display stats",
            .value.texts = (char*[]){ "No", "Yes"}},
        {.valueCount = 2, .currentValue = 0, .key = "Display help",
            .value.texts = (char*[]){ "No", "Yes"}},
        {.valueCount = 6, .currentValue = 0, .key = "Background color",
            .value.texts = (char*[]){ "Black", "Grey", "White", "Green", "Magenta", "Yellow"}},
        {.valueCount = 0, .currentValue = 0, .key = "Go to Stress Test", .value.texts = NULL},
        {.valueCount = 0, .currentValue = 0, .key = "Return to settings", .value.texts = NULL},
    }
};

enum menuItems {RESUME, DISPLAY_STATISTICS, DISPLAY_HELP, BACKGROUND_COLOR, NEXT_MODE, SETTINGS};
enum backGroundColor {BLACK, GREY, WHITE, GREEN, MAGENTA, YELLOW};

Bool isMenuEnabled = FALSE, isHelpDisplayed = FALSE, isStatisticsDisplayed = FALSE;
Uint32 currentBackgroundColor = GREY;

#include "pool_engine/model_loader.h" // see next line
#include "pool_engine/animation.h" // see next line
extern TTModelPools modelPools; // eventually, this line shouldn't be there, replaced by a MACRO getAnimationPools or so from base_viewer
extern TTAnimationPools animationPools; // eventually, this line shouldn't be there, replaced by a MACRO getAnimationPools or so from base_viewer

void displayHelp() {
    if(isHelpDisplayed)
        return;
    isHelpDisplayed = TRUE;

    int y = HELP_Y;
    ttDebugMsgXY("L/R previous/next model", 0, y++);
    ttDebugMsgXY("X/Y previous/next animation", 0, y++);
    ttDebugMsgXY("DPAD rotate", 0, y++);
    ttDebugMsgXY("B + DPAD move around", 0, y++);
    ttDebugMsgXY("C + up/down move up and down", 0, y++);
    ttDebugMsgXY("C + right/left useless rotation", 0, y++);
    ttDebugMsgXY("Z pause animation", 0, y++);
    if(menu.menuItemCount == 6) // means that we are in Stress Test mode
        ttDebugMsgXY("A place instance", 0, y++);
}

void hideHelp() {
    if(!isHelpDisplayed)
        return;
    isHelpDisplayed = FALSE;

    for(int i = HELP_Y; i < HELP_Y + 8; i++)
        ttDebugMsgXY("", 0, i);
}

void displayStatistics() {
    Uint32 y = STATISTICS_Y;
    if(!isStatisticsDisplayed) {
        isStatisticsDisplayed = TRUE;

        slPrint("DispPolygons: ", slLocate(0, y++));
        slPrint("TextureCells:", slLocate(0, y));
        y+=2;
        slPrint("HRAM (  %)      LRAM (  %)", slLocate(13, y++));
        slPrint("3D Models:", slLocate(0, y++));
        slPrint("Animations:", slLocate(0, y++));
    }

    char msg[40];
    Uint32 reservedHRAMForModels, reservedLRAMForModels, reservedHRAMForAnimations, reservedLRAMForAnimations;
    ttGetReservedModelRAMUsage(&modelPools, &reservedHRAMForModels, &reservedLRAMForModels);
    ttGetReservedAnimationRAMUsage(&animationPools, &reservedHRAMForAnimations, &reservedLRAMForAnimations);
    
    y = STATISTICS_Y;
    slPrintFX(DispPolygons << 16, slLocate(13, y++));
    snprintf(msg, sizeof(msg), "%6d/%-6d (%2d%% VRAM)",
            ADDR_OFFSET_TO_CG_COUNT(modelPools.nextFreeTextureDataAddr - TT_VDP1_TEXTURE_POOL),
            ADDR_OFFSET_TO_CG_COUNT(GouraudRAM-CGADDRESS),
            100*(modelPools.nextFreeTextureDataAddr - TT_VDP1_TEXTURE_POOL) / (GouraudRAM-CGADDRESS));
    slPrint(msg, slLocate(13, y));

    Uint32 modelHRAMUsage, modelLRAMUsage, animationHRAMUsage, animationLRAMUsage;
    ttGetCurrentModelRAMUsage(&modelPools, &modelHRAMUsage, &modelLRAMUsage);
    ttGetCurrentAnimationRAMUsage(&modelPools, &animationPools, &animationHRAMUsage, &animationLRAMUsage);
    const Uint32 hRAMUsagePercent = 100 * (modelHRAMUsage + animationHRAMUsage) / (reservedHRAMForModels + reservedHRAMForAnimations);
    const Uint32 lRAMUsagePercent = 100 * (modelLRAMUsage + animationLRAMUsage) / (reservedLRAMForModels + reservedLRAMForAnimations);

    y+=2;
    snprintf(msg, sizeof(msg), "%2d", hRAMUsagePercent);
    slPrint(msg, slLocate(19, y));
    snprintf(msg, sizeof(msg), "%2d", lRAMUsagePercent);
    slPrint(msg, slLocate(35, y++));
    snprintf(msg, sizeof(msg), "%6d/%-6d %6d/%-6d", modelHRAMUsage, reservedHRAMForModels, modelLRAMUsage, reservedLRAMForModels);
    slPrint(msg, slLocate(12, y++));
    snprintf(msg, sizeof(msg), "%6d/%-6d %6d/%-6d", animationHRAMUsage, reservedHRAMForAnimations, animationLRAMUsage, reservedLRAMForAnimations);
    slPrint(msg, slLocate(12, y));
}

void hideStatistics() {
    if(!isStatisticsDisplayed)
        return;
    isStatisticsDisplayed = FALSE;

    char msg[40];
    snprintf(msg, sizeof(msg), "%-39s", "");
    for(int i = STATISTICS_Y; i < STATISTICS_Y+6; i++)
        slPrint(msg, slLocate(0, i));
}

void setBackgroundColor() {
    currentBackgroundColor = menu.menuItems[BACKGROUND_COLOR].currentValue;
    Uint32 color;
    switch(currentBackgroundColor) {
        case BLACK: color = CD_Black; break;
        case GREY: color = CD_DarkWhite; break;
        case WHITE: color = CD_SemiWhite; break;
        case GREEN: color = CD_MediumGreen; break;
        case MAGENTA: color = CD_MediumMagenta; break;
        case YELLOW: color = CD_MediumYellow;
    }
    slBack1ColSet((void*) VDP2_VRAM_A1, color);
}

void setMenuModelViewerMode() {
    menu.menuItemCount = 5;
    menu.menuItems[NEXT_MODE].key = "Go to Stress Test";
}

void setMenuStressTestMode() {
    menu.menuItemCount = 6;
    menu.menuItems[NEXT_MODE].key = "Return to Model Viewer";
}

void toggleMenu() {
    isMenuEnabled = !isMenuEnabled;
    if(!isMenuEnabled)
        ttClearMenu(&menu);
    else
        menu.currentItem = 0;
}

MenuStatus updateMenu() {
    if(isMenuEnabled) {
        ttFocusMenu(&menu);
        if(menu.menuItems[RESUME].currentValue) {
            menu.menuItems[RESUME].currentValue = FALSE;
            toggleMenu();
            return MENU_RESUME;
        }
        if(menu.menuItems[DISPLAY_HELP].currentValue) //"yes" happens to be value 1 and "no" value 0
            displayHelp();
        else
            hideHelp();
        if(menu.menuItems[DISPLAY_STATISTICS].currentValue)  //"yes" happens to be value 1 and "no" value 0
            displayStatistics();
        else
            hideStatistics();
        if(menu.menuItems[BACKGROUND_COLOR].currentValue != currentBackgroundColor)
            setBackgroundColor();
        if(menu.menuItems[NEXT_MODE].currentValue) {
            menu.menuItems[NEXT_MODE].currentValue = FALSE;
            toggleMenu();
            hideHelp();
            hideStatistics();
            return MENU_GO_TO_NEXT_MODE;
        }
        if(menu.menuItems[SETTINGS].currentValue) {
            menu.menuItems[SETTINGS].currentValue = FALSE;
            toggleMenu();
            hideHelp();
            hideStatistics();
            return MENU_GO_TO_SETTINGS;
        }
        return MENU_CONTINUE;
    }
    else
        if(menu.menuItems[DISPLAY_STATISTICS].currentValue)
            displayStatistics();

    return MENU_RESUME;
}
