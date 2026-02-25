#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum {
    STATE_MENU,
    STATE_SETTINGS,
    STATE_GAMEPLAY,
    STATE_EXIT
} MenuSystemState; 

void DrawMainMenu(MenuSystemState *state);
void DrawPauseMenu(bool *paused, MenuSystemState *state);

#endif
