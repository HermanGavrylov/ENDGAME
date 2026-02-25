#ifndef MENU_H
#define MENU_H

#include "raylib.h"

// Define the states for the whole program
typedef enum GameState { 
    STATE_MENU,
    STATE_PAUSE, 
    STATE_GAMEPLAY, 
    STATE_SETTINGS, 
    STATE_EXIT 
} GameState;

void DrawMainMenu(GameState *currentState);
void DrawPauseMenu(bool *isPaused, GameState *currentState);

#endif
