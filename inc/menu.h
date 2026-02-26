#ifndef MENU_H
#define MENU_H

#define PLAYER_NAME_LEN 32


typedef enum {
    STATE_MENU,
    STATE_CHARSELECT,
    STATE_SETTINGS,
    STATE_SCOREBOARD,
    STATE_GAMEOVER,
    STATE_GAMEPLAY,
    STATE_EXIT
} MenuSystemState;

void DrawMainMenu(MenuSystemState *state);
void DrawPauseMenu(bool *paused, MenuSystemState *state);
extern char gPlayerName[PLAYER_NAME_LEN];
void DrawGameOver(MenuSystemState *currentState, GameState *gs);
void DrawCharSelect(MenuSystemState *currentState, CharClass *selected);

#endif
