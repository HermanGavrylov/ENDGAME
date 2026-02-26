#include "header.h"

#define PATH_LEN            32
#define BFS_MAX             4096
#define BFS_W               80
#define BFS_H               60
#define PATH_RETARGET       1.2f
#define JUMP_COOLDOWN_TIME  1.0f

void MonstersInit(Monsters *ms) {
    memset(ms, 0, sizeof(Monsters));
}

static void SpawnMonster(Monsters *ms, Vector2 pos, MonsterType kind) {
    if (ms->count >= MAX_MONSTERS) return;
    Monster *m       = &ms->list[ms->count++];
    memset(m, 0, sizeof(Monster));
    m->pos           = pos;
    m->alive         = true;
    m->kind          = kind;
    m->breakTX       = -1;
    m->breakTY       = -1;

    switch (kind) {
        case MONSTER_TYPE_SPIDER: m->hp = m->maxHp = SPIDER_HP;  break;
        case MONSTER_TYPE_GIANT:  m->hp = m->maxHp = GIANT_HP;   break;
        default:                  m->hp = m->maxHp = MONSTER_HP;  break;
    }
}

static int MonsterW(const Monster *m) {
    if (m->kind == MONSTER_TYPE_SPIDER) return SPIDER_W;
    if (m->kind == MONSTER_TYPE_GIANT)  return GIANT_W;
    return MONSTER_W;
}
static int MonsterH(const Monster *m) {
    if (m->kind == MONSTER_TYPE_SPIDER) return SPIDER_H;
    if (m->kind == MONSTER_TYPE_GIANT)  return GIANT_H;
    return MONSTER_H;
}
static float MonsterSpeed(const Monster *m) {
    if (m->kind == MONSTER_TYPE_SPIDER) return SPIDER_SPEED;
    if (m->kind == MONSTER_TYPE_GIANT)  return GIANT_SPEED;
    return MONSTER_SPEED;
}
static int MonsterDamage(const Monster *m) {
    if (m->kind == MONSTER_TYPE_SPIDER) return SPIDER_DAMAGE;
    if (m->kind == MONSTER_TYPE_GIANT)  return GIANT_DAMAGE;
    return MONSTER_DAMAGE;
}
static float MonsterBreakTime(const Monster *m) {
    if (m->kind == MONSTER_TYPE_GIANT) return GIANT_BREAK_TIME;
    return MONSTER_BREAK_TIME;
}

void MonstersSpawnNight(Monsters *ms, const Player *p, const World *w, const DayNight *dn) {
    if (dn->isDay) return;

    float nightT         = dn->elapsed - DAY_DURATION;
    float spawnInterval  = 8.0f;
    int   spawnsExpected = (int)(nightT / spawnInterval);
    if (ms->count >= spawnsExpected * 2 || ms->count >= MAX_MONSTERS) return;

    float side   = (rand() % 2) ? 1.0f : -1.0f;
    float dist   = 280.0f + rand() % 220;
    float spawnX = p->pos.x + side * dist;

    int tx = (int)(spawnX / TILE_SIZE);
    if (tx < 1) tx = 1;
    if (tx >= WORLD_W - 1) tx = WORLD_W - 2;

    int roll = rand() % 10;
    MonsterType kind = (roll < 6) ? MONSTER_TYPE_ZOMBIE
                     : (roll < 9) ? MONSTER_TYPE_SPIDER
                                  : MONSTER_TYPE_GIANT;

    bool  cave   = (rand() % 3) == 0;
    float spawnY = p->pos.y;

    if (cave) {
        int startTY = (int)(p->pos.y / TILE_SIZE) + 2;
        if (startTY < 0) startTY = 0;
        if (startTY >= WORLD_H) startTY = WORLD_H - 1;
        for (int ty = startTY; ty < WORLD_H - 1; ty++) {
            if (w->tiles[ty][tx].type == TILE_AIR &&
                w->tiles[ty + 1][tx].type != TILE_AIR &&
                !TileIsLiquid(w->tiles[ty + 1][tx].type)) {
                spawnY = ty * TILE_SIZE - MonsterH(&(Monster){ .kind = kind });
                break;
            }
        }
    } else {
        for (int ty = 0; ty < WORLD_H - 1; ty++) {
            if (w->tiles[ty][tx].type == TILE_AIR &&
                w->tiles[ty + 1][tx].type != TILE_AIR &&
                !TileIsLiquid(w->tiles[ty + 1][tx].type)) {
                spawnY = ty * TILE_SIZE - MonsterH(&(Monster){ .kind = kind });
                break;
            }
        }
    }

    SpawnMonster(ms, (Vector2){ spawnX, spawnY }, kind);
}

