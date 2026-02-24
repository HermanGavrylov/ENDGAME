#include "header.h"

#define PATH_LEN         32
#define BFS_MAX          4096
#define BFS_W            80
#define BFS_H            60
#define PATH_RETARGET    1.2f

void MonstersInit(Monsters *ms) {
    memset(ms, 0, sizeof(Monsters));
    ms->count = 0;
}

static void SpawnMonster(Monsters *ms, Vector2 pos) {
    if (ms->count >= MAX_MONSTERS) return;
    Monster *m       = &ms->list[ms->count++];
    m->pos           = pos;
    m->vel           = (Vector2){ 0, 0 };
    m->hp            = MONSTER_HP;
    m->iframes       = 0.0f;
    m->alive         = true;
    m->facingLeft    = false;
    m->breakTimer    = 0.0f;
    m->breakCooldown = 0.0f;
    m->breakTX       = -1;
    m->breakTY       = -1;
    m->pathLen       = 0;
    m->pathStep      = 0;
    m->retargetTimer = 0.0f;
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

    static signed char  prevX[BFS_H][BFS_W];
    static signed char  prevY[BFS_H][BFS_W];
    static bool         visited[BFS_H][BFS_W];
    memset(visited, 0, sizeof(visited));

    BFSNode queue[BFS_MAX];
    int head = 0, tail = 0;

    int sx = ptx - ox, sy = pty - oy;
    int ex = gtx - ox, ey = gty - oy;

    if (sx < 0 || sx >= BFS_W || sy < 0 || sy >= BFS_H) { m->pathLen = 0; return; }
    if (ex < 0 || ex >= BFS_W || ey < 0 || ey >= BFS_H) { m->pathLen = 0; return; }

    visited[sy][sx] = true;
    prevX[sy][sx]   = -1;
    prevY[sy][sx]   = -1;
    queue[tail++]   = (BFSNode){ (short)sx, (short)sy };

    bool found = false;

    while (head < tail && tail < BFS_MAX) {
        BFSNode cur = queue[head++];
        if (cur.x == ex && cur.y == ey) { found = true; break; }

        int dx[] = { -1, 1, 0, 0, -1, 1, -1, 1 };
        int dy[] = {  0, 0,-1, 1, -1,-1,  1,  1 };

        for (int d = 0; d < 8; d++) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];
            if (nx < 0 || nx >= BFS_W || ny < 0 || ny >= BFS_H) continue;
            if (visited[ny][nx]) continue;

            int wx = nx + ox, wy = ny + oy;

            bool walkable = BFSWalkable(w, wx, wy) && BFSWalkable(w, wx, wy + 1);
            bool grounded = WorldIsSolid(w, wx, wy + 2);
            bool jump1    = (dy[d] < 0) && WorldIsSolid(w, cur.x + ox, cur.y + oy + 2);
            bool diagonal = (dx[d] != 0 && dy[d] != 0);

            if (!walkable) continue;
            if (!grounded && !(ny == ey)) continue;
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
    int rawLen = 0;
    int cx = ex, cy = ey;
    while ((cx != sx || cy != sy) && rawLen < PATH_LEN * 2) {
        raw[rawLen++] = (BFSNode){ (short)(cx + ox), (short)(cy + oy) };
        int nx = cx + prevX[cy][cx];
        int ny = cy + prevY[cy][cx];
        cx = nx; cy = ny;
    }

    int len = 0;
    for (int i = rawLen - 1; i >= 0 && len < PATH_LEN; i--)
        m->path[len++] = (Vector2){ raw[i].x * TILE_SIZE, raw[i].y * TILE_SIZE };
    m->pathLen  = len;
    m->pathStep = 0;
}

static bool WallAhead(const World *w, const Monster *m, bool goingLeft, int *outTX, int *outTY) {
    int checkX = goingLeft
        ? (int)floorf((m->pos.x - 2.0f) / TILE_SIZE)
        : (int)floorf((m->pos.x + MONSTER_W + 1.0f) / TILE_SIZE);

    if (checkX < 0 || checkX >= WORLD_W) return false;

    int tyFeet = (int)floorf((m->pos.y + MONSTER_H - 2.0f) / TILE_SIZE);
    int tyMid  = (int)floorf((m->pos.y + MONSTER_H * 0.5f) / TILE_SIZE);

    for (int ty = tyMid; ty <= tyFeet; ty++) {
        if (ty < 0 || ty >= WORLD_H) continue;
        if (WorldIsSolid(w, checkX, ty)) {
            *outTX = checkX;
            *outTY = ty;
            return true;
        }
    }
    return false;
}

static bool WallTooTall(const World *w, int wallTX, int wallTY) {
    int above = wallTY - 1;
    if (above < 0) return false;
    return WorldIsSolid(w, wallTX, above);
}

