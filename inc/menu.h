#ifndef MENU_H
#define MENU_H

typedef enum {
    STATE_MENU,
    STATE_CHARSELECT,
    STATE_SETTINGS,
    STATE_GAMEOVER,
    STATE_GAMEPLAY,
    STATE_EXIT
} MenuSystemState;

void DrawMainMenu(MenuSystemState *state);
void DrawPauseMenu(bool *paused, MenuSystemState *state);
void DrawGameOver(MenuSystemState *currentState, GameState *gs);
void DrawCharSelect(MenuSystemState *currentState, CharClass *selected);

#endif
