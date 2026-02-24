#include "header.h"

void InputInit(InputState *inp) {
    inp->mineCooldown = 0.0f;
    inp->hoveredTile  = (Vector2){ 0, 0 };
    inp->hoverValid   = false;
}

static Vector2 PlayerCenter(const Player *p) {
    return (Vector2){ p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };
}

static float DistInTiles(Vector2 a, Vector2 b) {
    float dx = (a.x - b.x) / TILE_SIZE;
    float dy = (a.y - b.y) / TILE_SIZE;
    return sqrtf(dx * dx + dy * dy);
}

static bool HasSolidNeighbour(const World *w, int tx, int ty) {
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1,  0, 0 };
    for (int i = 0; i < 4; i++)
        if (WorldIsSolid(w, tx + dx[i], ty + dy[i])) return true;
    return false;
}

static bool OverlapsPlayer(const Player *p, int tx, int ty) {
    Rectangle tile   = { (float)(tx * TILE_SIZE), (float)(ty * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE };
    Rectangle player = { p->pos.x, p->pos.y, PLAYER_W, PLAYER_H };
    return CheckCollisionRecs(player, tile);
}

void InputUpdate(InputState *inp, World *w, const Player *p,
                 const Camera2D *cam, Inventory *inv, float dt) {

    if (inp->mineCooldown > 0.0f) inp->mineCooldown -= dt;

    if (IsKeyPressed(KEY_E)) inv->open = !inv->open;

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f && !inv->open) {
        inv->activeSlot = (inv->activeSlot - (int)wheel + HOTBAR_SIZE) % HOTBAR_SIZE;
    }

    for (int i = 0; i < HOTBAR_SIZE; i++)
        if (IsKeyPressed(KEY_ONE + i)) inv->activeSlot = i;

    Vector2 mouse      = GetMousePosition();
    Vector2 worldMouse = GetScreenToWorld2D(mouse, *cam);
    int tx = (int)floorf(worldMouse.x / TILE_SIZE);
    int ty = (int)floorf(worldMouse.y / TILE_SIZE);

    Vector2 tCenter = { tx * TILE_SIZE + TILE_SIZE * 0.5f, ty * TILE_SIZE + TILE_SIZE * 0.5f };
    float   dist    = DistInTiles(PlayerCenter(p), tCenter);

    inp->hoveredTile = (Vector2){ (float)tx, (float)ty };
    inp->hoverValid  = WorldInBounds(tx, ty) && dist <= MINE_REACH;

    if (inv->open) return;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && inp->hoverValid && inp->mineCooldown <= 0.0f) {
        TileType cur = w->tiles[ty][tx].type;
        if (cur != TILE_AIR && !TileIsLiquid(cur)) {
            InvAddItem(inv, cur);
            w->tiles[ty][tx].type = TILE_AIR;
            inp->mineCooldown = MINE_COOLDOWN;
        }
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && inp->hoverValid) {
        ItemStack *active = &inv->hotbar[inv->activeSlot];
        if (active->type != TILE_AIR &&
            w->tiles[ty][tx].type == TILE_AIR &&
            !OverlapsPlayer(p, tx, ty) &&
            HasSolidNeighbour(w, tx, ty)) {

            w->tiles[ty][tx].type = active->type;
            InvConsumeActive(inv);
        }
    }
}

void InputDrawCursor(const InputState *inp) {
    if (!inp->hoverValid) return;
    int tx = (int)inp->hoveredTile.x;
    int ty = (int)inp->hoveredTile.y;
    DrawRectangle(tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE, (Color){ 255, 255, 255, 60 });
    DrawRectangleLines(tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE, (Color){ 255, 220, 0, 200 });
}
