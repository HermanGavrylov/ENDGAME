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
