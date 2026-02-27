#include "header.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static Texture2D gPigTex;
static Texture2D gRabbitTex;
static bool      gPigLoaded    = false;
static bool      gRabbitLoaded = false;

static Sound sPigHurt;
static Sound sPigIdle;
static Sound sRabbitIdle;
static bool  gPigSoundLoaded = false;
static bool  gRabbitSoundLoaded = false;

void MobsLoadTextures(void) {
    if (FileExists("resource/pig.png")) {
        gPigTex    = LoadTexture("resource/pig.png");
        gPigLoaded = true;
    }
    if (FileExists("resource/rabbit.png")) {
        gRabbitTex    = LoadTexture("resource/rabbit.png");
        gRabbitLoaded = true;
    }

    if (FileExists("resource/sound/pig.mp3")) {
        sPigHurt = LoadSound("resource/sound/pig.mp3");
        sPigIdle = LoadSound("resource/sound/pig.mp3");
        gPigSoundLoaded = true;
    }

    if (FileExists("resource/sound/cow.mp3")) {
        sRabbitIdle = LoadSound("resource/sound/cow.mp3");
        gRabbitSoundLoaded = true;
    }
}

void MobsUnloadTextures(void) {
    if (gPigLoaded)    UnloadTexture(gPigTex);
    if (gRabbitLoaded) UnloadTexture(gRabbitTex);

    if (gPigSoundLoaded) {
        UnloadSound(sPigHurt);
        UnloadSound(sPigIdle);
    }
    
    if (gRabbitSoundLoaded) {
        UnloadSound(sRabbitIdle);
    }
}

static bool TrySpawnMob(Mobs *mobs, const World *w, MobType kind) {
    if (mobs->count >= MAX_MOBS)
        return false;

    int tx = 10 + rand() % (WORLD_W - 20);

    for (int ty = 0; ty < WORLD_H - 1; ty++) {
        if (w->tiles[ty][tx].type == TILE_AIR &&
            w->tiles[ty + 1][tx].type == TILE_GRASS) {

            Mob *mob = &mobs->list[mobs->count];
            memset(mob, 0, sizeof(Mob));
            
            mob->kind      = kind;
            mob->hp        = mob->maxHp = (kind == MOB_PIG) ? PIG_HP : RABBIT_HP;

            mob->pos = (Vector2){
                (float)(tx * TILE_SIZE),
                (float)((ty - 1) * TILE_SIZE)
            };

            mob->alive       = true;
            mob->wanderTimer = (float)(rand() % 3);
            mob->jumpTimer   = RABBIT_JUMP_INTERVAL;
            
            mobs->count++; 
            return true;
        }
    }
    return false;
}

void MobsInit(Mobs *mobs, const World *w) {
    memset(mobs, 0, sizeof(Mobs));
    for (int i = 0; i < 20; i++)
        TrySpawnMob(mobs, w, (rand() % 2) ? MOB_PIG : MOB_RABBIT);
}

static int MobW(const Mob *m) {
    return m->kind == MOB_RABBIT ? RABBIT_W : PIG_W;
}
static int MobH(const Mob *m) {
    return m->kind == MOB_RABBIT ? RABBIT_H : PIG_H;
}

