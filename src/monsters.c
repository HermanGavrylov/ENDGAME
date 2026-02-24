#include "header.h"

void MonstersInit(Monsters *ms) {
    memset(ms, 0, sizeof(Monsters));
    ms->count = 0;
}

static void SpawnMonster(Monsters *ms, Vector2 pos) {
    if (ms->count >= MAX_MONSTERS) return;
    Monster *m  = &ms->list[ms->count++];
    m->pos      = pos;
    m->vel      = (Vector2){ 0, 0 };
    m->hp       = MONSTER_HP;
    m->iframes  = 0.0f;
    m->alive    = true;
    m->facingLeft = false;
}

void MonstersSpawnNight(Monsters *ms, const Player *p, const World *w, const DayNight *dn) {
    if (dn->isDay) return;

    float nightT = dn->elapsed - DAY_DURATION;
    float spawnInterval = 8.0f;
    int   spawnsExpected = (int)(nightT / spawnInterval);
    if (ms->count >= spawnsExpected * 2 || ms->count >= MAX_MONSTERS) return;

    float side  = (rand() % 2) ? 1.0f : -1.0f;
    float dist  = 280.0f + rand() % 220;
    float spawnX = p->pos.x + side * dist;

    int tx = (int)(spawnX / TILE_SIZE);
    if (tx < 1) tx = 1;
    if (tx >= WORLD_W - 1) tx = WORLD_W - 2;

    bool cave = (rand() % 3) == 0;

    float spawnY = p->pos.y;

    if (cave) {
        int startTY = (int)(p->pos.y / TILE_SIZE) + 2;
        if (startTY < 0) startTY = 0;
        if (startTY >= WORLD_H) startTY = WORLD_H - 1;
        for (int ty = startTY; ty < WORLD_H - 1; ty++) {
            if (w->tiles[ty][tx].type == TILE_AIR &&
                w->tiles[ty + 1][tx].type != TILE_AIR &&
                !TileIsLiquid(w->tiles[ty + 1][tx].type)) {
                spawnY = ty * TILE_SIZE - MONSTER_H;
                break;
            }
        }
    } else {
        for (int ty = 0; ty < WORLD_H - 1; ty++) {
            if (w->tiles[ty][tx].type == TILE_AIR &&
                w->tiles[ty + 1][tx].type != TILE_AIR &&
                !TileIsLiquid(w->tiles[ty + 1][tx].type)) {
                spawnY = ty * TILE_SIZE - MONSTER_H;
                break;
            }
        }
    }

    SpawnMonster(ms, (Vector2){ spawnX, spawnY });
}

static Rectangle MonsterRect(const Monster *m) {
    return (Rectangle){ m->pos.x, m->pos.y, MONSTER_W, MONSTER_H };
}

static void ResolveMonsterCollision(const World *w, Monster *m) {
    Rectangle rect = MonsterRect(m);

    int left   = (int)floorf(rect.x / TILE_SIZE);
    int right  = (int)floorf((rect.x + rect.width  - 0.01f) / TILE_SIZE);
    int top    = (int)floorf(rect.y / TILE_SIZE);
    int bottom = (int)floorf((rect.y + rect.height - 0.01f) / TILE_SIZE);

    for (int ty = top; ty <= bottom; ty++) {
        for (int tx = left; tx <= right; tx++) {
            if (!WorldIsSolid(w, tx, ty)) continue;
            Rectangle tile = {
                (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE),
                (float)TILE_SIZE, (float)TILE_SIZE
            };
            if (!CheckCollisionRecs(rect, tile)) continue;

            float oL = (rect.x + rect.width)  - tile.x;
            float oR = (tile.x + tile.width)   - rect.x;
            float oT = (rect.y + rect.height)  - tile.y;
            float oB = (tile.y + tile.height)  - rect.y;
            float mH = (oL < oR) ? oL : oR;
            float mV = (oT < oB) ? oT : oB;

            if (mV <= mH) {
                if (oT < oB) { rect.y -= oT; if (m->vel.y > 0) m->vel.y = 0; }
                else         { rect.y += oB; if (m->vel.y < 0) m->vel.y = 0; }
            } else {
                if (oL < oR) { rect.x -= oL; m->vel.x = -m->vel.x; }
                else         { rect.x += oR; m->vel.x = -m->vel.x; }
            }
        }
    }
    m->pos.x = rect.x;
    m->pos.y = rect.y;
}

