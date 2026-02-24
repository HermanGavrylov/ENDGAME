#include "raylib.h"
#include "menu.h"

// Private helper function: only visible inside this file
static bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, rect);
    
    if (isHovering) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

    DrawRectangleRec(rect, isHovering ? hoverColor : baseColor);
    DrawRectangleLinesEx(rect, 2, DARKGRAY);

    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, rect.x + (rect.width/2 - textWidth/2), rect.y + (rect.height/2 - fontSize/2), fontSize, BLACK);

    return (isHovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

void DrawPauseMenu(bool *isPaused, GameState *currentState) {
    // 1. Dim the background (0.5f is 50% transparency)
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    // Define all rectangles first
    float centerX = GetScreenWidth() / 2.0f - 100;
    Rectangle resumeBtn   = { centerX, 180, 200, 50 };
    Rectangle settingsBtn = { centerX, 250, 200, 50 }; // Added this missing line!
    Rectangle menuBtn     = { centerX, 320, 200, 50 };

    DrawText("PAUSED", GetScreenWidth()/2 - MeasureText("PAUSED", 30)/2, 100, 30, WHITE);

    // 2. Button Logic
    if (DrawButton(resumeBtn, "CONTINUE", LIGHTGRAY, GREEN)) {
        *isPaused = false; 
    }

    if (DrawButton(settingsBtn, "SETTINGS", LIGHTGRAY, GOLD)) {
        // We usually don't want the game 'unpaused' while we are in settings
        // But we switch the screen state
        *currentState = STATE_SETTINGS;
    }   

    if (DrawButton(menuBtn, "MAIN MENU", LIGHTGRAY, RED)) {
        *isPaused = false;      
        *currentState = STATE_MENU; 
    }
}
