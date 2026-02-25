#include "raylib.h"
#include "menu.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Raylib Modular Menu System");
    SetExitKey(KEY_NULL); // SO ESC will no longer close the window and I can use it for menu
    InitAudioDevice(); // Init music

    GameState currentState = STATE_MENU;
    GameState previousState = STATE_MENU; // Tracks where we came from
    bool isPaused = false;                // Tracks if the pause overlay is active
    
    SetTargetFPS(60);

    // Main Game Loop
    while (currentState != STATE_EXIT && !WindowShouldClose()) {
        
        // --- DRAWING ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

            switch(currentState) {
                
                case STATE_MENU:
                    DrawMainMenu(&currentState);
                    
                    // If user clicked START, set to Gameplay AND Pause immediately
                    if (currentState == STATE_GAMEPLAY) {
                        isPaused = true; 
                        previousState = STATE_MENU;
                    }
                    // If user clicked SETTINGS, remember we came from Main Menu
                    if (currentState == STATE_SETTINGS) {
                        previousState = STATE_MENU;
                    }
                    break;

                case STATE_SETTINGS:
                    DrawText("SETTINGS SCREEN", 280, 150, 30, DARKGRAY);
                    DrawText("Volume: [ ||||||||-- ]", 280, 220, 20, GRAY);
                    DrawText("Press ESC to go back", 10, 10, 20, LIGHTGRAY);
                    
                    // Go back to whichever screen we were on before
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        currentState = previousState;
                    }
                    break;

                case STATE_GAMEPLAY:
                    //Draw the Game World (stays visible behind pause)
                    DrawCircle(screenWidth/2, screenHeight/2, 60, MAROON);
                    DrawText("GAME WORLD ACTIVE", 10, 40, 20, DARKGRAY);
                    DrawText("Press ESC to pause", 10, 10, 20, GRAY);

                    //Input Handling
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        isPaused = !isPaused;
                    }

                    //Conditional Pause Menu
                    if (isPaused) {
                        DrawPauseMenu(&isPaused, &currentState);
                        
                        // If user enters settings from Pause, remember we are in Gameplay
                        if (currentState == STATE_SETTINGS) {
                            previousState = STATE_GAMEPLAY;
                        }
                    }
                    break;
                
                default: break;
            }

        EndDrawing();
    }
    CloseWindow();

    return 0;
}