static void ResolveMobCollision(const World *w, Mob *m) {
    Rectangle rect = { m->pos.x, m->pos.y, (float)MobW(m), (float)MobH(m) };
    int left   = (int)floorf(rect.x / TILE_SIZE);
    int right  = (int)floorf((rect.x + rect.width  - 0.01f) / TILE_SIZE);
    int top    = (int)floorf(rect.y / TILE_SIZE);
    int bottom = (int)floorf((rect.y + rect.height - 0.01f) / TILE_SIZE);

    for (int ty = top; ty <= bottom; ty++) {
        for (int tx = left; tx <= right; tx++) {
            if (!WorldIsSolid(w, tx, ty)) continue;
            Rectangle tile = { (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE };
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

void MobsSpawnDay(Mobs *mobs, const World *w, const DayNight *dn, float dt) {
    if (!dn->isDay)              return;
    if (mobs->count >= MAX_MOBS) return;

    mobs->spawnTimer += dt;
    if (mobs->spawnTimer < 4.0f) return;
    mobs->spawnTimer = 0.0f;

    for (int i = 0; i < 3; i++)
        TrySpawnMob(mobs, w, (rand() % 2) ? MOB_PIG : MOB_RABBIT);
}

void MobsUpdate(Mobs *mobs, Player *p, const World *w,
                Particles *ps, Inventory *inv, float dt) {
    Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };

    for (int i = 0; i < mobs->count; i++) {
        Mob *m = &mobs->list[i];
        if (!m->alive) continue;
        if (m->iframes > 0.0f) m->iframes -= dt;
        
        float fleeRange = (m->kind == MOB_RABBIT) ? RABBIT_FLEE_RANGE : PIG_FLEE_RANGE;
        float speed     = (m->kind == MOB_RABBIT) ? RABBIT_SPEED      : PIG_SPEED;

        float cx   = m->pos.x + MobW(m) * 0.5f;
        float cy   = m->pos.y + MobH(m) * 0.5f;
        float dx   = pCenter.x - cx;
        float dy   = pCenter.y - cy;
        float dist = sqrtf(dx * dx + dy * dy);

        if (m->kind == MOB_PIG && gPigSoundLoaded) {
            if (dist < 100.0f && dist > 30.0f) {
                if (!IsSoundPlaying(sPigIdle)) {
                    PlaySound(sPigIdle);
                }
            }
        }
        if (m->kind == MOB_RABBIT && gRabbitSoundLoaded) {
            if (dist < 100.0f && dist > 30.0f) {
                if (!IsSoundPlaying(sRabbitIdle)) {
                    PlaySound(sRabbitIdle);
                }
            }
        }
        int by  = (int)floorf((m->pos.y + MobH(m) + 0.5f) / TILE_SIZE);
        int bx0 = (int)floorf((m->pos.x + 1.0f)            / TILE_SIZE);
        int bx1 = (int)floorf((m->pos.x + MobW(m) - 1.0f)  / TILE_SIZE);
        m->onGround = false;
        for (int tx = bx0; tx <= bx1; tx++)
            if (WorldIsSolid(w, tx, by)) { m->onGround = true; break; }

        if (dist < fleeRange) {
            m->vel.x      = (dx > 0) ? -speed : speed;
            m->facingLeft = dx > 0;
            if (m->kind == MOB_RABBIT && m->onGround) {
                m->jumpTimer -= dt;
                if (m->jumpTimer <= 0.0f) {
                    m->vel.y     = JUMP_FORCE * 0.7f;
                    m->jumpTimer = RABBIT_JUMP_INTERVAL;
                }
            }
        } else {
            m->wanderTimer -= dt;
            if (m->wanderTimer <= 0.0f) {
                int dir       = (rand() % 3) - 1;
                m->vel.x      = dir * speed * 0.5f;
                m->facingLeft = dir < 0;
                m->wanderTimer = 2.0f + (rand() % 30) * 0.1f;
            }
        }

        m->vel.y += GRAVITY * dt;
        if (m->vel.y > MAX_FALL_SPEED) m->vel.y = MAX_FALL_SPEED;
        m->pos.x += m->vel.x * dt;
        m->pos.y += m->vel.y * dt;
        ResolveMobCollision(w, m);

        if (m->hp <= 0) {
            m->alive = false;
            int drops = (m->kind == MOB_PIG) ? PIG_MEAT_DROP : RABBIT_MEAT_DROP;
            for (int d = 0; d < drops; d++)
                InvAddItem(inv, TILE_MEAT);
            if (rand() % 100 < LIFEPOT_DROP_MOB)
                InvAddItem(inv, TILE_LIFEPOT);
            ParticlesSpawnBlood(ps, (Vector2){ cx, cy }, 5);
        }
    }

    int alive = 0;
    for (int i = 0; i < mobs->count; i++)
        if (mobs->list[i].alive) mobs->list[alive++] = mobs->list[i];
    mobs->count = alive;
}

void MobsAttack(Mobs *mobs, Player *p, Particles *ps,
                const Inventory *inv, float damageMult) {
    if (!PlayerHasSwordActive(inv)) return;
    if (p->swordTimer <= 0.0f)      return;

    float cx  = p->pos.x + PLAYER_W * 0.5f;
    float cy  = p->pos.y + PLAYER_H * 0.5f;
    float dir = p->facingLeft ? -1.0f : 1.0f;

    float hitW = SWORD_REACH;
    float hitH = PLAYER_H * 1.1f;
    float hitX = cx + dir * (PLAYER_W * 0.5f + hitW * 0.5f);
    Rectangle hitbox = { hitX - hitW * 0.5f, cy - hitH * 0.5f, hitW, hitH };
    int dmg = (int)(SWORD_DAMAGE * damageMult);

    for (int i = 0; i < mobs->count; i++) {
        Mob *m = &mobs->list[i];
        if (!m->alive || m->iframes > 0.0f) continue;
        Rectangle mr = { m->pos.x, m->pos.y, (float)MobW(m), (float)MobH(m) };
        if (CheckCollisionRecs(hitbox, mr)) {
            m->hp     -= dmg;
            m->iframes = MOB_IFRAMES;

            float kbDir = (m->pos.x > cx) ? 1.0f : -1.0f;
            m->vel.x = kbDir * 100.0f;
            m->vel.y = -120.0f;
            ParticlesSpawnBlood(ps, (Vector2){ m->pos.x + MobW(m) * 0.5f, m->pos.y + MobH(m) * 0.5f }, 5);
        }
    }
}

void MobsDraw(const Mobs *mobs) {
    for (int i = 0; i < mobs->count; i++) {
        const Mob *m = &mobs->list[i];
        if (!m->alive) continue;

        bool flash = (m->iframes > 0.0f) && ((int)(m->iframes * 10) % 2 == 0);
        int  mw = MobW(m), mh = MobH(m);
        int  px = (int)m->pos.x, py = (int)m->pos.y;
        Color tint = flash ? WHITE : (Color){ 255, 255, 255, 255 };

        if (m->kind == MOB_PIG) {
            if (gPigLoaded) {
                float tw = (float)gPigTex.width;
                float th = (float)gPigTex.height;
                Rectangle src = { m->facingLeft ? tw : 0, 0, m->facingLeft ? -tw : tw, th };
                Rectangle dst = { (float)px, (float)py, (float)mw, (float)mh };
                DrawTexturePro(gPigTex, src, dst, (Vector2){0,0}, 0.0f, tint);
            } else {
                DrawRectangle(px, py, mw, mh, flash ? WHITE : (Color){ 255, 180, 180, 255 });
                DrawRectangle(px + (m->facingLeft ? 1 : mw-4), py+2, 3, 3, (Color){ 80, 40, 40, 255 });
            }
        } else {
            if (gRabbitLoaded) {
                float tw = (float)gRabbitTex.width;
                float th = (float)gRabbitTex.height;
                Rectangle src = { m->facingLeft ? tw : 0, 0, m->facingLeft ? -tw : tw, th };
                Rectangle dst = { (float)px, (float)py, (float)mw, (float)mh };
                DrawTexturePro(gRabbitTex, src, dst, (Vector2){0,0}, 0.0f, tint);
            } else {
                DrawRectangle(px, py, mw, mh, flash ? WHITE : (Color){ 230, 230, 220, 255 });
                DrawRectangle(px + mw/2 - 1, py - 4, 2, 5, (Color){ 210, 210, 200, 255 });
            }
        }
    }
}
