#include "header.h"

static Color MyColorLerp(Color a, Color b, float t)
{
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

static void DrawBarPretty(
    int x,
    int y,
    int w,
    int h,
    float frac,
    Color colA,
    Color colB,
    const char *text)
{
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;

    int fillW = (int)(w * frac);

    DrawRectangle(
        x - 6,
        y - 6,
        w + 12,
        h + 12,
        (Color){0, 0, 0, 80}
    );

    DrawRectangle(
        x - 2,
        y - 2,
        w + 4,
        h + 4,
        (Color){25, 25, 25, 240}
    );

    DrawRectangleGradientV(
        x,
        y,
        w,
        h,
        (Color){45,45,45,220},
        (Color){25,25,25,220}
    );

    DrawRectangleGradientH(
        x,
        y,
        fillW,
        h,
        colA,
        colB
    );

    DrawRectangleGradientV(
        x,
        y,
        fillW,
        h/2,
        (Color){255,255,255,60},
        (Color){255,255,255,0}
    );

    DrawRectangleLinesEx(
        (Rectangle){x,y,w,h},
        1,
        (Color){255,255,255,40}
    );

    DrawText(
        text,
        x + 9,
        y + h/2 - 7 + 1,
        14,
        BLACK
    );

    // text
    DrawText(
        text,
        x + 8,
        y + h/2 - 7,
        14,
        WHITE
    );
}

static void HPDrawHUD(const Player *p)
{
    float frac =
        (float)p->hp /
        (float)p->maxHp;

    Color a =
        MyColorLerp(
            (Color){220,60,60,255},
            (Color){60,220,100,255},
            frac);

    Color b =
        MyColorLerp(
            (Color){180,40,40,255},
            (Color){40,180,80,255},
            frac);

    char buf[64];

    snprintf(
        buf,
        sizeof(buf),
        "HP   %d / %d",
        p->hp,
        p->maxHp
    );

    DrawBarPretty(
        20,
        SCREEN_H - 90,
        220,
        20,
        frac,
        a,
        b,
        buf
    );
}

void HungerDrawHUD(const Player *p)
{
    float frac =
        p->hunger /
        HUNGER_MAX;

    Color a =
        MyColorLerp(
            (Color){200,50,50,255},
            (Color){220,160,60,255},
            frac);

    Color b =
        MyColorLerp(
            (Color){160,40,40,255},
            (Color){180,120,40,255},
            frac);

    char buf[64];

    snprintf(
        buf,
        sizeof(buf),
        "FOOD %d / %d",
        (int)p->hunger,
        (int)HUNGER_MAX
    );

    DrawBarPretty(
        20,
        SCREEN_H - 60,
        220,
        20,
        frac,
        a,
        b,
        buf
    );
}

static void TempDrawHUD(const Player *p)
{
    float frac =
        (p->temperature - TEMP_MIN) /
        (TEMP_MAX - TEMP_MIN);

    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;

    Color cold = (Color){60,120,255,255};
    Color hot  = (Color){255,140,60,255};

    Color a =
        MyColorLerp(
            cold,
            hot,
            frac);

    Color b =
        MyColorLerp(
            (Color){40,90,200,255},
            (Color){220,90,40,255},
            frac);

    char buf[64];

    snprintf(
        buf,
        sizeof(buf),
        "TEMP %.0f C",
        p->temperature
    );

    DrawBarPretty(
        20,
        SCREEN_H - 120,
        220,
        20,
        frac,
        a,
        b,
        buf
    );

    if (p->temperature < TEMP_FREEZE_THRESHOLD)
    {
        float intensity =
            (TEMP_FREEZE_THRESHOLD - p->temperature) /
            (TEMP_FREEZE_THRESHOLD - TEMP_MIN);

        if (intensity > 1)
            intensity = 1;

        DrawRectangle(
            0,
            0,
            SCREEN_W,
            SCREEN_H,
            (Color){80,140,255,(unsigned char)(intensity * 100)}
        );
    }
}

void PlayerDrawHUD(
    const Player *p,
    const World *w,
    const Camera2D *cam)
{
    (void)cam;

    DrawMinimap(p, w);

    TempDrawHUD(p);

    HPDrawHUD(p);

    HungerDrawHUD(p);
}