void MonstersUpdate(Monsters *ms, Player *p, World *w, float dt) {
    Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };

    if (p->iframes > 0.0f) p->iframes -= dt;

    for (int i = 0; i < ms->count; i++) {
        Monster *m = &ms->list[i];
        if (!m->alive) continue;

        if (m->iframes       > 0.0f) m->iframes       -= dt;
        if (m->breakCooldown > 0.0f) m->breakCooldown  -= dt;
        m->retargetTimer += dt;

        Vector2 mCenter = { m->pos.x + MONSTER_W * 0.5f, m->pos.y + MONSTER_H * 0.5f };
        float   dx      = pCenter.x - mCenter.x;
        float   dist    = fabsf(dx) + fabsf(pCenter.y - mCenter.y);

        int mtx = (int)floorf(mCenter.x / TILE_SIZE);
        int mty = (int)floorf((m->pos.y + MONSTER_H - 1.0f) / TILE_SIZE);
        int ptx = (int)floorf(pCenter.x / TILE_SIZE);
        int pty = (int)floorf((p->pos.y + PLAYER_H - 1.0f) / TILE_SIZE);

        bool onGround = false;
        int bx0 = (int)floorf((m->pos.x + 1.0f) / TILE_SIZE);
        int bx1 = (int)floorf((m->pos.x + MONSTER_W - 1.0f) / TILE_SIZE);
        int by  = (int)floorf((m->pos.y + MONSTER_H + 0.5f) / TILE_SIZE);
        for (int tx = bx0; tx <= bx1; tx++)
            if (WorldIsSolid(w, tx, by)) { onGround = true; break; }

        if (m->retargetTimer >= PATH_RETARGET) {
            m->retargetTimer = 0.0f;
            BuildPath(w, m, mtx, mty, ptx, pty);
        }

        bool hasPath    = m->pathLen > 0 && m->pathStep < m->pathLen;
        bool digging    = false;
        float moveX     = 0.0f;
        bool  goLeft    = dx < 0;

        if (hasPath) {
            Vector2 target = m->path[m->pathStep];
            float   tdx    = (target.x + TILE_SIZE * 0.5f) - mCenter.x;
            float   tdy    = (target.y + TILE_SIZE * 0.5f) - mCenter.y;
            float   tdist  = fabsf(tdx) + fabsf(tdy);

            if (tdist < TILE_SIZE * 0.8f) {
                m->pathStep++;
                hasPath = m->pathStep < m->pathLen;
            }

            if (hasPath) {
                target  = m->path[m->pathStep];
                tdx     = (target.x + TILE_SIZE * 0.5f) - mCenter.x;
                tdy     = (target.y + TILE_SIZE * 0.5f) - mCenter.y;
                goLeft  = tdx < 0;
                moveX   = goLeft ? -MONSTER_SPEED : MONSTER_SPEED;

                if (tdy < -TILE_SIZE * 0.5f && onGround && m->vel.y == 0.0f)
                    m->vel.y = JUMP_FORCE * 0.85f;
            }
        } else {
            moveX = goLeft ? -MONSTER_SPEED : MONSTER_SPEED;
        }

        int wallTX = -1, wallTY = -1;
        if (WallAhead(w, m, goLeft, &wallTX, &wallTY)) {
            if (!WallTooTall(w, wallTX, wallTY)) {
                if (onGround && m->vel.y == 0.0f)
                    m->vel.y = JUMP_FORCE * 0.85f;
            } else if (dist < MONSTER_DIG_RANGE) {
                digging = true;

                if (wallTX != m->breakTX || wallTY != m->breakTY) {
                    m->breakTX    = wallTX;
                    m->breakTY    = wallTY;
                    m->breakTimer = 0.0f;
                }

                if (m->breakCooldown <= 0.0f) {
                    m->breakTimer += dt;
                    if (m->breakTimer >= MONSTER_BREAK_TIME) {
                        if (WorldInBounds(m->breakTX, m->breakTY) &&
                            TileBreakable(w->tiles[m->breakTY][m->breakTX].type)) {
                            w->tiles[m->breakTY][m->breakTX].type = TILE_AIR;
                            int above = m->breakTY - 1;
                            if (above >= 0 &&
                                TileBreakable(w->tiles[above][m->breakTX].type))
                                w->tiles[above][m->breakTX].type = TILE_AIR;
                        }
                        m->breakTimer    = 0.0f;
                        m->breakCooldown = MONSTER_BREAK_COOLDOWN;
                        m->breakTX       = -1;
                        m->breakTY       = -1;
                        m->retargetTimer = PATH_RETARGET;
                    }
                }
            }
        }

        if (!digging) {
            m->vel.x      = moveX;
            m->facingLeft = goLeft;
        } else {
            m->vel.x = 0;
        }

        if (onGround && fabsf(dx) < MONSTER_SIGHT * 0.5f
            && pCenter.y < mCenter.y && m->vel.y == 0.0f)
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

        bool  flash = (m->iframes > 0.0f) && ((int)(m->iframes * 10) % 2 == 0);
        Color body  = flash ? WHITE : (Color){ 180, 30, 30, 255 };
        Color eyes  = { 255, 50, 50, 255 };

        DrawRectangle((int)m->pos.x, (int)m->pos.y, MONSTER_W, MONSTER_H, body);

        int eyeX = m->facingLeft
                   ? (int)m->pos.x + 2
                   : (int)m->pos.x + MONSTER_W - 5;
        DrawRectangle(eyeX, (int)m->pos.y + 4, 3, 3, eyes);

        if (m->breakTX >= 0 && m->breakTimer > 0.0f) {
            float prog = m->breakTimer / MONSTER_BREAK_TIME;
            int   bw   = MONSTER_W + 4;
            int   bx   = (int)m->pos.x - 2;
            int   bary = (int)m->pos.y - 10;
            DrawRectangle(bx, bary, bw, 3, (Color){ 40, 40, 40, 200 });
            DrawRectangle(bx, bary, (int)(bw * prog), 3, (Color){ 255, 165, 0, 255 });
        }

        float hpFrac = (float)m->hp / MONSTER_HP;
        int   bw     = MONSTER_W + 4;
        int   bx     = (int)m->pos.x - 2;
        int   bary   = (int)m->pos.y - 6;
        DrawRectangle(bx, bary, bw, 3, (Color){ 60, 0, 0, 200 });
        DrawRectangle(bx, bary, (int)(bw * hpFrac), 3, (Color){ 220, 50, 50, 255 });
    }
}
