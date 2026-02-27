#include "header.h"

void PlayerInit(Player *p, const World *w, CharClass cls) {
    CharDef cd  = GetCharDef(cls);
    int cx      = WORLD_W / 2;
    int surface = SURFACE_LEVEL;
    for (int y = 0; y < WORLD_H; y++) {
        if (WorldIsSolid(w, cx, y)) { surface = y; break; }
    }
    p->pos        = (Vector2){ cx * TILE_SIZE - PLAYER_W * 0.5f,
                               (surface - 3) * (float)TILE_SIZE };
    p->vel        = (Vector2){ 0.0f, 0.0f };
    p->onGround   = false;
    p->facingLeft = false;
    p->maxHp      = PLAYER_MAX_HP + cd.bonusHp;
    p->hp         = p->maxHp;
    p->hunger        = HUNGER_MAX;
    p->hungerDmgTimer = HUNGER_DMG_TICK;
    p->iframes    = 0.0f;
    p->swordTimer = 0.0f;
    p->attacking  = false;
    p->kills      = 0;
    p->temperature    = TEMP_MAX;
    p->freezeDmgTimer = TEMP_FREEZE_TICK;
}

static Rectangle ResolveCollision(const World *w, Rectangle rect, Vector2 *vel) {
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
                if (oT < oB) { rect.y -= oT; if (vel->y > 0) vel->y = 0; }
                else         { rect.y += oB; if (vel->y < 0) vel->y = 0; }
            } else {
                if (oL < oR) rect.x -= oL; else rect.x += oR;
                vel->x = 0;
            }
        }
    }
    return rect;
}

void PlayerUpdate(Player *p, const World *w, float dt, float speedMult) {
    if (p->iframes    > 0.0f) p->iframes    -= dt;
    if (p->swordTimer > 0.0f) {
        p->swordTimer -= dt;
        if (p->swordTimer <= 0.0f) p->attacking = false;
    }

    float speed = PLAYER_SPEED * speedMult;
    p->vel.x = 0;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { p->vel.x = -speed; p->facingLeft = true;  }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { p->vel.x =  speed; p->facingLeft = false; }

    if (p->onGround && (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP))) {
        p->vel.y = JUMP_FORCE; p->onGround = false;
    }

    p->vel.y += GRAVITY * dt;
    if (p->vel.y > MAX_FALL_SPEED) p->vel.y = MAX_FALL_SPEED;

    Rectangle rect = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
    rect.x += p->vel.x * dt; rect = ResolveCollision(w, rect, &p->vel);
    rect.y += p->vel.y * dt; rect = ResolveCollision(w, rect, &p->vel);
    p->pos.x = rect.x; p->pos.y = rect.y;

    int bx0 = (int)floorf((rect.x + 1.0f) / TILE_SIZE);
    int bx1 = (int)floorf((rect.x + rect.width - 1.0f) / TILE_SIZE);
    int by  = (int)floorf((rect.y + rect.height + 0.5f) / TILE_SIZE);
    p->onGround = false;
    for (int tx = bx0; tx <= bx1; tx++)
        if (WorldIsSolid(w, tx, by)) { p->onGround = true; break; }
}

bool PlayerHasSwordActive(const Inventory *inv) {
    return inv->hotbar[inv->activeSlot].type == TILE_SWORD;
}

