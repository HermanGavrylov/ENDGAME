#include "header.h"

static Texture2D gTileTex[TILE_COUNT];
static bool      gTileLoaded[TILE_COUNT];

static Texture2D gPlayerTex[CHAR_COUNT];
static bool      gPlayerLoaded[CHAR_COUNT];

static const char *PlayerTexPath(CharClass cls) {
    switch (cls) {
        case CHAR_WARRIOR: return "resource/warrior.png";
        case CHAR_SCOUT:   return "resource/scout.png";
        case CHAR_TANK:    return "resource/tank.png";
        default:           return "resource/warrior.png";
    }
}

static Texture2D gSwordTex;
static bool      gSwordLoaded = false;
static Texture2D gTorchTex;
static bool      gTorchLoaded = false;
static Texture2D gLifePotTex;
static bool      gLifePotLoaded = false;

static const char *TilePath(TileType t) {
    switch (t) {
        case TILE_GRASS:  return "resource/grass.png";
        case TILE_DIRT:   return "resource/dirt.png";
        case TILE_STONE:  return "resource/rock.png";
        case TILE_COAL:   return "resource/coal.png";
        case TILE_IRON:   return "resource/iron.png";
        case TILE_GOLD:   return "resource/gold.png";
        case TILE_WOOD:   return "resource/trunk.png";
        case TILE_LEAVES: return "resource/leaves.png";
        case TILE_WATER:  return "resource/tile_0037.png";
        case TILE_MEAT:   return "resource/meat.png";
        default:          return NULL;
    }
}

void TexturesLoad(void) {
    memset(gTileLoaded,   0, sizeof(gTileLoaded));
    memset(gPlayerLoaded, 0, sizeof(gPlayerLoaded));

    for (int i = 1; i < TILE_COUNT; i++) {
        const char *path = TilePath((TileType)i);
        if (path && FileExists(path)) {
            gTileTex[i]    = LoadTexture(path);
            gTileLoaded[i] = true;
        }
    }

    for (int c = 0; c < CHAR_COUNT; c++) {
        const char *path = PlayerTexPath((CharClass)c);
        if (FileExists(path)) {
            gPlayerTex[c]    = LoadTexture(path);
            gPlayerLoaded[c] = true;
        }
    }

    if (FileExists("resource/sword.png")) {
        gSwordTex    = LoadTexture("resource/sword.png");
        gSwordLoaded = true;
    }
    if (FileExists("resource/torch.png")) {
        gTorchTex    = LoadTexture("resource/torch.png");
        gTorchLoaded = true;
    }
    if (FileExists("resource/LifePot.png")) {
        gLifePotTex    = LoadTexture("resource/LifePot.png");
        gLifePotLoaded = true;
    }
}

void TexturesUnload(void) {
    for (int i = 1; i < TILE_COUNT; i++)
        if (gTileLoaded[i]) UnloadTexture(gTileTex[i]);
    for (int c = 0; c < CHAR_COUNT; c++)
        if (gPlayerLoaded[c]) UnloadTexture(gPlayerTex[c]);
    if (gSwordLoaded)   UnloadTexture(gSwordTex);
    if (gTorchLoaded)   UnloadTexture(gTorchTex);
    if (gLifePotLoaded) UnloadTexture(gLifePotTex);
}

