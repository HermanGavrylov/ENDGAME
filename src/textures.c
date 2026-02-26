#include "header.h"

static Texture2D gTileTex[TILE_COUNT];
static bool      gTileLoaded[TILE_COUNT];

static Texture2D gPlayerTex[CHAR_COUNT];
static bool      gPlayerLoaded[CHAR_COUNT];

static const char *PlayerTexPath(CharClass cls) {
    switch (cls) {
        case CHAR_WARRIOR: return "resource/Dude_Monster.png";
        case CHAR_SCOUT:   return "resource/Owlet_Monster.png";
        case CHAR_TANK:    return "resource/Pink_Monster.png";
        default:           return "resource/Dude_Monster.png";
    }
}

static Texture2D gSwordTex;
static bool      gSwordLoaded = false;
static Texture2D gTorchTex;
static bool      gTorchLoaded = false;

static const char *TilePath(TileType t) {
    switch (t) {
        case TILE_GRASS:  return "resource/tile_0001.png";
        case TILE_DIRT:   return "resource/tile_0000.png";
        case TILE_STONE:  return "resource/tile_0024.png";
        case TILE_COAL:   return "resources/tile_0003.png";
        case TILE_IRON:   return "resource/tile_0012.png";
        case TILE_GOLD:   return "resources/tile_0005.png";
        case TILE_WOOD:   return "resources/tile_0006.png";
        case TILE_LEAVES: return "resources/tile_0007.png";
        case TILE_WATER:  return "resources/tile_0008.png";
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
}

void TexturesUnload(void) {
    for (int i = 1; i < TILE_COUNT; i++)
        if (gTileLoaded[i]) UnloadTexture(gTileTex[i]);
    for (int c = 0; c < CHAR_COUNT; c++)
        if (gPlayerLoaded[c]) UnloadTexture(gPlayerTex[c]);
    if (gSwordLoaded) UnloadTexture(gSwordTex);
    if (gTorchLoaded) UnloadTexture(gTorchTex);
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
    if (!gPlayerLoaded[cls]) {
        DrawRectangle((int)px, (int)py, PLAYER_W, PLAYER_H, tint);
        DrawRectangle((int)px + 1, (int)py, PLAYER_W - 2, PLAYER_W - 2,
                      (Color){ 255, 210, 160, 255 });
        return;
    }
    float tw = (float)gPlayerTex[cls].width;
    float th = (float)gPlayerTex[cls].height;
    float offsetX = px - (tw - PLAYER_W) * 0.5f;
    Rectangle src = { facingLeft ? tw : 0, 0,
                      facingLeft ? -tw : tw, th };
    Rectangle dst = { offsetX, py, tw, th };
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
