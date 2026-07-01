#ifndef STRESS_TEST_SETTINGS_H
#define STRESS_TEST_SETTINGS_H

#include "base_viewer.h"

typedef enum {
    SETTINGS_MENU_CONTINUE,
    SETTINGS_MENU_START,
    SETTINGS_MENU_EXIT
} SettingsMenuStatus;

extern bool isSettingsMenuEnabled;

void initSettingsMenu();
void toggleSettingsMenu();
SettingsMenuStatus updateSettingsMenu();

int32_t getCurrentSettings(Settings* settings);

#endif //STRESS_TEST_SETTINGS_H