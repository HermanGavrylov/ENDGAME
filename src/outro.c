#include "header.h"

#define OUTRO_LINES      6
#define OUTRO_CHAR_DELAY 0.032f
#define OUTRO_LINE_HOLD  2.5f
#define OUTRO_FADE_OUT   0.6f
#define OUTRO_FINAL_HOLD 4.0f

static const char *OUTRO_TEXT[OUTRO_LINES] = {
    "Dawn broke over the island.",
    "James stood at the shoreline,\nbattered but alive.",
    "They found him three days later.",
    "He never spoke of that night.",
    "Some things are better\nleft in the dark.",
    "You survived.",
};

static const float OUTRO_SIZES[OUTRO_LINES] = {
    20, 20, 22, 20, 22, 34,
};

static const int OUTRO_BOLD[OUTRO_LINES] = {
    0, 0, 0, 0, 0, 1,
};

static void OutroDrawCenteredWrapped(const char *text, int fontSize, Color col) {
    int lineH = fontSize + 6;
    int total = 0;
    const char *p = text;
    while (*p) { if (*p == '\n') total++; p++; }
    total++;

    int startY = SCREEN_H / 2 - (total * lineH) / 2;
    int line   = 0;
    char buf[128];
    int  bi = 0;

    p = text;
    while (1) {
        if (*p == '\n' || *p == '\0') {
            buf[bi] = '\0';
            int tw = MeasureText(buf, fontSize);
            DrawText(buf, (SCREEN_W - tw) / 2, startY + line * lineH, fontSize, col);
            line++;
            bi = 0;
            if (*p == '\0') break;
        } else {
            if (bi < 126) buf[bi++] = *p;
        }
        p++;
    }
}

static int OutroCountChars(const char *s) {
    int n = 0;
    while (*s) { if (*s != '\n') n++; s++; }
    return n;
}

static void OutroDrawTypewriter(const char *text, int fontSize, Color col, float t) {
    int totalChars = OutroCountChars(text);
    int visible    = (int)(t / OUTRO_CHAR_DELAY);
    if (visible > totalChars) visible = totalChars;

    int lineH = fontSize + 6;
    int total = 0;
    const char *p = text;
    while (*p) { if (*p == '\n') total++; p++; }
    total++;

    int startY = SCREEN_H / 2 - (total * lineH) / 2;
    int line   = 0;
    char buf[128];
    int  bi = 0;
    int  drawn = 0;

    p = text;
    while (1) {
        if (*p == '\n' || *p == '\0') {
            buf[bi] = '\0';
            int tw = MeasureText(buf, fontSize);
            DrawText(buf, (SCREEN_W - tw) / 2, startY + line * lineH, fontSize, col);
            line++;
            bi = 0;
            if (*p == '\0') break;
        } else {
            if (drawn < visible && bi < 126) {
                buf[bi++] = *p;
                drawn++;
            } else if (drawn >= visible) {
                buf[bi] = '\0';
                int tw = MeasureText(buf, fontSize);
                DrawText(buf, (SCREEN_W - tw) / 2, startY + line * lineH, fontSize, col);
                line++;
                bi = 0;
                while (*p && *p != '\n') p++;
                if (*p == '\n') { p++; continue; }
                break;
            }
        }
        p++;
    }
}

void OutroRun(void) {
    float t         = 0.0f;
    int   lineIdx   = 0;
    float bgAlpha   = 1.0f;   /* start from black, fade in */
    bool  finalPause = false;
    float finalT    = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* Space skips the whole outro */
        if (IsKeyPressed(KEY_SPACE)) return;

        if (finalPause) {
            finalT += dt;
            /* Fade to black, then hold */
            bgAlpha = finalT / 1.5f;
            if (bgAlpha > 1.0f) bgAlpha = 1.0f;
            if (finalT >= OUTRO_FINAL_HOLD) return;

            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                          (Color){ 0, 0, 0, (unsigned char)(bgAlpha * 255) });
            EndDrawing();
            continue;
        }

        t += dt;

        int   fs      = (int)OUTRO_SIZES[lineIdx];
        (void)OUTRO_BOLD[lineIdx];
        int   typeLen = OutroCountChars(OUTRO_TEXT[lineIdx]);
        float typeDur = typeLen * OUTRO_CHAR_DELAY;
        float hold    = typeDur + OUTRO_LINE_HOLD;

        /* Fade-in for the very first beat after typewriter ends */
        float lineAlpha = 1.0f;
        if (t > hold - OUTRO_FADE_OUT && t < hold)
            lineAlpha = 1.0f - (t - (hold - OUTRO_FADE_OUT)) / OUTRO_FADE_OUT;
        else if (t >= hold)
            lineAlpha = 0.0f;

        unsigned char ca = (unsigned char)(lineAlpha * 255.0f);
        Color textCol    = { 220, 215, 205, ca };

        /* Keep background dark throughout */
        bgAlpha += (0.88f - bgAlpha) * dt * 2.0f;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                      (Color){ 0, 0, 0, (unsigned char)(bgAlpha * 255) });

        if (t < typeDur) {
            OutroDrawTypewriter(OUTRO_TEXT[lineIdx], fs, textCol, t);
        } else {
            OutroDrawCenteredWrapped(OUTRO_TEXT[lineIdx], fs, textCol);
        }

        /* On the last card show a pulsing "Thank you for playing" subtitle */
        if (lineIdx == OUTRO_LINES - 1) {
            const char *sub = "Thank you for playing";
            int sw = MeasureText(sub, 14);
            unsigned char sa = (unsigned char)(
                fminf(1.0f, (t - typeDur) / 0.8f) *
                (0.5f + 0.5f * sinf(t * 2.5f)) * 180.0f);
            DrawText(sub, (SCREEN_W - sw) / 2, SCREEN_H * 3 / 4, 14,
                     (Color){ 180, 170, 150, sa });
        }

        DrawText("SPACE - skip", 12, SCREEN_H - 24, 12, (Color){ 100, 100, 100, 120 });

        EndDrawing();

        if (t >= hold) {
            t = 0.0f;
            lineIdx++;
            if (lineIdx >= OUTRO_LINES) {
                finalPause = true;
                finalT     = 0.0f;
                bgAlpha    = 0.0f;
            }
        }
    }
}
