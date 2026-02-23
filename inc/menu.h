#ifndef MENU_H
#define MENU_H

#include <raylib.h>

typedef enum GameState { 
    STATE_MENU, 
    STATE_GAMEPLAY, 
    STATE_SETTINGS, 
    STATE_EXIT 
} GameState;

bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor);

#endif
