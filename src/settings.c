#include "settings.h"
#include "menu.h"
#include <stdio.h>

void SaveSettings(GameSettings settings) {
    SaveFileData("config.bin", &settings, sizeof(GameSettings));
}

GameSettings LoadSettings(void) {
    GameSettings settings = { 0.5f }; // Default to 50% volume
    
    if (FileExists("config.bin")) {
        int bytesRead = 0;
        unsigned char *data = LoadFileData("config.bin", &bytesRead);
        
        if (data != NULL && bytesRead == sizeof(GameSettings)) {
            settings = *(GameSettings *)data;
            UnloadFileData(data);
        }
    }
    
    SetMasterVolume(settings.volume);
    return settings;
}

void DrawSettingsScreen(GameSettings *settings, int *currentState, int previousState) {
    Vector2 mousePos = GetMousePosition();
    
    // Slider dimensions and position
    Rectangle volumeBar = { 200, 200, 400, 30 };
    
    // KEYBOARD CONTROLS
    // Change volume by 1% per frame if key is held
    if (IsKeyDown(KEY_RIGHT)) settings->volume += 0.01f;
    if (IsKeyDown(KEY_LEFT)) settings->volume -= 0.01f;

    // MOUSE DRAGGING LOGIC
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, volumeBar)) {
            settings->volume = (mousePos.x - volumeBar.x) / volumeBar.width;
        }
    }

    if (settings->volume < 0.0f) settings->volume = 0.0f;
    if (settings->volume > 1.0f) settings->volume = 1.0f;
    
    SetMasterVolume(settings->volume);

    ClearBackground(RAYWHITE);
    
    DrawText("SETTINGS", 300, 80, 40, DARKGRAY);

    // Volume Percentage Text
    int volPercent = (int)(settings->volume * 100.0f + 0.5f);
    DrawText(TextFormat("Master Volume: %i%%", volPercent), 200, 160, 20, BLACK);
    
    // Draw Slider Background
    DrawRectangleRec(volumeBar, LIGHTGRAY);
    
    // Draw Filled Part
    DrawRectangle(volumeBar.x, volumeBar.y, (int)(volumeBar.width * settings->volume), volumeBar.height, MAROON);
    
    // Draw a "Knob/Handle" at the current volume position
    DrawCircle(volumeBar.x + (volumeBar.width * settings->volume), volumeBar.y + volumeBar.height/2, 12, DARKGRAY);
    
    // Draw Slider Outline
    DrawRectangleLinesEx(volumeBar, 2, BLACK);

    // UI Instructions
    DrawText("Use Arrows or Mouse to Adjust", 200, 250, 15, GRAY);
    
    DrawRectangle(180, 330, 440, 60, Fade(LIGHTGRAY, 0.3f));
    DrawText("[S] SAVE SETTINGS", 200, 350, 20, DARKGREEN);
    DrawText("[ESC] BACK", 450, 350, 20, MAROON);

    if (IsKeyPressed(KEY_S)) {
        SaveSettings(*settings);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        *currentState = previousState;
    }
}