static Rectangle MonsterRect(const Monster *m) {
    return (Rectangle){ m->pos.x, m->pos.y,
                        (float)MonsterW(m), (float)MonsterH(m) };
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

static bool TileBreakable(TileType t) {
    return t == TILE_DIRT || t == TILE_GRASS || t == TILE_STONE;
}

typedef struct { short x, y; } BFSNode;

static bool BFSWalkable(const World *w, int tx, int ty) {
    if (tx < 0 || tx >= WORLD_W || ty < 0 || ty >= WORLD_H) return false;
    TileType t = w->tiles[ty][tx].type;
    return t == TILE_AIR || TileIsPassable(t);
}

static void BuildPath(const World *w, Monster *m, int ptx, int pty, int gtx, int gty) {
    int ox = ptx - BFS_W / 2;
    int oy = pty - BFS_H / 2;

    static signed char prevX[BFS_H][BFS_W];
    static signed char prevY[BFS_H][BFS_W];
    static bool        visited[BFS_H][BFS_W];
    memset(visited, 0, sizeof(visited));

    BFSNode queue[BFS_MAX];
    int head = 0, tail = 0;
    int sx = ptx - ox, sy = pty - oy;
    int ex = gtx - ox, ey = gty - oy;

    if (sx < 0 || sx >= BFS_W || sy < 0 || sy >= BFS_H) { m->pathLen = 0; return; }
    if (ex < 0 || ex >= BFS_W || ey < 0 || ey >= BFS_H) { m->pathLen = 0; return; }

    visited[sy][sx] = true;
    prevX[sy][sx] = prevY[sy][sx] = -1;
    queue[tail++] = (BFSNode){ (short)sx, (short)sy };
    bool found = false;

    while (head < tail && tail < BFS_MAX) {
        BFSNode cur = queue[head++];
        if (cur.x == ex && cur.y == ey) { found = true; break; }
        int dx[] = { -1, 1, 0, 0, -1, 1, -1, 1 };
        int dy[] = {  0, 0,-1, 1, -1,-1,  1,  1 };
        for (int d = 0; d < 8; d++) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            if (nx < 0 || nx >= BFS_W || ny < 0 || ny >= BFS_H) continue;
            if (visited[ny][nx]) continue;
            int wx = nx + ox, wy = ny + oy;
            bool walkable = BFSWalkable(w, wx, wy) && BFSWalkable(w, wx, wy + 1);
            bool grounded = WorldIsSolid(w, wx, wy + 2);
            bool jump1    = (dy[d] < 0) && WorldIsSolid(w, cur.x + ox, cur.y + oy + 2);
            bool diagonal = (dx[d] != 0 && dy[d] != 0);
            if (!walkable) continue;
            if (!grounded && ny != ey) continue;
            if (diagonal && !grounded) continue;
            if (dy[d] < 0 && !jump1) continue;
            visited[ny][nx] = true;
            prevX[ny][nx]   = (signed char)(-dx[d]);
            prevY[ny][nx]   = (signed char)(-dy[d]);
            queue[tail++]   = (BFSNode){ (short)nx, (short)ny };
        }
    }

    if (!found) { m->pathLen = 0; return; }
    BFSNode raw[PATH_LEN * 2];
    int rawLen = 0, cx = ex, cy = ey;
    while ((cx != sx || cy != sy) && rawLen < PATH_LEN * 2) {
        raw[rawLen++] = (BFSNode){ (short)(cx + ox), (short)(cy + oy) };
        int nx = cx + prevX[cy][cx], ny = cy + prevY[cy][cx];
        cx = nx; cy = ny;
    }
    int len = 0;
    for (int i = rawLen - 1; i >= 0 && len < PATH_LEN; i--)
        m->path[len++] = (Vector2){ raw[i].x * TILE_SIZE, raw[i].y * TILE_SIZE };
    m->pathLen = len; m->pathStep = 0;
}

