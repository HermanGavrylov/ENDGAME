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

void DrawPauseMenu(bool *isPaused, MenuSystemState *currentState) {
    //Dim the background (50% transparency)
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    // Define all menu buttons
    float centerX = GetScreenWidth() / 2.0f - 100;
    Rectangle resumeBtn   = { centerX, 180, 200, 50 };
    Rectangle settingsBtn = { centerX, 250, 200, 50 };
    Rectangle menuBtn     = { centerX, 320, 200, 50 };

    DrawText("PAUSED", GetScreenWidth()/2 - MeasureText("PAUSED", 30)/2, 100, 30, WHITE);

    //Button Logic
    if (DrawButton(resumeBtn, "CONTINUE", LIGHTGRAY, GREEN)) {
        *isPaused = false; 
    }

    if (DrawButton(settingsBtn, "SETTINGS", LIGHTGRAY, GOLD)) {
        *currentState = STATE_SETTINGS;
    }   

    if (DrawButton(menuBtn, "MAIN MENU", LIGHTGRAY, RED)) {
        *isPaused = false;      
        *currentState = STATE_MENU; 
    }
}
