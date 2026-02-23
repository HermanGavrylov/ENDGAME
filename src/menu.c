#include "raylib.h"
#include "menu.h"

int main() {
    InitWindow(800, 450, "Modular Raylib Menu");
    GameState currentState = STATE_MENU;
    SetTargetFPS(60);

    while (currentState != STATE_EXIT && !WindowShouldClose()) {
        // Toggle menu with TAB
        if (IsKeyPressed(KEY_TAB)) {
            currentState = (currentState == STATE_GAMEPLAY);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch(currentState) {
                // Calling main_menu
                case STATE_MENU:
                    DrawMainMenu(&currentState);
                    break;
                // Calling settings
                case STATE_SETTINGS:
                    DrawText("SETTINGS SCREEN", 280, 200, 20, GRAY);
                    DrawText("Press TAB to go back", 10, 10, 20, DARKGRAY);
                    if (IsKeyPressed(KEY_TAB)) currentState = STATE_MENU;
                    break;
                // Calling gameolay
                case STATE_GAMEPLAY:
                    DrawText("Press TAB to open menu", 10, 10, 20, DARKGRAY);
                    if (IsKeyPressed(KEY_TAB)) currentState = STATE_MENU;
                    break;
                
                default: break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
