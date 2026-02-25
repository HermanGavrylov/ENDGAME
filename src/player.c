#include "header.h"

void PlayerInit(Player *p, const World *w) {
    int cx      = WORLD_W / 2;
    int surface = SURFACE_LEVEL;
    for (int y = 0; y < WORLD_H; y++) {
        if (WorldIsSolid(w, cx, y)) { surface = y; break; }
    }
    p->pos        = (Vector2){ cx * TILE_SIZE - PLAYER_W * 0.5f, (surface - 3) * (float)TILE_SIZE };
    p->vel        = (Vector2){ 0.0f, 0.0f };
    p->onGround   = false;
    p->facingLeft = false;
    p->hp         = PLAYER_MAX_HP;
    p->iframes    = 0.0f;
    p->swordTimer = 0.0f;
    p->attacking  = false;
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
                if (oL < oR) { rect.x -= oL; } else { rect.x += oR; }
                vel->x = 0;
            }
        }
    }
    return rect;
}

void PlayerUpdate(Player *p, const World *w, float dt) {
    if (p->iframes    > 0.0f) p->iframes    -= dt;
    if (p->swordTimer > 0.0f) {
        p->swordTimer -= dt;
        if (p->swordTimer <= 0.0f) p->attacking = false;
    }

    p->vel.x = 0;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { p->vel.x = -PLAYER_SPEED; p->facingLeft = true;  }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { p->vel.x =  PLAYER_SPEED; p->facingLeft = false; }

    if (p->onGround && (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP))) {
        p->vel.y    = JUMP_FORCE;
        p->onGround = false;
    }

    p->vel.y += GRAVITY * dt;
    if (p->vel.y > MAX_FALL_SPEED) p->vel.y = MAX_FALL_SPEED;

    Rectangle rect = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
    rect.x += p->vel.x * dt;
    rect = ResolveCollision(w, rect, &p->vel);
    rect.y += p->vel.y * dt;
    rect = ResolveCollision(w, rect, &p->vel);

    p->pos.x = rect.x;
    p->pos.y = rect.y;

    int bx0 = (int)floorf((rect.x + 1.0f) / TILE_SIZE);
    int bx1 = (int)floorf((rect.x + rect.width - 1.0f) / TILE_SIZE);
    int by  = (int)floorf((rect.y + rect.height + 0.5f) / TILE_SIZE);
    p->onGround = false;
    for (int tx = bx0; tx <= bx1; tx++) {
        if (WorldIsSolid(w, tx, by)) { p->onGround = true; break; }
    }
}

bool PlayerHasSwordActive(const Inventory *inv) {
    return inv->hotbar[inv->activeSlot].type == TILE_SWORD;
}

void PlayerAttack(Player *p, Monsters *ms, const Inventory *inv, float dt) {
    (void)dt;

    if (!PlayerHasSwordActive(inv)) { p->attacking = false; return; }
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsKeyPressed(KEY_Z)) return;
    if (p->swordTimer > 0.0f) return;

    p->attacking  = true;
    p->swordTimer = SWORD_COOLDOWN;

    float cx = p->pos.x + PLAYER_W * 0.5f;
    float cy = p->pos.y + PLAYER_H * 0.5f;
    float dir = p->facingLeft ? -1.0f : 1.0f;

    float hitX = cx + dir * (PLAYER_W * 0.5f + SWORD_REACH * 0.5f);
    float hitY = cy;
    float hitW = SWORD_REACH;
    float hitH = PLAYER_H * 1.1f;

    Rectangle hitbox = { hitX - hitW * 0.5f, hitY - hitH * 0.5f, hitW, hitH };

    for (int i = 0; i < ms->count; i++) {
        Monster *m = &ms->list[i];
        if (!m->alive || m->iframes > 0.0f) continue;

        Rectangle mr = { m->pos.x, m->pos.y, MONSTER_W, MONSTER_H };
        if (CheckCollisionRecs(hitbox, mr)) {
            m->hp      -= SWORD_DAMAGE;
            m->iframes  = MONSTER_IFRAMES;
            float kbDir = (m->pos.x > cx) ? 1.0f : -1.0f;
            m->vel.x    = kbDir * 120.0f;
            m->vel.y    = -160.0f;
            if (m->hp <= 0) m->alive = false;
        }
    }
}

static void DrawSword(const Player *p) {
    if (!p->attacking) return;

    float progress = 1.0f - (p->swordTimer / SWORD_COOLDOWN);
    float dir      = p->facingLeft ? -1.0f : 1.0f;

    float cx = p->pos.x + PLAYER_W * 0.5f;
    float cy = p->pos.y + PLAYER_H * 0.4f;

    float angle = p->facingLeft
        ? (180.0f - (1.0f - progress) * 90.0f)
        :            (1.0f - progress) * 90.0f - 45.0f;

    float rad  = angle * DEG2RAD;
    float ex   = cx + dir * cosf(rad) * SWORD_REACH;
    float ey   = cy + sinf(rad) * SWORD_REACH;

    Color blade = { 200, 210, 230, 230 };
    Color guard = { 160, 130,  60, 255 };

    DrawLineEx((Vector2){ cx, cy }, (Vector2){ ex, ey }, 2.5f, blade);

    float gx = cx + dir * PLAYER_W * 0.5f;
    DrawRectangle((int)(gx - 3), (int)(cy - 3), 6, 6, guard);
}

void PlayerDraw(const Player *p) {
    bool flash = (p->iframes > 0.0f) && ((int)(p->iframes * 10) % 2 == 0);
    if (!flash)
        PlayerSpriteDraw(p->pos.x, p->pos.y, p->facingLeft);
    DrawSword(p);
}

void PlayerDrawHUD(const Player *p) {
    int   barW = 160;
    int   barH = 14;
    int   barX = 12;
    int   barY = SCREEN_H - barH - 70;
    float frac = (float)p->hp / PLAYER_MAX_HP;

    Color fill = (frac > 0.5f)
        ? (Color){ 50, 200, 80, 255 }
        : (frac > 0.25f ? (Color){ 230, 180, 0, 255 }
                        : (Color){ 220,  40, 40, 255 });

    DrawRectangle(barX - 1, barY - 1, barW + 2, barH + 2, (Color){ 0, 0, 0, 180 });
    DrawRectangle(barX, barY, barW, barH, (Color){ 40, 10, 10, 200 });
    DrawRectangle(barX, barY, (int)(barW * frac), barH, fill);

    char buf[16];
    snprintf(buf, sizeof(buf), "HP  %d / %d", p->hp, PLAYER_MAX_HP);
    DrawText(buf, barX + 4, barY + 2, 10, WHITE);

    if (p->attacking) {
        DrawText("SWORD", barX, barY - 18, 11, (Color){ 200, 210, 230, 200 });
    }
}
