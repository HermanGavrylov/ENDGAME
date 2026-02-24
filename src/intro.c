#include "header.h"

#define INTRO_LINES      7
#define CHAR_DELAY       0.032f
#define LINE_HOLD        2.2f
#define LINE_FADE_IN     0.4f
#define LINE_FADE_OUT    0.5f
#define FINAL_HOLD       3.0f
#define SKIP_KEY         KEY_SPACE

static const char *LINES[INTRO_LINES] = {
    "Flight 247. Somewhere over the Atlantic.",
    "For James Harlow, it was supposed to be\na routine trip home.",
    "It wasn't.",
    "The crash was swift. The silence that followed - absolute.",
    "He is the only survivor.",
    "The island is unknown. The night is coming.\nSomething stirs in the dark.",
    "Survive until dawn.",
};

static const float LINE_SIZES[INTRO_LINES] = {
    18, 20, 28, 20, 24, 20, 30,
};

static const int LINE_BOLD[INTRO_LINES] = {
    0, 0, 1, 0, 1, 0, 1,
};

static void DrawCenteredWrapped(const char *text, int fontSize, Color col) {
    int lineH   = fontSize + 6;
    int total   = 0;
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

static int CountChars(const char *s) {
    int n = 0;
    while (*s) { if (*s != '\n') n++; s++; }
    return n;
}

static void DrawTypewriter(const char *text, int fontSize, Color col, float t) {
    int totalChars = CountChars(text);
    int visible    = (int)(t / CHAR_DELAY);
    if (visible > totalChars) visible = totalChars;

    int lineH   = fontSize + 6;
    int total   = 0;
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

void IntroRun(void) {
    float  t          = 0.0f;
    int    lineIdx    = 0;
    float  bgAlpha    = 0.0f;
    bool   finalPause = false;
    float  finalT     = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(SKIP_KEY)) return;

        if (finalPause) {
            finalT += dt;
            bgAlpha = 1.0f - (finalT / 1.2f);
            if (bgAlpha < 0.0f) bgAlpha = 0.0f;
            if (finalT >= FINAL_HOLD) return;

            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                          (Color){ 0, 0, 0, (unsigned char)(bgAlpha * 255) });
            EndDrawing();
            continue;
        }

        t += dt;

        int    fs      = (int)LINE_SIZES[lineIdx];
        bool   bold    = LINE_BOLD[lineIdx];
        int    typeLen = CountChars(LINES[lineIdx]);
        float  typeDur = typeLen * CHAR_DELAY;
        float  hold    = typeDur + LINE_HOLD;
        (void)bold;

        float lineAlpha = 1.0f;
        if (t > hold - LINE_FADE_OUT && t < hold)
            lineAlpha = 1.0f - (t - (hold - LINE_FADE_OUT)) / LINE_FADE_OUT;
        else if (t >= hold)
            lineAlpha = 0.0f;

        unsigned char ca = (unsigned char)(lineAlpha * 255.0f);
        Color textCol    = { 220, 215, 205, ca };
        Color dimCol     = { 140, 135, 125, (unsigned char)(lineAlpha * 160.0f) };

        float bgTarget = (lineIdx == INTRO_LINES - 1) ? 0.0f : 0.85f;
        bgAlpha += (bgTarget - bgAlpha) * dt * 2.0f;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                      (Color){ 0, 0, 0, (unsigned char)(bgAlpha * 255) });

        if (t < typeDur) {
            DrawTypewriter(LINES[lineIdx], fs, textCol, t);
        } else {
            DrawCenteredWrapped(LINES[lineIdx], fs, textCol);
        }

        if (lineIdx == INTRO_LINES - 1) {
            const char *hint = "Press SPACE to begin";
            int hw = MeasureText(hint, 14);
            unsigned char ha = (unsigned char)(fminf(1.0f, (t - typeDur) / 0.8f) *
                                               (0.5f + 0.5f * sinf(t * 3.0f)) * 180.0f);
            DrawText(hint, (SCREEN_W - hw) / 2, SCREEN_H * 3 / 4, 14, (Color){ 180, 170, 150, ha });
            (void)dimCol;
        }

        DrawText("SPACE — skip", 12, SCREEN_H - 24, 12, (Color){ 100, 100, 100, 160 });

        EndDrawing();

        if (t >= hold) {
            t = 0.0f;
            lineIdx++;
            if (lineIdx >= INTRO_LINES) {
                finalPause = true;
                finalT     = 0.0f;
                bgAlpha    = 1.0f;
            }
        }
    }
}
