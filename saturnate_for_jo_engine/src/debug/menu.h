#ifndef TT_DEBUG_MENU_H
#define TT_DEBUG_MENU_H

#include <jo/sgl_prototypes.h>

typedef struct {
    Sint16 valueCount;
    Sint16 currentValue; // if > 0 then point to an index in value.texts. if == 0 then is a button that can be pressed (e.g. quit game). if < 0, then is the choice of a Uint8 in value.integer
    char* key; //key[textLength]
    union {
        char** texts; //values[valueCount][textLength] //if valueCount == 0 then currentValue is true when button is pressed (pressed/non pressed situation)
        Uint8 integer;
    } value;
} TTMenuItem;

typedef struct {
    Uint8 y;
    Uint8 menuItemCount;
    Uint8 currentItem;
    Uint8 maxKeyLength; //SpaceAllocated to the key string. Space allocated to the value is (32 - maxKeyLength)
    TTMenuItem* menuItems; //menuItems[menuItemCount]
} TTMenu;

void ttFocusMenu(TTMenu* menu);
void ttClearMenu(TTMenu* menu);

#endif //TT_DEBUG_MENU_H