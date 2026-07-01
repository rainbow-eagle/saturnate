#ifndef TT_DEBUG_MENU_H
#define TT_DEBUG_MENU_H

#include <sl_def.h>

typedef struct {
    int16_t valueCount;
    int16_t currentValue; // if > 0 then point to an index in value.texts. if == 0 then is a button that can be pressed (e.g. quit game). if < 0, then is the choice of a uint8_t in value.integer
    char* key; //key[textLength]
    union {
        char** texts; //values[valueCount][textLength] //if valueCount == 0 then currentValue is true when button is pressed (pressed/non pressed situation)
        uint8_t integer;
    } value;
} TTMenuItem;

typedef struct {
    uint8_t y;
    uint8_t menuItemCount;
    uint8_t currentItem;
    uint8_t maxKeyLength; //SpaceAllocated to the key string. Space allocated to the value is (32 - maxKeyLength)
    TTMenuItem* menuItems; //menuItems[menuItemCount]
} TTMenu;

void ttFocusMenu(TTMenu* menu);
void ttClearMenu(TTMenu* menu);

#endif //TT_DEBUG_MENU_H