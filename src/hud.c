#include "header.h"

static void DrawBar(int x, int y, int w, int h, float frac, Color fill, const char *text)
{
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;

    DrawRectangle(x + 2, y + 2, w, h, (Color){0,0,0,100});
    DrawRectangle(x - 2, y - 2, w + 4, h + 4, (Color){20,20,20,220});
    DrawRectangle(x, y, w, h, (Color){40,40,40,200});
    DrawRectangle(x, y, (int)(w * frac), h, fill);
    DrawRectangle(x, y, (int)(w * frac), 2, (Color){255,255,255,40});

    DrawText(text, x + 6, y + h/2 - 6, 12, WHITE);
}

void HungerDrawHUD(const Player *p)
{
    float frac = p->hunger / HUNGER_MAX;

    Color fill =
        frac > 0.5f  ? (Color){220,160,60,255} :
        frac > 0.25f ? (Color){220,110,40,255} :
                       (Color){200,50,50,255};

    char buf[64];
    snprintf(buf, sizeof(buf), "FOOD %d / %d", (int)p->hunger, (int)HUNGER_MAX);

    DrawBar(20, SCREEN_H - 50, 160, 16, frac, fill, buf);
}

void PlayerDrawHUD(const Player *p, const World *w, const Camera2D *cam)
{
    (void)cam;
    (void)w;

    float frac = (float)p->hp / (float)p->maxHp;

    Color fill =
        frac > 0.5f  ? (Color){60,220,100,255} :
        frac > 0.25f ? (Color){240,180,40,255} :
                       (Color){220,60,60,255};

    char buf[64];
    snprintf(buf, sizeof(buf), "HP %d / %d", p->hp, p->maxHp);

    DrawMinimap(p, w);

    DrawBar(20, SCREEN_H - 80, 160, 16, frac, fill, buf);

    HungerDrawHUD(p);
}
