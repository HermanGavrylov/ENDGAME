#include "header.h"

static Texture2D gTileTex[TILE_COUNT];
static bool      gTileLoaded[TILE_COUNT];

static Texture2D gPlayerTex;
static bool      gPlayerLoaded = false;

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
    memset(gTileLoaded, 0, sizeof(gTileLoaded));
    for (int i = 1; i < TILE_COUNT; i++) {
        const char *path = TilePath((TileType)i);
        if (path && FileExists(path)) {
            gTileTex[i]    = LoadTexture(path);
            gTileLoaded[i] = true;
        }
    }

    if (FileExists("resource/tile_0098.png")) {
        gPlayerTex    = LoadTexture("resource/tile_0098.png");
        gPlayerLoaded = true;
    }
}

void TexturesUnload(void) {
    for (int i = 1; i < TILE_COUNT; i++)
        if (gTileLoaded[i]) UnloadTexture(gTileTex[i]);
    if (gPlayerLoaded) UnloadTexture(gPlayerTex);
}

void TileDrawScaled(TileType type, int px, int py, int size) {
    if (type == TILE_AIR) return;
    if (gTileLoaded[type]) {
        Rectangle src = { 0, 0, TILE_SIZE, TILE_SIZE };
        Rectangle dst = { (float)px, (float)py, (float)size, (float)size };
        DrawTexturePro(gTileTex[type], src, dst, (Vector2){0,0}, 0.0f, WHITE);
    } else {
        Color c = TileColor(type);
        DrawRectangle(px, py, size, size, c);
        if (!TileIsLiquid(type))
            DrawRectangleLines(px, py, size, size, (Color){ 0, 0, 0, 40 });
    }
}

void TileDraw(TileType type, int px, int py) {
    if (type == TILE_AIR) return;
    if (gTileLoaded[type]) {
        DrawTextureEx(gTileTex[type], (Vector2){ (float)px, (float)py }, 0.0f, 1.0f, WHITE);
    } else {
        Color c = TileColor(type);
        DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, c);
        if (!TileIsLiquid(type))
            DrawRectangleLines(px, py, TILE_SIZE, TILE_SIZE, (Color){ 0, 0, 0, 40 });
    }
}

void PlayerSpriteDraw(float px, float py, bool facingLeft) {
    if (!gPlayerLoaded) {
        DrawRectangle((int)px, (int)py, PLAYER_W, PLAYER_H, (Color){ 30, 120, 220, 255 });
        DrawRectangle((int)px + 1, (int)py, PLAYER_W - 2, PLAYER_W - 2, (Color){ 255, 210, 160, 255 });
        return;
    }

    float offsetX = px - (PLAYER_SPRITE_W - PLAYER_W) * 0.5f;

    Rectangle src = {
        0, 0,
        facingLeft ? -(float)PLAYER_SPRITE_W : (float)PLAYER_SPRITE_W,
        (float)PLAYER_SPRITE_H
    };

    Rectangle dst = {
        offsetX, py,
        (float)PLAYER_SPRITE_W,
        (float)PLAYER_H
    };

    DrawTexturePro(gPlayerTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
}
