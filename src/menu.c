#include <raylib.h>
#include "menu.h"

// Implementation of the button helper
bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, rect);
    
    // Change cursor if hovering
    if (isHovering) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    // Draw button body
    DrawRectangleRec(rect, isHovering ? hoverColor : baseColor);
    DrawRectangleLinesEx(rect, 2, DARKGRAY);

    // Center text
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, rect.x + (rect.width/2 - textWidth/2), rect.y + (rect.height/2 - fontSize/2), fontSize, BLACK);

    return (isHovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

int main() {
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "ENDGAME");
    
    GameState currentState = STATE_MENU;
    
    // Layout definitions
    Rectangle playBtn = { screenWidth/2 - 100, 160, 200, 50 };
    Rectangle settingsBtn = { 300, 220, 200, 50 };
    Rectangle exitBtn = { screenWidth/2 - 100, 280, 200, 50 };

    SetTargetFPS(60);

    while (currentState != STATE_EXIT && !WindowShouldClose()) {
        // Update Logic (Global)
        if (IsKeyPressed(KEY_TAB)) currentState = STATE_MENU; // Emergency return

        // Drawing
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch(currentState) {
                case STATE_MENU:
                    DrawText("ENDGAME", screenWidth/2 - 100, 80, 40, MAROON);
                    
                    if (DrawButton(playBtn, "START GAME", LIGHTGRAY, SKYBLUE)) {
                        currentState = STATE_GAMEPLAY;
                    }
                    if (DrawButton(settingsBtn, "SETTINGS", LIGHTGRAY, GOLD)) {
                        currentState = STATE_SETTINGS;
                    }
                    if (DrawButton(exitBtn, "QUIT", LIGHTGRAY, RED)) {
                        currentState = STATE_EXIT;
                    }
                    break;

                case STATE_GAMEPLAY:
                    DrawRectangle(0, 0, screenWidth, screenHeight, DARKBLUE);
                    DrawText("GAME", 230, 200, 30, RAYWHITE);
                    DrawText("Press TAB to return to Menu", 10, 10, 20, LIGHTGRAY);
                    break;

                case STATE_SETTINGS:
                    DrawRectangle(0, 0, screenWidth, screenHeight, DARKBLUE);
                    DrawText("SETTINGS HERE", 230, 200, 30, RAYWHITE);
                    DrawText("Press TAB to return to Menu", 10, 10, 20, LIGHTGRAY);
                    break;
                
                default: break;
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
