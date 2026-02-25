#include "header.h"

#define INTRO_LINES      7
#define CHAR_DELAY       0.032f
#define LINE_HOLD        2.2f
#define LINE_FADE_OUT    0.5f
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

static int CountChars(const char *s) {
    int n = 0;
    while (*s) { if (*s != '\n') n++; s++; }
    return n;
}

static void DrawCenteredWrapped(const char *text, int fontSize, Color col) {
    int lineH   = fontSize + 6;
    int total   = 1;
    for (const char *p = text; *p; p++)
        if (*p == '\n') total++;

    int startY = SCREEN_H / 2 - (total * lineH) / 2;
    int line   = 0;
    char buf[128];
    int  bi = 0;

    for (const char *p = text;; p++) {
        if (*p == '\n' || *p == '\0') {
            buf[bi] = '\0';
            int tw = MeasureText(buf, fontSize);
            DrawText(buf, (SCREEN_W - tw) / 2,
                     startY + line * lineH, fontSize, col);
            line++;
            bi = 0;
            if (*p == '\0') break;
        } else {
            if (bi < 126) buf[bi++] = *p;
        }
    }
}

static void DrawTypewriter(const char *text, int fontSize, Color col, float t) {
    int visible = (int)(t / CHAR_DELAY);
    int drawn   = 0;

    int lineH   = fontSize + 6;
    int total   = 1;
    for (const char *p = text; *p; p++)
        if (*p == '\n') total++;

    int startY = SCREEN_H / 2 - (total * lineH) / 2;
    int line   = 0;
    char buf[128];
    int  bi = 0;

    for (const char *p = text;; p++) {
        if (*p == '\n' || *p == '\0') {
            buf[bi] = '\0';
            int tw = MeasureText(buf, fontSize);
            DrawText(buf, (SCREEN_W - tw) / 2,
                     startY + line * lineH, fontSize, col);
            line++;
            bi = 0;
            if (*p == '\0') break;
        } else {
            if (drawn < visible && bi < 126) {
                buf[bi++] = *p;
                drawn++;
            }
        }
    }
}

void IntroRun(void) {
    float  t       = 0.0f;
    int    lineIdx = 0;
    bool   finishedText = false;

    while (!WindowShouldClose()) {

        float dt = GetFrameTime();
        t += dt;

        int   fs      = (int)LINE_SIZES[lineIdx];
        int   typeLen = CountChars(LINES[lineIdx]);
        float typeDur = typeLen * CHAR_DELAY;
        float hold    = typeDur + LINE_HOLD;

        BeginDrawing();
        ClearBackground(BLACK);

        if (t < typeDur) {
            DrawTypewriter(LINES[lineIdx], fs,
                           (Color){220,215,205,255}, t);
        } else {
            DrawCenteredWrapped(LINES[lineIdx], fs,
                                (Color){220,215,205,255});
        }

        if (lineIdx == INTRO_LINES - 1 && t >= hold) {
            finishedText = true;

            const char *hint = "Press SPACE to begin";
            int hw = MeasureText(hint, 16);
            DrawText(hint,
                     (SCREEN_W - hw) / 2,
                     SCREEN_H * 3 / 4,
                     16,
                     (Color){180,170,150,
                             (unsigned char)(120 + 60 * sinf(GetTime() * 3))});
        }

        DrawText("SPACE - skip",
                 12, SCREEN_H - 24,
                 12, (Color){100,100,100,160});

        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {

            if (!finishedText) {
                lineIdx++;
                t = 0.0f;

                if (lineIdx >= INTRO_LINES)
                    lineIdx = INTRO_LINES - 1;
            }
            else {
                return;
            }
        }

        if (!finishedText && t >= hold) {
            t = 0.0f;
            lineIdx++;
            if (lineIdx >= INTRO_LINES)
                lineIdx = INTRO_LINES - 1;
        }
    }
}