void PlayerAttack(Player *p, Monsters *ms, Particles *ps,
                  const Inventory *inv, float damageMult, float dt) {
    (void)dt;
    if (!PlayerHasSwordActive(inv)) { p->attacking = false; return; }
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsKeyPressed(KEY_Z)) return;
    if (p->swordTimer > 0.0f) return;

    p->attacking  = true;
    p->swordTimer = SWORD_COOLDOWN;

    float cx  = p->pos.x + PLAYER_W * 0.5f;
    float cy  = p->pos.y + PLAYER_H * 0.5f;
    float dir = p->facingLeft ? -1.0f : 1.0f;

    float hitW = SWORD_REACH;
    float hitH = PLAYER_H * 1.1f;
    float hitX = cx + dir * (PLAYER_W * 0.5f + hitW * 0.5f);
    Rectangle hitbox = { hitX - hitW * 0.5f, cy - hitH * 0.5f, hitW, hitH };

    int dmg = (int)(SWORD_DAMAGE * damageMult);

    for (int i = 0; i < ms->count; i++) {
        Monster *m = &ms->list[i];
        if (!m->alive || m->iframes > 0.0f) continue;
        int mw = (m->kind == MONSTER_TYPE_SPIDER) ? SPIDER_W
               : (m->kind == MONSTER_TYPE_GIANT)  ? GIANT_W : MONSTER_W;
        int mh = (m->kind == MONSTER_TYPE_SPIDER) ? SPIDER_H
               : (m->kind == MONSTER_TYPE_GIANT)  ? GIANT_H : MONSTER_H;
        Rectangle mr = { m->pos.x, m->pos.y, (float)mw, (float)mh };
        if (CheckCollisionRecs(hitbox, mr)) {
            m->hp     -= dmg;
            m->iframes = MONSTER_IFRAMES;
            float kbDir = (m->pos.x > cx) ? 1.0f : -1.0f;
            m->vel.x = kbDir * 120.0f;
            m->vel.y = -160.0f;
            Vector2 hitPos = { m->pos.x + mw * 0.5f, m->pos.y + mh * 0.5f };
            ParticlesSpawnBlood(ps, hitPos, 8);
            ParticlesSpawnSpark(ps, hitPos, 4);
        }
    }
}

static void DrawHeldItem(const Player *p, const Inventory *inv) {
    float cx = p->pos.x + PLAYER_W * 0.5f;
    float cy = p->pos.y + PLAYER_H * 0.4f;
    TileType held = inv->hotbar[inv->activeSlot].type;

    if (held == TILE_SWORD && p->attacking) {
        float progress = 1.0f - (p->swordTimer / SWORD_COOLDOWN);
        float angle = p->facingLeft
            ? (180.0f - (1.0f - progress) * 90.0f)
            : ((1.0f - progress) * 90.0f - 45.0f);
        SwordDrawInHand(cx, cy, angle, SWORD_REACH, p->facingLeft);
    }
}

void PlayerDraw(const Player *p, const Inventory *inv, Color tint, CharClass cls) {
    (void)inv;
    bool flash = (p->iframes > 0.0f) && ((int)(p->iframes * 10) % 2 == 0);
    if (!flash) PlayerSpriteDrawClass(p->pos.x, p->pos.y, p->facingLeft, tint, cls);
    DrawHeldItem(p, inv);
}

void DrawMinimap(const Player *p, const World *w) {
    int ox = SCREEN_W - MM_W - MM_PAD;
    int oy = MM_PAD;
    DrawRectangle(ox - 2, oy - 2, MM_W + 4, MM_H + 4, (Color){ 0, 0, 0, 180 });
    DrawRectangleLines(ox - 2, oy - 2, MM_W + 4, MM_H + 4,
                       (Color){ 120, 120, 120, 200 });
    int ptx    = (int)(p->pos.x / TILE_SIZE);
    int pty    = (int)(p->pos.y / TILE_SIZE);
    int startX = ptx - (MM_W * MM_SCALE) / 2;
    int startY = pty - (MM_H * MM_SCALE) / 2;
    for (int py = 0; py < MM_H; py++) {
        for (int px = 0; px < MM_W; px++) {
            int tx = startX + px * MM_SCALE;
            int ty = startY + py * MM_SCALE;
            if (!WorldInBounds(tx, ty)) continue;
            TileType t = w->tiles[ty][tx].type;
            if (t == TILE_AIR) continue;
            Color c = TileColor(t); c.a = 220;
            DrawPixel(ox + px, oy + py, c);
        }
    }
    DrawRectangle(ox + MM_W / 2 - 1, oy + MM_H / 2 - 1, 3, 3,
                  (Color){ 255, 255, 80, 255 });
}
