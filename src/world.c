#include "header.h"

static int perm[512];

static void PerlinInit(unsigned int seed) {
    srand(seed);
    for (int i = 0; i < 256; i++) perm[i] = i;
    for (int i = 255; i > 0; i--) {
        int j   = rand() % (i + 1);
        int tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }
    for (int i = 0; i < 256; i++) perm[256 + i] = perm[i];
}

static float PerlinFade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static float PerlinLerp(float a, float b, float t) { return a + t * (b - a); }

static float PerlinGrad(int hash, float x, float y) {
    switch (hash & 3) {
        case 0:  return  x + y;
        case 1:  return -x + y;
        case 2:  return  x - y;
        default: return -x - y;
    }
}

static float Noise2D(float x, float y) {
    int   xi = (int)floorf(x) & 255;
    int   yi = (int)floorf(y) & 255;
    float xf = x - floorf(x);
    float yf = y - floorf(y);
    float u  = PerlinFade(xf);
    float v  = PerlinFade(yf);

    int aa = perm[perm[xi]     + yi];
    int ab = perm[perm[xi]     + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];

    return PerlinLerp(
        PerlinLerp(PerlinGrad(aa, xf,   yf),   PerlinGrad(ba, xf-1, yf),   u),
        PerlinLerp(PerlinGrad(ab, xf,   yf-1), PerlinGrad(bb, xf-1, yf-1), u),
        v
    );
}

static float OctaveNoise(float x, float y, int octaves, float persistence, float lacunarity) {
    float val = 0, amp = 1, freq = 1, max = 0;
    for (int i = 0; i < octaves; i++) {
        val += Noise2D(x * freq, y * freq) * amp;
        max += amp;
        amp  *= persistence;
        freq *= lacunarity;
    }
    return val / max;
}

static float Normalize01(float v) { return (v + 1.0f) * 0.5f; }
static float SeedOffset(unsigned int seed) { return (seed % 10000) * 0.0001f; }

static int SurfaceHeight(int x, float soff) {
    float n = OctaveNoise(x * 0.008f, soff, 5, 0.55f, 2.1f);
    return SURFACE_LEVEL + (int)(Normalize01(n) * SURFACE_RANGE - SURFACE_RANGE * 0.5f);
}

static bool IsCave(int x, int y, float soff) {
    float n = Normalize01(OctaveNoise(x * 0.05f, y * 0.05f + soff * 3.7f, CAVE_OCTAVES, 0.5f, 2.0f));
    return n > CAVE_THRESHOLD;
}

static TileType OreAt(int x, int y, int surface, float soff) {
    int depth = y - surface;
    if (depth < ORE_MIN_DEPTH) return TILE_STONE;

    float nCoal = Normalize01(OctaveNoise(x * 0.15f + soff * 1.1f, y * 0.15f,            2, 0.5f, 2.0f));
    float nIron = Normalize01(OctaveNoise(x * 0.15f + soff * 2.3f, y * 0.15f + soff,     2, 0.5f, 2.0f));
    float nGold = Normalize01(OctaveNoise(x * 0.15f + soff * 4.7f, y * 0.15f + soff * 2, 2, 0.5f, 2.0f));

    if (depth > 60 && nGold > ORE_GOLD_THR) return TILE_GOLD;
    if (depth > 30 && nIron > ORE_IRON_THR) return TILE_IRON;
    if (nCoal > ORE_COAL_THR)               return TILE_COAL;

    return TILE_STONE;
}

static void PlaceTree(World *w, int x, int surfY) {
    int height = TREE_MIN_HEIGHT + rand() % (TREE_MAX_HEIGHT - TREE_MIN_HEIGHT + 1);

    for (int i = 1; i <= height; i++) {
        int ty = surfY - i;
        if (!WorldInBounds(x, ty)) break;
        w->tiles[ty][x].type = TILE_WOOD;
    }

    int topY   = surfY - height;
    int radius = 2;

    for (int dy = -radius - 1; dy <= 0; dy++) {
        int r = (dy == -(radius + 1)) ? 1 : radius;
        for (int dx = -r; dx <= r; dx++) {
            int lx = x + dx;
            int ly = topY + dy;
            if (!WorldInBounds(lx, ly)) continue;
            if (w->tiles[ly][lx].type == TILE_AIR)
                w->tiles[ly][lx].type = TILE_LEAVES;
        }
    }
}