void TileDrawScaled(TileType type, int px, int py, int size) {
    if (type == TILE_AIR) return;

    if (type == TILE_SWORD) {
        if (gSwordLoaded) {
            Rectangle src = { 0, 0, (float)gSwordTex.width, (float)gSwordTex.height };
            Rectangle dst = { (float)px, (float)py, (float)size, (float)size };
            DrawTexturePro(gSwordTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, size, size, TileColor(TILE_SWORD));
        }
        return;
    }

    if (type == TILE_TORCH) {
        if (gTorchLoaded) {
            Rectangle src = { 0, 0, (float)gTorchTex.width, (float)gTorchTex.height };
            Rectangle dst = { (float)px, (float)py, (float)size, (float)size };
            DrawTexturePro(gTorchTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, size, size, TileColor(TILE_TORCH));
        }
        return;
    }

    if (type == TILE_LIFEPOT) {
        if (gLifePotLoaded) {
            Rectangle src = { 0, 0, (float)gLifePotTex.width, (float)gLifePotTex.height };
            Rectangle dst = { (float)px, (float)py, (float)size, (float)size };
            DrawTexturePro(gLifePotTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, size, size, TileColor(TILE_LIFEPOT));
        }
        return;
    }

    if (gTileLoaded[type]) {
        Rectangle src = { 0, 0, TILE_SIZE, TILE_SIZE };
        Rectangle dst = { (float)px, (float)py, (float)size, (float)size };
        DrawTexturePro(gTileTex[type], src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        Color c = TileColor(type);
        DrawRectangle(px, py, size, size, c);
        if (!TileIsLiquid(type))
            DrawRectangleLines(px, py, size, size, (Color){ 0, 0, 0, 40 });
    }
}

void TileDraw(TileType type, int px, int py) {
    if (type == TILE_AIR) return;

    if (type == TILE_SWORD) {
        if (gSwordLoaded) {
            Rectangle src = { 0, 0, (float)gSwordTex.width, (float)gSwordTex.height };
            Rectangle dst = { (float)px, (float)py, (float)TILE_SIZE, (float)TILE_SIZE };
            DrawTexturePro(gSwordTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, TileColor(TILE_SWORD));
        }
        return;
    }

    if (type == TILE_TORCH) {
        if (gTorchLoaded) {
            Rectangle src = { 0, 0, (float)gTorchTex.width, (float)gTorchTex.height };
            Rectangle dst = { (float)px, (float)py, (float)TILE_SIZE, (float)TILE_SIZE };
            DrawTexturePro(gTorchTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, TileColor(TILE_TORCH));
        }
        return;
    }

    if (type == TILE_LIFEPOT) {
        if (gLifePotLoaded) {
            Rectangle src = { 0, 0, (float)gLifePotTex.width, (float)gLifePotTex.height };
            Rectangle dst = { (float)px, (float)py, (float)TILE_SIZE, (float)TILE_SIZE };
            DrawTexturePro(gLifePotTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, TileColor(TILE_LIFEPOT));
        }
        return;
    }

    if (gTileLoaded[type]) {
        DrawTextureEx(gTileTex[type], (Vector2){ (float)px, (float)py }, 0.0f, 1.0f, WHITE);
    } else {
        Color c = TileColor(type);
        DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, c);
        if (!TileIsLiquid(type))
            DrawRectangleLines(px, py, TILE_SIZE, TILE_SIZE, (Color){ 0, 0, 0, 40 });
    }
}

void PlayerSpriteDraw(float px, float py, bool facingLeft, Color tint) {
    PlayerSpriteDrawClass(px, py, facingLeft, tint, CHAR_WARRIOR);
}

void PlayerSpriteDrawClass(float px, float py, bool facingLeft,
                            Color tint, CharClass cls) {
    if (!gPlayerLoaded[cls]) { return; }

    float tw = (float)gPlayerTex[cls].width;
    float th = (float)gPlayerTex[cls].height;

    float scale = (float)PLAYER_H / th;
    float drawW = tw * scale;
    float drawH = th * scale;

    float drawX = px + (PLAYER_W - drawW) * 0.5f;
    float drawY = py;

    Rectangle src = {
        facingLeft ? tw : 0,
        0,
        facingLeft ? -tw : tw,
        th
    };
    Rectangle dst = { drawX, drawY, drawW, drawH };
    DrawTexturePro(gPlayerTex[cls], src, dst, (Vector2){0, 0}, 0.0f, tint);
}

void SwordDrawInHand(float cx, float cy, float angle, float reach, bool facingLeft) {
    if (!gSwordLoaded) return;

    float dir       = facingLeft ? -1.0f : 1.0f;
    float rad       = angle * DEG2RAD;
    float drawAngle = atan2f(sinf(rad) * dir, cosf(rad) * dir) * RAD2DEG;

    float texW = reach;
    float texH = TILE_SIZE;

    float startX = cx + dir * (PLAYER_W * 0.5f);
    float startY = cy;

    Rectangle src = {
        facingLeft ? (float)gSwordTex.width : 0,
        0,
        facingLeft ? -(float)gSwordTex.width : (float)gSwordTex.width,
        (float)gSwordTex.height
    };
    Rectangle dst    = { startX, startY, texW, texH };
    Vector2   origin = { facingLeft ? texW : 0, texH * 0.5f };

    DrawTexturePro(gSwordTex, src, dst, origin, drawAngle, WHITE);
}

void TorchDrawInHand(float cx, float cy, bool facingLeft) {
    float dir  = facingLeft ? -1.0f : 1.0f;
    float texW = TILE_SIZE * 0.8f;
    float texH = TILE_SIZE * 0.8f;
    float hx   = cx + dir * (PLAYER_W * 0.5f + 1.0f);
    float hy   = cy - texH * 0.3f;

    Rectangle src  = { 0, 0, (float)gTorchTex.width, (float)gTorchTex.height };
    Rectangle dst  = { hx, hy, texW, texH };
    Vector2 origin = { facingLeft ? texW : 0, texH * 0.5f };

    if (gTorchLoaded)
        DrawTexturePro(gTorchTex, src, dst, origin, 0.0f, WHITE);
    else
        DrawRectangle((int)hx, (int)hy, (int)texW, (int)texH,
                      (Color){ 255, 200, 60, 255 });
}

Texture2D PlayerTexGet(CharClass cls) {
    return gPlayerTex[cls];
}
bool PlayerTexIsLoaded(CharClass cls) {
    return gPlayerLoaded[cls];
}
