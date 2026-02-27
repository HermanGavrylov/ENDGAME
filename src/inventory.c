#include "header.h"


static int HotbarStartX(void) {
    return (SCREEN_W - (HOTBAR_SIZE * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD)) / 2;
}

void InvInit(Inventory *inv) {
    memset(inv, 0, sizeof(Inventory));
    inv->activeSlot     = 0;
    inv->open           = false;
    inv->dragSlot       = DRAG_NONE;
    inv->dragFromHotbar = false;
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

static Rectangle SlotRect(int x, int y) {
    return (Rectangle){ (float)x, (float)y, (float)SLOT_SIZE, (float)SLOT_SIZE };
}

static void GetBagLayout(int *ox, int *oy, int *hoy) {
    int totalW = INV_COLS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    int totalH = INV_ROWS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    *ox  = (SCREEN_W - totalW) / 2;
    *oy  = (SCREEN_H - totalH) / 2 - 40;
    *hoy = *oy + totalH + 30;
}

static void SwapSlots(ItemStack *a, ItemStack *b) {
    ItemStack tmp = *a; *a = *b; *b = tmp;
}

static void DrawSlot(int x, int y, const ItemStack *s, bool active, bool dragging) {
    Color bg     = dragging ? (Color){ 80, 80, 30, 210 } : (Color){ 50, 50, 50, 210 };
    Color border = active   ? (Color){ 255, 220,  0, 255 }
                 : dragging ? (Color){ 255, 200, 50, 255 }
                            : (Color){ 100, 100, 100, 200 };

    DrawRectangle(x, y, SLOT_SIZE, SLOT_SIZE, bg);
    DrawRectangleLines(x, y, SLOT_SIZE, SLOT_SIZE, border);

    if (s->type != TILE_AIR && !dragging) {
        TileDrawScaled(s->type, x + 6, y + 6, SLOT_SIZE - 12);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", s->count);
        int tw = MeasureText(buf, 10);
        DrawText(buf, x + SLOT_SIZE - tw - 3, y + SLOT_SIZE - 13, 10,
                 (Color){ 220, 220, 220, 255 });
    }
}

static void DrawHotbar(const Inventory *inv) {
    int sx = HotbarStartX();

    DrawRectangle(sx - 4, HOTBAR_Y - 4,
                  HOTBAR_SIZE * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD + 8, SLOT_SIZE + 8,
                  (Color){ 20, 20, 20, 160 });

    for (int i = 0; i < HOTBAR_SIZE; i++) {
        int  x       = sx + i * (SLOT_SIZE + SLOT_PAD);
        bool isDrag  = (inv->dragSlot == i) && inv->dragFromHotbar;
        DrawSlot(x, HOTBAR_Y, &inv->hotbar[i], i == inv->activeSlot, isDrag);
        char num[2]  = { '1' + i, '\0' };
        DrawText(num, x + 3, HOTBAR_Y + 3, 11, (Color){ 180, 180, 180, 180 });
    }

    const ItemStack *active = &inv->hotbar[inv->activeSlot];
    if (active->type != TILE_AIR && inv->dragSlot == DRAG_NONE) {
        const char *name = TileName(active->type);
        int tw = MeasureText(name, 16);
        DrawText(name, (SCREEN_W - tw) / 2, HOTBAR_Y - 22, 16,
                 (Color){ 255, 255, 255, 200 });
    }
}

static void DrawBag(const Inventory *inv) {
    int ox, oy, hoy;
    GetBagLayout(&ox, &oy, &hoy);

    int totalW = INV_COLS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    int totalH = INV_ROWS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;

    DrawRectangle(ox - 10, oy - 36, totalW + 20, totalH + 46 + 20 + SLOT_SIZE,
                  (Color){ 30, 30, 30, 230 });
    DrawRectangleLines(ox - 10, oy - 36, totalW + 20, totalH + 46 + 20 + SLOT_SIZE,
                       (Color){ 140, 140, 140, 255 });

    DrawText("Inventory  [E - close]", ox, oy - 26, 16, (Color){ 220, 220, 180, 255 });

    for (int i = 0; i < INV_SIZE; i++) {
        int row    = i / INV_COLS;
        int col    = i % INV_COLS;
        int x      = ox + col * (SLOT_SIZE + SLOT_PAD);
        int y      = oy + row * (SLOT_SIZE + SLOT_PAD);
        bool isDrag = (inv->dragSlot == i) && !inv->dragFromHotbar;
        DrawSlot(x, y, &inv->bag[i], false, isDrag);
    }

    DrawText("Hotbar:", ox, hoy - 16, 14, (Color){ 180, 180, 180, 200 });
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        int  x      = ox + i * (SLOT_SIZE + SLOT_PAD);
        bool isDrag = (inv->dragSlot == i) && inv->dragFromHotbar;
        DrawSlot(x, hoy, &inv->hotbar[i], i == inv->activeSlot, isDrag);
        char num[2] = { '1' + i, '\0' };
        DrawText(num, x + 3, hoy + 3, 11, (Color){ 180, 180, 180, 180 });
    }
}

