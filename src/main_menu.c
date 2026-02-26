#include "header.h"
#include "menu.h"

static Music menuMusic;
static bool musicLoaded = false;

void UpdateMenuAudio(void) {
    if (!musicLoaded) {
        menuMusic = LoadMusicStream("resource/sound/menu_background.wav");
        PlayMusicStream(menuMusic);
        musicLoaded = true;
    }
    UpdateMusicStream(menuMusic);
}

static bool DrawButton(Rectangle rect, const char *text, Color hoverTint) {
    static Sound clickSnd;
    static bool soundLoaded = false;

    if (!soundLoaded) {
        clickSnd = LoadSound("resource/sound/button.mp3");
        soundLoaded = true;
    }

    Vector2 mouse      = GetMousePosition();
    bool    isHovering = CheckCollisionPointRec(mouse, rect);

    Color baseBg      = { 30, 30, 40, 140 };
    Color borderColor = isHovering ? hoverTint : (Color){ 70, 65, 55, 255 };

    DrawRectangleRec(rect, baseBg);
    DrawRectangleLinesEx(rect, 1, borderColor);

    int   fontSize  = 14;
    int   textWidth = MeasureText(text, fontSize);
    Color textColor = isHovering ? hoverTint : (Color){ 230, 220, 200, 255 };

    DrawText(text,
             rect.x + (rect.width  / 2 - textWidth / 2),
             rect.y + (rect.height / 2 - fontSize  / 2),
             fontSize, textColor);

    if (isHovering) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(clickSnd);
            return true;
        }
    }
    return false;
}

void DrawMainMenu(MenuSystemState *currentState) {
    UpdateMenuAudio(); 

    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    static Texture2D menuBg;
    static bool      texLoaded = false;
    if (!texLoaded) {
        menuBg = LoadTexture("resource/images/background_image.jpg");
        texLoaded = true;
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

    int panelW = 300, panelH = 240;
    int px = (int)(sw / 2 - panelW / 2);
    int py = (int)(sh / 2 - panelH / 2);

    DrawRectangle(px, py, panelW, panelH, (Color){ 10, 10, 20, 200 });
    DrawRectangleLines(px, py, panelW, panelH, (Color){ 100, 90, 70, 255 });

    const char *title = "The Dark";
    int tw = MeasureText(title, 16);
    DrawText(title, sw / 2 - tw / 2, py + 8, 16, (Color){ 255, 220, 120, 255 });

    int btnW = 260, btnH = 45;
    int btnX = (int)(sw / 2 - btnW / 2);

    if (DrawButton((Rectangle){ (float)btnX, (float)py + 45, (float)btnW, (float)btnH },
                   "START SURVIVAL", (Color){ 120, 220, 120, 255 }))
        *currentState = STATE_CHARSELECT;

    if (DrawButton((Rectangle){ (float)btnX, (float)py + 105, (float)btnW, (float)btnH },
                   "SETTINGS", (Color){ 255, 220, 120, 255 }))
        *currentState = STATE_SETTINGS;

    if (DrawButton((Rectangle){ (float)btnX, (float)py + 165, (float)btnW, (float)btnH },
                   "QUIT", (Color){ 220, 80, 80, 255 }))
        *currentState = STATE_EXIT;

    const char *hint = "Press START SURVIVAL to start";
    int hw = MeasureText(hint, 11);
    DrawText(hint, sw / 2 - hw / 2, py + panelH - 18, 11, (Color){ 120, 120, 120, 255 });

    if (*currentState == STATE_GAMEPLAY || *currentState == STATE_EXIT) {
        StopMusicStream(menuMusic);
    }
}

void DrawCharSelect(MenuSystemState *currentState, CharClass *selected) {
    UpdateMenuAudio(); 
    
    static Sound selectSnd;
    static bool selectSndLoaded = false;
    if (!selectSndLoaded) {
        selectSnd = LoadSound("resource/sound/button.mp3");
        selectSndLoaded = true;
    }

    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    ClearBackground((Color){ 8, 8, 18, 255 });

    const char *title = "Choose Your Character";
    int tf = 22;
    int tw = MeasureText(title, tf);
    DrawText(title, (int)(sw / 2 - tw / 2), 50, tf, (Color){ 255, 220, 120, 255 });

    int cardW = 240, cardH = 200;
    int gap   = 28;
    int totalW = CHAR_COUNT * cardW + (CHAR_COUNT - 1) * gap;
    int startX = (int)(sw / 2 - totalW / 2);
    int cardY  = (int)(sh / 2 - cardH / 2) - 20;

    bool anyHover = false;

    for (int i = 0; i < CHAR_COUNT; i++) {
        CharDef   cd  = GetCharDef((CharClass)i);
        int       cx  = startX + i * (cardW + gap);
        bool      sel = (*selected == (CharClass)i);
        Vector2   mp  = GetMousePosition();
        Rectangle r   = { (float)cx, (float)cardY, (float)cardW, (float)cardH };
        bool      hov = CheckCollisionPointRec(mp, r);
        if (hov) { anyHover = true; SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); }

        Color bg = sel ? (Color){ 0, 0, 0, 0 }
                 : hov ? (Color){ 22, 22, 38, 220 }
                       : (Color){ 14, 14, 24, 200 };
        DrawRectangle(cx, cardY, cardW, cardH, bg);

        Color border = sel ? cd.tint
                     : hov ? (Color){ 140, 130, 100, 255 }
                           : (Color){ 55, 50, 42, 255 };
        DrawRectangleLines(cx, cardY, cardW, cardH, border);

        int avatarSz = 64;
        int ax = cx + cardW / 2 - avatarSz / 2;
        int ay = cardY + 10;
        if (PlayerTexIsLoaded((CharClass)i)) {
            Texture2D tex = PlayerTexGet((CharClass)i);
            Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
            Rectangle dst = { (float)ax, (float)ay, (float)avatarSz, (float)avatarSz };
            DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
        } else {
            DrawRectangle(ax, ay, avatarSz, avatarSz, cd.tint);
        }
        DrawRectangleLines(ax, ay, avatarSz, avatarSz, sel ? cd.tint : (Color){ 80, 75, 65, 180 });

        int nw = MeasureText(cd.name, 17);
        DrawText(cd.name, cx + cardW / 2 - nw / 2, cardY + 76, 17, cd.tint);

        int dw = MeasureText(cd.desc, 11);
        DrawText(cd.desc, cx + cardW / 2 - dw / 2, cardY + 102, 11, (Color){ 195, 190, 175, 255 });

        if (sel) {
            const char *lbl = "[ SELECTED ]";
            int lw = MeasureText(lbl, 12);
            DrawText(lbl, cx + cardW / 2 - lw / 2, cardY + cardH - 24, 12, (Color){ 100, 245, 100, 255 });
        }

        if (hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(selectSnd);
            *selected = (CharClass)i;
        }
    }

    if (!anyHover) SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    int btnW = 200, btnH = 44;
    int btnX = (int)(sw / 2 - btnW / 2);
    int btnY = cardY + cardH + 36;

    if (DrawButton((Rectangle){ (float)btnX, (float)btnY, (float)btnW, (float)btnH }, "START GAME", (Color){ 120, 220, 120, 255 })) {
        StopMusicStream(menuMusic);
        *currentState = STATE_GAMEPLAY;
    }

    if (DrawButton((Rectangle){ (float)btnX, (float)btnY + 54, (float)btnW, (float)btnH }, "BACK", (Color){ 220, 80, 80, 255 }))
        *currentState = STATE_MENU;
}

