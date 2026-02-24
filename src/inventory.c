#include "header.h"

#define SLOT_SIZE   46
#define SLOT_PAD    4
#define HOTBAR_Y    (SCREEN_H - SLOT_SIZE - 10)

static int HotbarStartX(void) {
    return (SCREEN_W - (HOTBAR_SIZE * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD)) / 2;
}

void InvInit(Inventory *inv) {
    memset(inv, 0, sizeof(Inventory));
    inv->activeSlot = 0;
    inv->open       = false;
    inv->hotbar[0].type  = TILE_TORCH;
    inv->hotbar[0].count = 10;
}

static bool StackAdd(ItemStack *s, TileType type) {
    if (s->type == TILE_AIR) {
        s->type  = type;
        s->count = 1;
        return true;
    }
    if (s->type == type && s->count < STACK_MAX) {
        s->count++;
        return true;
    }
    return false;
}

bool InvAddItem(Inventory *inv, TileType type) {
    if (type == TILE_AIR) return false;

    for (int i = 0; i < HOTBAR_SIZE; i++)
        if (inv->hotbar[i].type == type && inv->hotbar[i].count < STACK_MAX)
            return StackAdd(&inv->hotbar[i], type);

    for (int i = 0; i < INV_SIZE; i++)
        if (inv->bag[i].type == type && inv->bag[i].count < STACK_MAX)
            return StackAdd(&inv->bag[i], type);

    for (int i = 0; i < HOTBAR_SIZE; i++)
        if (inv->hotbar[i].type == TILE_AIR)
            return StackAdd(&inv->hotbar[i], type);

    for (int i = 0; i < INV_SIZE; i++)
        if (inv->bag[i].type == TILE_AIR)
            return StackAdd(&inv->bag[i], type);

    return false;
}

bool InvConsumeActive(Inventory *inv) {
    ItemStack *s = &inv->hotbar[inv->activeSlot];
    if (s->type == TILE_AIR || s->count == 0) return false;
    s->count--;
    if (s->count == 0) s->type = TILE_AIR;
    return true;
}

static void DrawSlot(int x, int y, const ItemStack *s, bool active) {
    Color bg     = (Color){ 50, 50, 50, 210 };
    Color border = active ? (Color){ 255, 220, 0, 255 } : (Color){ 100, 100, 100, 200 };

    DrawRectangle(x, y, SLOT_SIZE, SLOT_SIZE, bg);
    DrawRectangleLines(x, y, SLOT_SIZE, SLOT_SIZE, border);

    if (s->type != TILE_AIR) {
        int inner = SLOT_SIZE - 12;
        TileDrawScaled(s->type, x + 6, y + 6, inner);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d", s->count);
        int tw = MeasureText(buf, 10);
        DrawText(buf, x + SLOT_SIZE - tw - 3, y + SLOT_SIZE - 13, 10, (Color){ 220, 220, 220, 255 });
    }
}

static void DrawHotbar(const Inventory *inv) {
    int sx = HotbarStartX();

    DrawRectangle(sx - 4, HOTBAR_Y - 4,
                  HOTBAR_SIZE * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD + 8,
                  SLOT_SIZE + 8,
                  (Color){ 20, 20, 20, 160 });

    for (int i = 0; i < HOTBAR_SIZE; i++) {
        int x = sx + i * (SLOT_SIZE + SLOT_PAD);
        DrawSlot(x, HOTBAR_Y, &inv->hotbar[i], i == inv->activeSlot);

        char num[2] = { '1' + i, '\0' };
        DrawText(num, x + 3, HOTBAR_Y + 3, 11, (Color){ 180, 180, 180, 180 });
    }

    const ItemStack *active = &inv->hotbar[inv->activeSlot];
    if (active->type != TILE_AIR) {
        const char *name = TileName(active->type);
        int tw = MeasureText(name, 16);
        DrawText(name, (SCREEN_W - tw) / 2, HOTBAR_Y - 22, 16, (Color){ 255, 255, 255, 200 });
    }
}

static void DrawBag(const Inventory *inv) {
    int cols   = INV_COLS;
    int totalW = cols * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    int totalH = INV_ROWS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    int ox     = (SCREEN_W - totalW) / 2;
    int oy     = (SCREEN_H - totalH) / 2 - 40;

    DrawRectangle(ox - 10, oy - 36, totalW + 20, totalH + 46 + 20, (Color){ 30, 30, 30, 230 });
    DrawRectangleLines(ox - 10, oy - 36, totalW + 20, totalH + 46 + 20, (Color){ 140, 140, 140, 255 });

    DrawText("Inventory  [E - close]", ox, oy - 26, 16, (Color){ 220, 220, 180, 255 });

    for (int i = 0; i < INV_SIZE; i++) {
        int row = i / cols;
        int col = i % cols;
        int x   = ox + col * (SLOT_SIZE + SLOT_PAD);
        int y   = oy + row * (SLOT_SIZE + SLOT_PAD);
        DrawSlot(x, y, &inv->bag[i], false);
    }

    int hoy = oy + totalH + 12;
    DrawText("Hotbar:", ox, hoy, 14, (Color){ 180, 180, 180, 200 });
    hoy += 18;
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        int x = ox + i * (SLOT_SIZE + SLOT_PAD);
        DrawSlot(x, hoy, &inv->hotbar[i], i == inv->activeSlot);
        char num[2] = { '1' + i, '\0' };
        DrawText(num, x + 3, hoy + 3, 11, (Color){ 180, 180, 180, 180 });
    }
}

void InvDraw(const Inventory *inv) {
    DrawHotbar(inv);
    if (inv->open) DrawBag(inv);

    DrawText("WASD - move | Space - jump | E - inventory | LMB - mine | RMB - place",
             10, 10, 14, (Color){ 255, 255, 255, 180 });
}
