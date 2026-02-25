#include "header.h"
#include "menu.h"

static bool DrawButton(Rectangle rect, const char* text, Color hoverTint) {
    Vector2 mouse = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mouse, rect);
    
    Color baseBg = { 30, 30, 40, 140 };
    Color currBg = isHovering ? (Color){ 20, 60, 20, 160 } : baseBg;
    Color borderColor = isHovering ? hoverTint : (Color){ 70, 65, 55, 255 };

    DrawRectangleRec(rect, currBg);
    DrawRectangleLinesEx(rect, 1, borderColor);

    int fontSize = 14;
    int textWidth = MeasureText(text, fontSize);
    Color textColor = isHovering ? hoverTint : (Color){ 230, 220, 200, 255 };
    
    DrawText(text, rect.x + (rect.width/2 - textWidth/2), 
                   rect.y + (rect.height/2 - fontSize/2), 
                   fontSize, textColor);

    if (isHovering) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return (isHovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

void DrawMainMenu(MenuSystemState *currentState) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    
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
        DrawText("IMAGE NOT FOUND", 10, 10, 20, RAYWHITE);
    }

    DrawRectangle(0, 0, (int)sw, (int)sh, (Color){ 0, 0, 0, 80 });

    int panelW = 300;
    int panelH = 240;
    int px = (int)(sw/2 - panelW/2);
    int py = (int)(sh/2 - panelH/2);

    DrawRectangle(px, py, panelW, panelH, (Color){ 10, 10, 20, 200 });
    DrawRectangleLines(px, py, panelW, panelH, (Color){ 100, 90, 70, 255 });

    const char *title = "The Dark";
    int tw = MeasureText(title, 16);
    DrawText(title, sw/2 - tw/2, py + 8, 16, (Color){ 255, 220, 120, 255 });
    
    int btnW = 260;
    int btnH = 45;
    int btnX = sw/2 - btnW/2;

    if (DrawButton((Rectangle){ btnX, py + 45, btnW, btnH }, "START SURVIVAL", (Color){ 120, 220, 120, 255 })) {
        *currentState = STATE_GAMEPLAY;
    }
    
    if (DrawButton((Rectangle){ btnX, py + 105, btnW, btnH }, "ADJUST GEAR", (Color){ 255, 220, 120, 255 })) {
        *currentState = STATE_SETTINGS;
    }

    if (DrawButton((Rectangle){ btnX, py + 165, btnW, btnH }, "ABANDON", (Color){ 220, 80, 80, 255 })) {
        *currentState = STATE_EXIT;
    }

    const char *hint = "TAB - survival log";
    int hw = MeasureText(hint, 11);
    DrawText(hint, sw/2 - hw/2, py + panelH - 18, 11, (Color){ 120, 120, 120, 255 });
}
