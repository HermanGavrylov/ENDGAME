#include "header.h"

static bool NearTorch(const Player *p, const World *w, const Inventory *inv) {
    if (inv->hotbar[inv->activeSlot].type == TILE_TORCH) return true;

    float px = p->pos.x + PLAYER_W * 0.5f;
    float py = p->pos.y + PLAYER_H * 0.5f;

    int tx0 = (int)((px - TEMP_TORCH_DIST) / TILE_SIZE);
    int tx1 = (int)((px + TEMP_TORCH_DIST) / TILE_SIZE);
    int ty0 = (int)((py - TEMP_TORCH_DIST) / TILE_SIZE);
    int ty1 = (int)((py + TEMP_TORCH_DIST) / TILE_SIZE);

    if (tx0 < 0) tx0 = 0;
    if (ty0 < 0) ty0 = 0;
    if (tx1 >= WORLD_W) tx1 = WORLD_W - 1;
    if (ty1 >= WORLD_H) ty1 = WORLD_H - 1;

    for (int ty = ty0; ty <= ty1; ty++) {
        for (int tx = tx0; tx <= tx1; tx++) {
            if (w->tiles[ty][tx].type != TILE_TORCH) continue;
            float wx = tx * TILE_SIZE + TILE_SIZE * 0.5f;
            float wy = ty * TILE_SIZE + TILE_SIZE * 0.5f;
            float dx = wx - px, dy = wy - py;
            if (dx*dx + dy*dy <= TEMP_TORCH_DIST * TEMP_TORCH_DIST)
                return true;
        }
    }
    return false;
}

void TemperatureUpdate(Player *p, const World *w, const Inventory *inv,
                       const DayNight *dn, float dt) {
    bool nearTorch = NearTorch(p, w, inv);

    if (nearTorch) {
        p->temperature += TEMP_TORCH_RESTORE * dt;
    } else if (dn->isDay) {
        if (p->temperature < TEMP_MAX)
            p->temperature += TEMP_DAY_RESTORE * dt;
    } else {
        p->temperature -= TEMP_NIGHT_DRAIN * dt;
    }

    if (p->temperature > TEMP_MAX) p->temperature = TEMP_MAX;
    if (p->temperature < TEMP_MIN) p->temperature = TEMP_MIN;

    if (p->temperature <= TEMP_FREEZE_THRESHOLD) {
        p->freezeDmgTimer -= dt;
        if (p->freezeDmgTimer <= 0.0f) {
            p->hp -= (int)TEMP_FREEZE_DMG;
            if (p->hp < 0) p->hp = 0;
            p->freezeDmgTimer = TEMP_FREEZE_TICK;
        }
    } else {
        p->freezeDmgTimer = TEMP_FREEZE_TICK;
    }
}