static void PlaceLake(World *w, int cx, int surfY) {
    int halfW = LAKE_MIN_WIDTH / 2 + rand() % ((LAKE_MAX_WIDTH - LAKE_MIN_WIDTH) / 2 + 1);

    for (int dx = -halfW; dx <= halfW; dx++) {
        int lx = cx + dx;
        if (!WorldInBounds(lx, surfY)) continue;

        int depth = LAKE_DEPTH - (int)(abs(dx) * LAKE_DEPTH / (float)(halfW + 1));
        if (depth < 1) depth = 1;

        for (int dy = 0; dy < depth; dy++) {
            int ly = surfY + dy;
            if (!WorldInBounds(lx, ly)) continue;

            if (dy == 0 && w->tiles[ly][lx].type == TILE_GRASS)
                w->tiles[ly][lx].type = TILE_WATER;
            else if (dy > 0 && (w->tiles[ly][lx].type == TILE_DIRT ||
                                w->tiles[ly][lx].type == TILE_GRASS))
                w->tiles[ly][lx].type = TILE_WATER;
        }

        int bedY = surfY + depth;
        if (WorldInBounds(lx, bedY) && w->tiles[bedY][lx].type == TILE_GRASS)
            w->tiles[bedY][lx].type = TILE_DIRT;
    }
}

static void GenerateTrees(World *w, const int *surface) {
    int x = 6 + rand() % 10;
    while (x < WORLD_W - 6) {
        if (surface[x] > 2 && surface[x] < WORLD_H - 2 &&
            w->tiles[surface[x]][x].type != TILE_WATER)
            PlaceTree(w, x, surface[x]);
        x += 10 + rand() % 14;
    }
}

static void GenerateLakes(World *w, const int *surface) {
    int x = 20 + rand() % 30;
    while (x < WORLD_W - 20) {
        float n = Normalize01(Noise2D(x * 0.03f, 99.0f));
        if (n > 0.55f)
            PlaceLake(w, x, surface[x]);
        x += 30 + rand() % 40;
    }
}

void WorldGenerate(World *w) {
    unsigned int seed = (unsigned int)time(NULL);
    PerlinInit(seed);
    float soff = SeedOffset(seed);
    memset(w->tiles, TILE_AIR, sizeof(w->tiles));

    int surface[WORLD_W];
    for (int x = 0; x < WORLD_W; x++)
        surface[x] = SurfaceHeight(x, soff);

    for (int x = 0; x < WORLD_W; x++) {
        int surf = surface[x];
        for (int y = 0; y < WORLD_H; y++) {
            if (y < surf) continue;
            if (y == surf) {
                w->tiles[y][x].type = TILE_GRASS;
            } else if (y < surf + DIRT_DEPTH) {
                w->tiles[y][x].type = TILE_DIRT;
            } else {
                if (IsCave(x, y, soff)) continue;
                w->tiles[y][x].type = OreAt(x, y, surf, soff);
            }
        }
    }

    GenerateLakes(w, surface);
    GenerateTrees(w, surface);
}

bool WorldInBounds(int tx, int ty) {
    return tx >= 0 && tx < WORLD_W && ty >= 0 && ty < WORLD_H;
}

bool WorldIsSolid(const World *w, int tx, int ty) {
    if (!WorldInBounds(tx, ty)) return true;
    TileType t = w->tiles[ty][tx].type;
    return t != TILE_AIR && !TileIsLiquid(t) && !TileIsPassable(t);
}

void WorldDraw(const World *w, const Camera2D *cam) {
    float visHalfW = (SCREEN_W / cam->zoom) * 0.5f;
    float visHalfH = (SCREEN_H / cam->zoom) * 0.5f;

    int startX = (int)floorf((cam->target.x - visHalfW) / TILE_SIZE) - 1;
    int startY = (int)floorf((cam->target.y - visHalfH) / TILE_SIZE) - 1;
    int endX   = (int)floorf((cam->target.x + visHalfW) / TILE_SIZE) + 2;
    int endY   = (int)floorf((cam->target.y + visHalfH) / TILE_SIZE) + 2;

    if (startX < 0)      startX = 0;
    if (startY < 0)      startY = 0;
    if (endX > WORLD_W)  endX   = WORLD_W;
    if (endY > WORLD_H)  endY   = WORLD_H;

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            TileType t = w->tiles[y][x].type;
            if (t == TILE_AIR) continue;
            TileDraw(t, x * TILE_SIZE, y * TILE_SIZE);
        }
    }
}
