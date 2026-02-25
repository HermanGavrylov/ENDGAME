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

void DrawMainMenu(MenuSystemState *currentState) {
    float screenW = GetScreenWidth();
    float screenH = GetScreenHeight();
    
    // Define all menu buttons
    Rectangle playBtn = { screenW/2 - 100, screenH/2 - 60, 200, 50 };
    Rectangle settingsBtn = { screenW/2 - 100, screenH/2 + 10, 200, 50 };
    Rectangle exitBtn = { screenW/2 - 100, screenH/2 + 80, 200, 50 };

    // Name of the game
    DrawText("ENDGAME", screenW/2 - MeasureText("ENDGAME", 40)/2, screenH/2 - 150, 40, DARKGRAY);

    // Buttons design and destinations
    if (DrawButton(playBtn, "START", LIGHTGRAY, SKYBLUE)) {
        *currentState = STATE_GAMEPLAY;
    }
    
    if (DrawButton(settingsBtn, "SETTINGS", LIGHTGRAY, GOLD)) {
        *currentState = STATE_SETTINGS;
    }

    if (DrawButton(exitBtn, "QUIT", LIGHTGRAY, RED)) {
        *currentState = STATE_EXIT;
    }
}
