#ifndef VIEWER_MENU_H
#define VIEWER_MENU_H

#include "debug/menu.h"

typedef enum {
    MENU_CONTINUE,
    MENU_RESUME,
    MENU_GO_TO_NEXT_MODE,
    MENU_GO_TO_SETTINGS
} MenuStatus;

extern bool isMenuEnabled;

void setMenuModelViewerMode();
void setMenuStressTestMode();

void toggleMenu();
MenuStatus updateMenu();

#endif //VIEWER_MENU_H