void DrawGameOver(MenuSystemState *currentState, GameState *gs) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    DrawRectangle(0, 0, (int)sw, (int)sh, (Color){ 0, 0, 0, 180 });

    int panelW = 320, panelH = 220;
    int px = (int)(sw / 2 - panelW / 2);
    int py = (int)(sh / 2 - panelH / 2);

    DrawRectangle(px, py, panelW, panelH, (Color){ 10, 5, 5, 230 });
    DrawRectangleLines(px, py, panelW, panelH, (Color){ 180, 40, 40, 255 });

    const char *title = "GAME OVER";
    int tf = 36;
    int tw = MeasureText(title, tf);
    DrawText(title, (int)(sw / 2 - tw / 2), py + 20, tf, (Color){ 220, 40, 40, 255 });

    const char *sub = "You died in the dark...";
    int sw2 = MeasureText(sub, 13);
    DrawText(sub, (int)(sw / 2 - sw2 / 2), py + 70, 13, (Color){ 180, 140, 140, 255 });

    int btnW = 260, btnH = 45;
    int btnX = (int)(sw / 2 - btnW / 2);

    if (DrawButton((Rectangle){ (float)btnX, (float)py + 105, (float)btnW, (float)btnH }, "PLAY AGAIN", (Color){ 120, 220, 120, 255 })) {
        CharDef cd = GetCharDef(gs->selectedChar);
        WorldGenerate(&gs->world);
        PlayerInit(&gs->player, &gs->world, gs->selectedChar);
        InputInit(&gs->input);
        InvInit(&gs->inv);
        gs->inv.hotbar[1].type  = TILE_SWORD;
        gs->inv.hotbar[1].count = 1;
        if (cd.startTorches > 0) {
            gs->inv.hotbar[0].type  = TILE_TORCH;
            gs->inv.hotbar[0].count = cd.startTorches;
        }
        DayNightInit(&gs->daynight);
        MonstersInit(&gs->monsters);
        ParticlesInit(&gs->particles);
        QuestInit(&gs->quests);
        gs->camera.zoom = 2.5f;
        *currentState = STATE_GAMEPLAY;
    }

    if (DrawButton((Rectangle){ (float)btnX, (float)py + 160, (float)btnW, (float)btnH }, "MAIN MENU", (Color){ 255, 220, 120, 255 }))
        *currentState = STATE_MENU;
}
