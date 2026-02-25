#include "header.h"

void ParticlesInit(Particles *ps) {
    memset(ps, 0, sizeof(Particles));
}

static void Spawn(Particles *ps, Vector2 pos, Vector2 vel,
                  float life, Color col, float size, ParticleType kind) {
    if (ps->count >= MAX_PARTICLES) {
        /* overwrite oldest */
        for (int i = 0; i < MAX_PARTICLES - 1; i++)
            ps->list[i] = ps->list[i + 1];
        ps->count = MAX_PARTICLES - 1;
    }
    Particle *p = &ps->list[ps->count++];
    p->pos     = pos;
    p->vel     = vel;
    p->life    = life;
    p->maxLife = life;
    p->color   = col;
    p->size    = size;
    p->kind    = kind;
}

void ParticlesSpawnBlood(Particles *ps, Vector2 pos, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 vel = {
            ((rand() % 200) - 100) * 0.01f * 60.0f,
            ((rand() % 100) - 120) * 0.01f * 60.0f,
        };
        float life = 0.3f + (rand() % 40) * 0.01f;
        Color  col = { 200, 20 + rand() % 30, 20, 255 };
        float  sz  = 2.0f + (rand() % 3);
        Spawn(ps, pos, vel, life, col, sz, PARTICLE_BLOOD);
    }
}

void ParticlesSpawnDust(Particles *ps, Vector2 pos, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 vel = {
            ((rand() % 160) - 80) * 0.01f * 60.0f,
            ((rand() % 60)  - 80) * 0.01f * 60.0f,
        };
        float life = 0.2f + (rand() % 30) * 0.01f;
        int   g    = 160 + rand() % 60;
        Color col  = { (unsigned char)g, (unsigned char)(g - 20), (unsigned char)(g - 40), 220 };
        float sz   = 1.5f + (rand() % 2);
        Spawn(ps, pos, vel, life, col, sz, PARTICLE_DUST);
    }
}

void ParticlesSpawnSpark(Particles *ps, Vector2 pos, int count) {
    for (int i = 0; i < count; i++) {
        Vector2 vel = {
            ((rand() % 300) - 150) * 0.01f * 60.0f,
            ((rand() % 200) - 180) * 0.01f * 60.0f,
        };
        float life = 0.15f + (rand() % 20) * 0.01f;
        Color col  = { 255, 200 + rand() % 55, 30, 255 };
        float sz   = 1.5f;
        Spawn(ps, pos, vel, life, col, sz, PARTICLE_SPARK);
    }
}

void ParticlesUpdate(Particles *ps, float dt) {
    for (int i = 0; i < ps->count; i++) {
        Particle *p = &ps->list[i];
        p->vel.y += GRAVITY * 0.4f * dt;
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->life  -= dt;
    }
    int alive = 0;
    for (int i = 0; i < ps->count; i++)
        if (ps->list[i].life > 0.0f)
            ps->list[alive++] = ps->list[i];
    ps->count = alive;
}

void ParticlesDraw(const Particles *ps) {
    for (int i = 0; i < ps->count; i++) {
        const Particle *p = &ps->list[i];
        float t   = p->life / p->maxLife;
        Color col = p->color;
        col.a     = (unsigned char)(t * p->color.a);
        DrawRectangleV(
            (Vector2){ p->pos.x - p->size * 0.5f, p->pos.y - p->size * 0.5f },
            (Vector2){ p->size, p->size },
            col
        );
    }
}