static int WallProfileFromGround(const World *w, const Monster *m,
                                  bool goingLeft, int groundTY,
                                  int *outTX, int *outTY) {
    int checkX = goingLeft
        ? (int)floorf((m->pos.x - 2.0f)               / TILE_SIZE)
        : (int)floorf((m->pos.x + MonsterW(m) + 1.0f) / TILE_SIZE);
    if (checkX < 0 || checkX >= WORLD_W) return 0;

    int tyFeet = groundTY - 1;
    int tyHead = groundTY - (MonsterH(m) / TILE_SIZE) - 1;
    int solidCount = 0, lowestSolid = -1;
    for (int ty = tyHead; ty <= tyFeet; ty++) {
        if (ty < 0 || ty >= WORLD_H) continue;
        if (WorldIsSolid(w, checkX, ty)) { solidCount++; lowestSolid = ty; }
    }
    if (solidCount > 0) { *outTX = checkX; *outTY = lowestSolid; }
    return solidCount;
}

static bool CanJumpOver(const World *w, int wallTX, int wallTY, int solidCount) {
    if (solidCount != 1) return false;
    return BFSWalkable(w, wallTX, wallTY - 1) &&
           BFSWalkable(w, wallTX, wallTY - 2);
}

static void TryDigDown(Monster *m, World *w, Vector2 pCenter, float dt) {
    float myBottom = m->pos.y + MonsterH(m);
    if (pCenter.y + PLAYER_H * 0.5f <= myBottom) return;
    int digTX0 = (int)floorf((m->pos.x + 2.0f)               / TILE_SIZE);
    int digTX1 = (int)floorf((m->pos.x + MonsterW(m) - 2.0f) / TILE_SIZE);
    int digTY  = (int)floorf((myBottom + 1.0f)                / TILE_SIZE);
    if (digTY < 0 || digTY >= WORLD_H) return;
    for (int tx = digTX0; tx <= digTX1; tx++) {
        if (!WorldInBounds(tx, digTY)) continue;
        if (!TileBreakable(w->tiles[digTY][tx].type)) continue;
        if (tx != m->breakTX || digTY != m->breakTY) {
            m->breakTX = tx; m->breakTY = digTY; m->breakTimer = 0.0f;
        }
        if (m->breakCooldown <= 0.0f) {
            m->breakTimer += dt;
            if (m->breakTimer >= MonsterBreakTime(m)) {
                w->tiles[digTY][tx].type = TILE_AIR;
                int below = digTY + 1;
                if (below < WORLD_H && TileBreakable(w->tiles[below][tx].type))
                    w->tiles[below][tx].type = TILE_AIR;
                m->breakTimer    = 0.0f;
                m->breakCooldown = MONSTER_BREAK_COOLDOWN;
                m->breakTX = -1; m->breakTY = -1;
                m->retargetTimer = PATH_RETARGET;
            }
        }
        return;
    }
}

static void UpdateSpider(Monster *m, const World *w,
                         Vector2 pCenter, float dt) {
    float cx = m->pos.x + SPIDER_W * 0.5f;
    float cy = m->pos.y + SPIDER_H * 0.5f;
    float dx = pCenter.x - cx;
    float dy = pCenter.y - cy;

    int txL = (int)floorf((m->pos.x - 1.0f)              / TILE_SIZE);
    int txR = (int)floorf((m->pos.x + SPIDER_W + 0.5f)   / TILE_SIZE);
    int tyM = (int)floorf(cy / TILE_SIZE);
    bool wallL = WorldIsSolid(w, txL, tyM);
    bool wallR = WorldIsSolid(w, txR, tyM);

    if (wallL || wallR) {
        m->onWall   = true;
        m->climbDir = (dy > 0) ? 1.0f : -1.0f;
        m->vel.x    = 0;
        m->vel.y    = m->climbDir * SPIDER_SPEED;
        if (wallL) m->pos.x += 1.0f;
        if (wallR) m->pos.x -= 1.0f;
    } else {
        m->onWall = false;
        m->vel.x  = (dx > 0) ? SPIDER_SPEED : -SPIDER_SPEED;
        m->facingLeft = dx < 0;
        m->vel.y += GRAVITY * dt;
        if (m->vel.y > MAX_FALL_SPEED) m->vel.y = MAX_FALL_SPEED;
    }

    m->pos.x += m->vel.x * dt;
    m->pos.y += m->vel.y * dt;
    ResolveMonsterCollision(w, m);
}

