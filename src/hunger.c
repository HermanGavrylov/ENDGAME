#include "header.h"

void HungerUpdate(Player *p, float dt) {
    p->hunger -= HUNGER_DRAIN * dt;
    if (p->hunger < 0.0f) p->hunger = 0.0f;

    if (p->hunger <= 0.0f) {
        p->hungerDmgTimer -= dt;
        if (p->hungerDmgTimer <= 0.0f) {
            p->hp -= (int)HUNGER_DMG_RATE;
            if (p->hp < 0) p->hp = 0;
            p->hungerDmgTimer = HUNGER_DMG_TICK;
        }
    } else {
        p->hungerDmgTimer = HUNGER_DMG_TICK;
    }
}

void HungerDrawHUD(const Player *p) {
    int barW = 160, barH = 14;
    int barX = 12;
    int barY = SCREEN_H - barH - 50;

    float frac = p->hunger / HUNGER_MAX;
    Color fill = frac > 0.5f  ? (Color){ 200, 140, 40, 255 }
               : frac > 0.25f ? (Color){ 200, 90,  20, 255 }
                              : (Color){ 180, 30,  30, 255 };

    DrawRectangle(barX - 1, barY - 1, barW + 2, barH + 2, (Color){ 0, 0, 0, 180 });
    DrawRectangle(barX, barY, barW, barH, (Color){ 30, 15, 5, 200 });
    DrawRectangle(barX, barY, (int)(barW * frac), barH, fill);

    char buf[32];
    snprintf(buf, sizeof(buf), "Food  %d / %d", (int)p->hunger, (int)HUNGER_MAX);
    DrawText(buf, barX + 4, barY + 2, 10, WHITE);
}
