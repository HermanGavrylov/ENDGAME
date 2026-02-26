#include "header.h"
#include "settings.h"
#include "menu.h"
#include <stdio.h>

// Extern declaration to access the audio update from menu.c
// If you put UpdateMenuAudio in menu.h, you don't need the 'extern' line.
extern void UpdateMenuAudio(void); 

void SaveSettings(GameSettings settings) {
    SaveFileData("config.bin", &settings, sizeof(GameSettings));
}

GameSettings LoadSettings(void) {
    GameSettings settings = { 0.5f }; 
    
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
    // --- PERSISTENT MUSIC UPDATE ---
    UpdateMenuAudio(); 

    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    Vector2 mousePos = GetMousePosition();

    static Texture2D menuBg;
    static bool loaded = false;
    if (!loaded) {
        menuBg = LoadTexture("resource/images/background_image.jpg");
        loaded = true;
    }

    ClearBackground(BLACK);

    if (menuBg.id > 0) {
        DrawTexturePro(menuBg, 
            (Rectangle){ 0, 0, (float)menuBg.width, (float)menuBg.height },
            (Rectangle){ 0, 0, sw, sh },
            (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else {
        DrawRectangle(0, 0, (int)sw, (int)sh, MAGENTA);
    }

    DrawRectangle(0, 0, (int)sw, (int)sh, (Color){ 0, 0, 0, 100 });

    int panelW = 300;
    int panelH = 240;
    int px = (int)(sw/2 - panelW/2);
    int py = (int)(sh/2 - panelH/2);

    DrawRectangle(px, py, panelW, panelH, (Color){ 10, 10, 20, 200 });
    DrawRectangleLines(px, py, panelW, panelH, (Color){ 100, 90, 70, 255 });

    const char *title = "System Settings";
    int tw = MeasureText(title, 16);
    DrawText(title, sw/2 - tw/2, py + 8, 16, (Color){ 255, 220, 120, 255 });

    Rectangle volumeBar = { (float)px + 20, (float)py + 80, (float)panelW - 40, 12 };
    
    // Smooth volume adjustment with keys
    if (IsKeyDown(KEY_RIGHT)) settings->volume += 0.01f;
    if (IsKeyDown(KEY_LEFT)) settings->volume -= 0.01f;

    // Mouse control for volume slider
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, (Rectangle){volumeBar.x, volumeBar.y - 10, volumeBar.width, volumeBar.height + 20})) {
            settings->volume = (mousePos.x - volumeBar.x) / volumeBar.width;
        }
    }

    // Clamp volume values
    if (settings->volume < 0.0f) settings->volume = 0.0f;
    if (settings->volume > 1.0f) settings->volume = 1.0f;
    
    // Apply volume changes instantly
    SetMasterVolume(settings->volume);

    int volPercent = (int)(settings->volume * 100.0f + 0.5f);
    DrawText(TextFormat("Master Volume: %i%%", volPercent), px + 20, py + 60, 14, (Color){ 230, 220, 200, 255 });
    
    // Draw Slider Bar
    DrawRectangleRec(volumeBar, (Color){ 30, 30, 30, 255 }); 
    DrawRectangle((int)volumeBar.x, (int)volumeBar.y, (int)(volumeBar.width * settings->volume), (int)volumeBar.height, (Color){ 200, 160, 40, 255 }); 
    DrawRectangleLines((int)volumeBar.x, (int)volumeBar.y, (int)volumeBar.width, (int)volumeBar.height, (Color){ 80, 75, 60, 255 });
    
    // Draw Slider Handle
    DrawCircle((int)(volumeBar.x + (volumeBar.width * settings->volume)), (int)(volumeBar.y + volumeBar.height/2), 8, (Color){ 255, 220, 120, 255 });

    const char *instr = "Use <- -> or Mouse";
    int iw = MeasureText(instr, 11);
    DrawText(instr, sw/2 - iw/2, py + 110, 11, (Color){ 160, 155, 140, 255 });

    DrawText("[S] SAVE & BACK", px + 20, py + panelH - 45, 12, (Color){ 120, 220, 120, 255 });
    DrawText("[ESC] CANCEL", px + panelW - MeasureText("[ESC] CANCEL", 12) - 20, py + panelH - 45, 12, (Color){ 220, 80, 80, 255 });

    const char *hint = "ESC - close";
    int hw = MeasureText(hint, 11);
    DrawText(hint, sw/2 - hw/2, py + panelH - 18, 11, (Color){ 120, 120, 120, 255 });

    if (IsKeyPressed(KEY_S)) {
        SaveSettings(*settings);
        *currentState = previousState;
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
        *currentState = previousState;
    }
}
