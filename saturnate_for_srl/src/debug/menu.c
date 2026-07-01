#include <string.h>
#include <stdio.h>
#include <sl_def.h>
#include "../debug/menu.h"

extern int snprintf(char *str, size_t size, const char *format, ...);

#define TT_MENU_MAX_LINE_LENGTH 40
#define TT_MENU_HARDCODED_USED_SPACE 11 //Nb of lost characters in the formatting of lines (spaces, "[,],<,>"...)
#define TT_MENU_USABLE_SPACE (TT_MENU_MAX_LINE_LENGTH - TT_MENU_HARDCODED_USED_SPACE)

void ttFocusMenu(TTMenu* menu) {
    char msg[40];
    TTMenuItem* menuItem = menu->menuItems;
    uint32_t y = menu->y;
    uint32_t indexCurrentItem = menu->menuItemCount - menu->currentItem;
    for(uint32_t i = menu->menuItemCount; i > 0; i--, menuItem++, y++) {
        if(i == indexCurrentItem)
            if(menuItem->valueCount > 0)
                snprintf(msg, sizeof(msg), "[%-*s]    < %-*s >", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, menuItem->value.texts[menuItem->currentValue]);
            else if(menuItem->valueCount == 0) // Key only - no value
                snprintf(msg, sizeof(msg), "[%-*s]     %-*s  ", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, "");
            else // value is a uint8_t
                snprintf(msg, sizeof(msg), "[%-*s]    < %-*d >", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, menuItem->value.integer);
        else
            if(menuItem->valueCount > 0)
                snprintf(msg, sizeof(msg), " %-*s       %-*s  ", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, menuItem->value.texts[menuItem->currentValue]);    
            else if(menuItem->valueCount == 0) // Key only - no value
                snprintf(msg, sizeof(msg), " %-*s       %-*s  ", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, "");
            else // value is a uint8_t
                snprintf(msg, sizeof(msg), " %-*s       %-*d  ", menu->maxKeyLength, menuItem->key, TT_MENU_USABLE_SPACE-menu->maxKeyLength, menuItem->value.integer);
        slPrint(msg, slLocate(0, y));
    }

    if(Per_Connect1) {
        uint16_t data = Smpc_Peripheral[0].push;
        if(!(data & PER_DGT_KU))
            menu->currentItem = (menu->currentItem == 0)? menu->menuItemCount-1 : menu->currentItem-1;
        if(!(data & PER_DGT_KD))
            menu->currentItem = (menu->currentItem == menu->menuItemCount-1)? 0 : menu->currentItem+1;
        if(!(data & PER_DGT_KL)) {
            menuItem = menu->menuItems + menu->currentItem;
            if(menuItem->valueCount > 0) // finite set of values
                menuItem->currentValue = (menuItem->currentValue == 0)? menuItem->valueCount-1 : menuItem->currentValue-1;
            else if(menuItem->valueCount < 0) // select uint8_t
                menuItem->value.integer--;
        }
        if(!(data & PER_DGT_KR)) {
            menuItem = menu->menuItems + menu->currentItem;
            if(menuItem->valueCount > 0)
                menuItem->currentValue = (menuItem->currentValue == menuItem->valueCount-1)? 0 : menuItem->currentValue+1;
            else if(menuItem->valueCount < 0) // select uint8_t
                menuItem->value.integer++;
        }
        if(!(data & PER_DGT_TA)) {
            menuItem = menu->menuItems + menu->currentItem;
            if(menuItem->valueCount == 0) // Key only, so it's just a boolean "pressed" or "not pressed"
                menuItem->currentValue = TRUE;
        }
    }
}

void ttClearMenu(TTMenu* menu) {
    uint32_t y = menu->y;
    char emptyMsg[40];
    snprintf(emptyMsg, sizeof(emptyMsg), "%-39s", "");
    for(uint32_t i = menu->menuItemCount; i > 0; i--, y++) {
        slPrint(emptyMsg, slLocate(0,y));
    }
}
