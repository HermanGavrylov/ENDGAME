#ifndef SETTINGS_H
#define SETTINGS_H

#include "raylib.h"

typedef struct GameSettings {
    float volume;
} GameSettings;

void SaveSettings(GameSettings settings);
GameSettings LoadSettings(void);
void DrawSettingsScreen(GameSettings *settings, int *currentState, int previousState);

#endif
