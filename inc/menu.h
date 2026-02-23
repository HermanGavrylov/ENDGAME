#ifndef MENU_H
#define MENU_H

#include "raylib.h"

// Define the states for the whole program
typedef enum GameState { 
    STATE_MENU, 
    STATE_GAMEPLAY, 
    STATE_SETTINGS, 
    STATE_EXIT 
} GameState;

// The main entry point for your menu logic
void DrawMainMenu(GameState *currentState);

#endif