void MonstersUpdate(Monsters *ms, Player *p, World *w,
                    Particles *ps, float defenceMult, float dt) {
    Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };
    if (p->iframes > 0.0f) p->iframes -= dt;

    for (int i = 0; i < ms->count; i++) {
        Monster *m = &ms->list[i];
        if (!m->alive) continue;
        if (m->iframes       > 0.0f) m->iframes       -= dt;
        if (m->breakCooldown > 0.0f) m->breakCooldown  -= dt;
        if (m->jumpCooldown  > 0.0f) m->jumpCooldown   -= dt;
        m->retargetTimer += dt;

        if (m->kind == MONSTER_TYPE_SPIDER) {
            UpdateSpider(m, w, pCenter, dt);
            Rectangle mr = MonsterRect(m);
            Rectangle pr = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
            if (CheckCollisionRecs(mr, pr) && p->iframes <= 0.0f) {
                int dmg = (int)(MonsterDamage(m) * defenceMult);
                p->hp     -= dmg;
                p->iframes = PLAYER_IFRAMES;
                ParticlesSpawnBlood(ps, pCenter, 6);
                if (p->hp < 0) p->hp = 0;
            }
            if (m->hp <= 0) { m->alive = false; p->kills++; }
            continue;
        }

        Vector2 mCenter = { m->pos.x + MonsterW(m) * 0.5f,
                            m->pos.y + MonsterH(m) * 0.5f };
        float dx   = pCenter.x - mCenter.x;
        float dist = fabsf(dx) + fabsf(pCenter.y - mCenter.y);

        int mtx = (int)floorf(mCenter.x / TILE_SIZE);
        int mty = (int)floorf((m->pos.y + MonsterH(m) - 1.0f) / TILE_SIZE);
        int ptx = (int)floorf(pCenter.x / TILE_SIZE);
        int pty = (int)floorf((p->pos.y + PLAYER_H   - 1.0f) / TILE_SIZE);

        int by  = (int)floorf((m->pos.y + MonsterH(m) + 0.5f) / TILE_SIZE);
        bool onGround = false;
        int bx0 = (int)floorf((m->pos.x + 1.0f)               / TILE_SIZE);
        int bx1 = (int)floorf((m->pos.x + MonsterW(m) - 1.0f) / TILE_SIZE);
        for (int tx = bx0; tx <= bx1; tx++)
            if (WorldIsSolid(w, tx, by)) { onGround = true; break; }

        if (m->retargetTimer >= PATH_RETARGET) {
            m->retargetTimer = 0.0f;
            BuildPath(w, m, mtx, mty, ptx, pty);
        }

        bool  hasPath = m->pathLen > 0 && m->pathStep < m->pathLen;
        bool  digging = false;
        float moveX   = 0.0f;
        bool  goLeft  = dx < 0;

        if (hasPath) {
            Vector2 target = m->path[m->pathStep];
            float tdx = (target.x + TILE_SIZE * 0.5f) - mCenter.x;
            float tdy = (target.y + TILE_SIZE * 0.5f) - mCenter.y;
            if (fabsf(tdx) + fabsf(tdy) < TILE_SIZE * 0.8f) {
                m->pathStep++;
                hasPath = m->pathStep < m->pathLen;
            }
            if (hasPath) {
                target = m->path[m->pathStep];
                tdx    = (target.x + TILE_SIZE * 0.5f) - mCenter.x;
                tdy    = (target.y + TILE_SIZE * 0.5f) - mCenter.y;
                goLeft = tdx < 0;
                moveX  = goLeft ? -MonsterSpeed(m) : MonsterSpeed(m);
                if (tdy < -TILE_SIZE * 0.5f && onGround &&
                    m->vel.y == 0.0f && m->jumpCooldown <= 0.0f) {
                    m->vel.y        = JUMP_FORCE * 0.85f;
                    m->jumpCooldown = JUMP_COOLDOWN_TIME;
                }
            }
        } else {
            moveX = goLeft ? -MonsterSpeed(m) : MonsterSpeed(m);
        }

        int wallTX = -1, wallTY = -1;
        int solidCount = onGround
            ? WallProfileFromGround(w, m, goLeft, by, &wallTX, &wallTY) : 0;

        if (solidCount > 0) {
            if (CanJumpOver(w, wallTX, wallTY, solidCount)) {
                if (onGround && m->vel.y == 0.0f && m->jumpCooldown <= 0.0f) {
                    m->vel.y        = JUMP_FORCE * 0.85f;
                    m->jumpCooldown = JUMP_COOLDOWN_TIME;
                }
            } else if (dist < MONSTER_DIG_RANGE) {
                digging = true;
                if (wallTX != m->breakTX || wallTY != m->breakTY) {
                    m->breakTX = wallTX; m->breakTY = wallTY; m->breakTimer = 0.0f;
                }
                if (m->breakCooldown <= 0.0f) {
                    m->breakTimer += dt;
                    if (m->breakTimer >= MonsterBreakTime(m)) {
                        if (WorldInBounds(m->breakTX, m->breakTY) &&
                            TileBreakable(w->tiles[m->breakTY][m->breakTX].type)) {
                            w->tiles[m->breakTY][m->breakTX].type = TILE_AIR;
                            int above = m->breakTY - 1;
                            if (above >= 0 &&
                                TileBreakable(w->tiles[above][m->breakTX].type))
                                w->tiles[above][m->breakTX].type = TILE_AIR;
                        }
                        ParticlesSpawnDust(ps,
                            (Vector2){ m->breakTX * TILE_SIZE + TILE_SIZE * 0.5f,
                                       m->breakTY * TILE_SIZE + TILE_SIZE * 0.5f }, 6);
                        m->breakTimer    = 0.0f;
                        m->breakCooldown = MONSTER_BREAK_COOLDOWN;
                        m->breakTX = -1; m->breakTY = -1;
                        m->retargetTimer = PATH_RETARGET;
                    }
                }
            }
        }

        if (onGround && dist < MONSTER_DIG_RANGE)
            TryDigDown(m, w, pCenter, dt);

        if (!digging) { m->vel.x = moveX; m->facingLeft = goLeft; }
        else            m->vel.x = 0;

        m->vel.y += GRAVITY * dt;
        if (m->vel.y > MAX_FALL_SPEED) m->vel.y = MAX_FALL_SPEED;
        m->pos.x += m->vel.x * dt;
        m->pos.y += m->vel.y * dt;
        ResolveMonsterCollision(w, m);

        Rectangle mr = MonsterRect(m);
        Rectangle pr = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
        if (CheckCollisionRecs(mr, pr) && p->iframes <= 0.0f) {
            int dmg = (int)(MonsterDamage(m) * defenceMult);
            p->hp     -= dmg;
            p->iframes = PLAYER_IFRAMES;
            ParticlesSpawnBlood(ps, pCenter, 6);
            if (p->hp < 0) p->hp = 0;
        }

        if (m->hp <= 0) { m->alive = false; p->kills++; }
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

        bool  flash = (m->iframes > 0.0f) && ((int)(m->iframes * 10) % 2 == 0);
        int   mw = MonsterW(m), mh = MonsterH(m);
        Color body, eyes;

        switch (m->kind) {
            case MONSTER_TYPE_SPIDER:
                body = flash ? WHITE : (Color){ 60, 20, 80, 255 };
                eyes = (Color){ 200, 0, 200, 255 };
                break;
            case MONSTER_TYPE_GIANT:
                body = flash ? WHITE : (Color){ 100, 50, 20, 255 };
                eyes = (Color){ 255, 80, 0, 255 };
                break;
            default:
                body = flash ? WHITE : (Color){ 180, 30, 30, 255 };
                eyes = (Color){ 255, 50, 50, 255 };
                break;
        }

        DrawRectangle((int)m->pos.x, (int)m->pos.y, mw, mh, body);

        if (m->kind == MONSTER_TYPE_SPIDER) {
            Color leg = { 80, 30, 100, 255 };
            for (int l = 0; l < 4; l++) {
                float lx = m->pos.x + (l < 2 ? -4 : mw + 1);
                float ly = m->pos.y + 2 + l % 2 * 4;
                DrawLineEx((Vector2){ m->pos.x + (l < 2 ? 0 : mw), ly },
                           (Vector2){ lx, ly + 3 }, 1.5f, leg);
            }
        }

        int eyeX = m->facingLeft ? (int)m->pos.x + 2 : (int)m->pos.x + mw - 5;
        DrawRectangle(eyeX, (int)m->pos.y + 3, 3, 3, eyes);

        if (m->breakTX >= 0 && m->breakTimer > 0.0f) {
            float prog = m->breakTimer / MonsterBreakTime(m);
            DrawRectangle((int)m->pos.x - 2, (int)m->pos.y - 10, mw + 4, 3,
                          (Color){ 40, 40, 40, 200 });
            DrawRectangle((int)m->pos.x - 2, (int)m->pos.y - 10,
                          (int)((mw + 4) * prog), 3, (Color){ 255, 165, 0, 255 });
        }

        float hpFrac = (float)m->hp / m->maxHp;
        DrawRectangle((int)m->pos.x - 2, (int)m->pos.y - 6, mw + 4, 3,
                      (Color){ 60, 0, 0, 200 });
        DrawRectangle((int)m->pos.x - 2, (int)m->pos.y - 6,
                      (int)((mw + 4) * hpFrac), 3, (Color){ 220, 50, 50, 255 });
    }
}