static void DrawDraggedItem(const Inventory *inv) {
    if (inv->dragSlot == DRAG_NONE || inv->dragItem.type == TILE_AIR) return;
    Vector2 mp = GetMousePosition();
    int sz = SLOT_SIZE - 8;
    TileDrawScaled(inv->dragItem.type, (int)mp.x - sz / 2, (int)mp.y - sz / 2, sz);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", inv->dragItem.count);
    DrawText(buf, (int)mp.x + sz / 2 - 8, (int)mp.y + sz / 2 - 14, 10,
             (Color){ 255, 255, 80, 255 });
}

void InvHandleDrag(Inventory *inv) {
    if (!inv->open) return;

    Vector2 mp = GetMousePosition();
    int ox, oy, hoy;
    GetBagLayout(&ox, &oy, &hoy);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < INV_SIZE; i++) {
            int x = ox + (i % INV_COLS) * (SLOT_SIZE + SLOT_PAD);
            int y = oy + (i / INV_COLS) * (SLOT_SIZE + SLOT_PAD);
            if (CheckCollisionPointRec(mp, SlotRect(x, y)) &&
                inv->bag[i].type != TILE_AIR) {
                inv->dragSlot       = i;
                inv->dragFromHotbar = false;
                inv->dragItem       = inv->bag[i];
                inv->bag[i]         = (ItemStack){ TILE_AIR, 0 };
                return;
            }
        }
        for (int i = 0; i < HOTBAR_SIZE; i++) {
            int x = ox + i * (SLOT_SIZE + SLOT_PAD);
            if (CheckCollisionPointRec(mp, SlotRect(x, hoy)) &&
                inv->hotbar[i].type != TILE_AIR) {
                inv->dragSlot       = i;
                inv->dragFromHotbar = true;
                inv->dragItem       = inv->hotbar[i];
                inv->hotbar[i]      = (ItemStack){ TILE_AIR, 0 };
                return;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && inv->dragSlot != DRAG_NONE) {
        bool placed = false;

        for (int i = 0; i < INV_SIZE && !placed; i++) {
            int x = ox + (i % INV_COLS) * (SLOT_SIZE + SLOT_PAD);
            int y = oy + (i / INV_COLS) * (SLOT_SIZE + SLOT_PAD);
            if (!CheckCollisionPointRec(mp, SlotRect(x, y))) continue;
            placed = true;
            if (inv->bag[i].type == TILE_AIR) {
                inv->bag[i]   = inv->dragItem;
            } else if (inv->bag[i].type == inv->dragItem.type &&
                       inv->bag[i].count + inv->dragItem.count <= STACK_MAX) {
                inv->bag[i].count += inv->dragItem.count;
            } else {
                SwapSlots(&inv->bag[i], &inv->dragItem);
                placed = false;
            }
            if (placed) inv->dragSlot = DRAG_NONE;
        }

        for (int i = 0; i < HOTBAR_SIZE && !placed; i++) {
            int x = ox + i * (SLOT_SIZE + SLOT_PAD);
            if (!CheckCollisionPointRec(mp, SlotRect(x, hoy))) continue;
            placed = true;
            if (inv->hotbar[i].type == TILE_AIR) {
                inv->hotbar[i] = inv->dragItem;
            } else if (inv->hotbar[i].type == inv->dragItem.type &&
                       inv->hotbar[i].count + inv->dragItem.count <= STACK_MAX) {
                inv->hotbar[i].count += inv->dragItem.count;
            } else {
                SwapSlots(&inv->hotbar[i], &inv->dragItem);
                placed = false;
            }
            if (placed) inv->dragSlot = DRAG_NONE;
        }

        if (inv->dragSlot != DRAG_NONE) {
            if (inv->dragFromHotbar)
                inv->hotbar[inv->dragSlot] = inv->dragItem;
            else
                inv->bag[inv->dragSlot] = inv->dragItem;
            inv->dragSlot = DRAG_NONE;
        }
    }
}

void InvDraw(const Inventory *inv) {
    DrawHotbar(inv);
    if (inv->open) DrawBag(inv);
    DrawDraggedItem(inv);

    const char *controls = "WASD - move | Space - jump | E - inventory | LMB - mine | RMB - place | F - consume";

    int cx = 10;
    int cy = 10;
    int cFont = 14;

    DrawText(
        controls,
        cx,
        cy,
        cFont,
        (Color){ 255, 255, 255, 180 }
    );

    const char *tabHint = "[TAB] Quests";

    int tabY = cy + cFont + 6;

    DrawText(
        tabHint,
        cx,
        tabY,
        15,
        (Color){ 255, 220, 60, 255 }
    );
}