void MonstersUpdate(Monsters *ms, Player *p, const World *w, float dt) {
    Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };

    if (p->iframes > 0.0f) p->iframes -= dt;

    for (int i = 0; i < ms->count; i++) {
        Monster *m = &ms->list[i];
        if (!m->alive) continue;

        if (m->iframes > 0.0f) m->iframes -= dt;

        Vector2 mCenter = { m->pos.x + MONSTER_W * 0.5f, m->pos.y + MONSTER_H * 0.5f };
        float   dx      = pCenter.x - mCenter.x;
        float   dist    = fabsf(dx) + fabsf(pCenter.y - mCenter.y);

        if (dist < MONSTER_SIGHT) {
            m->vel.x    = (dx > 0) ? MONSTER_SPEED : -MONSTER_SPEED;
            m->facingLeft = dx < 0;
        } else {
            m->vel.x = 0;
        }

        bool onGround = false;
        int bx0 = (int)floorf((m->pos.x + 1.0f) / TILE_SIZE);
        int bx1 = (int)floorf((m->pos.x + MONSTER_W - 1.0f) / TILE_SIZE);
        int by  = (int)floorf((m->pos.y + MONSTER_H + 0.5f) / TILE_SIZE);
        for (int tx = bx0; tx <= bx1; tx++)
            if (WorldIsSolid(w, tx, by)) { onGround = true; break; }

        if (onGround && dist < MONSTER_SIGHT && fabsf(dx) < MONSTER_SIGHT * 0.5f
            && pCenter.y < mCenter.y)
            m->vel.y = JUMP_FORCE * 0.8f;

        m->vel.y += GRAVITY * dt;
        if (m->vel.y > MAX_FALL_SPEED) m->vel.y = MAX_FALL_SPEED;

        m->pos.x += m->vel.x * dt;
        m->pos.y += m->vel.y * dt;
        ResolveMonsterCollision(w, m);

        Rectangle mr = MonsterRect(m);
        Rectangle pr = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
        if (CheckCollisionRecs(mr, pr) && p->iframes <= 0.0f) {
            p->hp     -= MONSTER_DAMAGE;
            p->iframes = PLAYER_IFRAMES;
            if (p->hp < 0) p->hp = 0;
        }

        if (m->hp <= 0) m->alive = false;
    }

    int alive = 0;
    for (int i = 0; i < ms->count; i++)
        if (ms->list[i].alive) ms->list[alive++] = ms->list[i];
    ms->count = alive;
}

void MonstersDraw(const Monsters *ms) {
    for (int i = 0; i < ms->count; i++) {
        const Monster *m = &ms->list[i];
        if (!m->alive) continue;

        bool flash = (m->iframes > 0.0f) && ((int)(m->iframes * 10) % 2 == 0);
        Color body = flash ? WHITE : (Color){ 180, 30, 30, 255 };
        Color eyes = { 255, 50, 50, 255 };

        DrawRectangle((int)m->pos.x, (int)m->pos.y, MONSTER_W, MONSTER_H, body);

        int eyeX = m->facingLeft
                   ? (int)m->pos.x + 2
                   : (int)m->pos.x + MONSTER_W - 5;
        DrawRectangle(eyeX, (int)m->pos.y + 4, 3, 3, eyes);

        float hpFrac = (float)m->hp / MONSTER_HP;
        int   bw     = MONSTER_W + 4;
        int   bx     = (int)m->pos.x - 2;
        int   bary   = (int)m->pos.y - 6;
        DrawRectangle(bx, bary, bw, 3, (Color){ 60, 0, 0, 200 });
        DrawRectangle(bx, bary, (int)(bw * hpFrac), 3, (Color){ 220, 50, 50, 255 });
    }
}
