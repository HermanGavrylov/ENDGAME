#include "header.h"

void QuestInit(QuestLog *ql) {
    memset(ql, 0, sizeof(QuestLog));
    ql->show      = false;
    ql->fadeAlpha = 0.0f;

    ql->quests[0] = (Quest){ .title = "Mine Coal",      .desc = "Gather 1 coal for torches",    .done = false };
    ql->quests[1] = (Quest){ .title = "Craft a Sword",  .desc = "Collect 2 iron ore",            .done = false };
    ql->quests[2] = (Quest){ .title = "Light the Night",.desc = "Place at least 1 torch",        .done = false };
    ql->quests[3] = (Quest){ .title = "Go Underground", .desc = "Dig down 10 tiles from surface",.done = false };
    ql->quests[4] = (Quest){ .title = "Hunt for Food", .desc = "Collect 3 pieces of meat", .done = false };
    ql->count = 5;

    memset(ql->wasDone,   0, sizeof(ql->wasDone));
    ql->notifText[0] = '\0';
    ql->notifTimer   = 0.0f;
}

void QuestUpdate(QuestLog *ql, const Player *p, const Inventory *inv,
                 const World *w, float dt) {
    if (IsKeyPressed(KEY_TAB))
        ql->show = !ql->show;

    float target = ql->show ? 1.0f : 0.0f;
    ql->fadeAlpha += (target - ql->fadeAlpha) * dt * 10.0f;
    if (ql->fadeAlpha < 0.005f) ql->fadeAlpha = 0.0f;
    if (ql->fadeAlpha > 0.995f) ql->fadeAlpha = 1.0f;

    int coal = 0;
    int iron = 0;

    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (inv->hotbar[i].type == TILE_COAL) coal += inv->hotbar[i].count;
        if (inv->hotbar[i].type == TILE_IRON) iron += inv->hotbar[i].count;
    }
    for (int i = 0; i < INV_SIZE; i++) {
        if (inv->bag[i].type == TILE_COAL) coal += inv->bag[i].count;
        if (inv->bag[i].type == TILE_IRON) iron += inv->bag[i].count;
    }

    int meat = 0;
    for (int i = 0; i < HOTBAR_SIZE; i++)
    if (inv->hotbar[i].type == TILE_MEAT) meat += inv->hotbar[i].count;
    for (int i = 0; i < INV_SIZE; i++)
    if (inv->bag[i].type == TILE_MEAT) meat += inv->bag[i].count;

    ql->quests[4].done    = (meat >= 3);
    ql->progress[4]       = (float)meat / 3.0f;
    if (ql->progress[4] > 1.0f) ql->progress[4] = 1.0f; 

    int torchesPlaced = 0;
    int surfaceTY     = SURFACE_LEVEL;
    int playerTY      = (int)(p->pos.y / TILE_SIZE);

    for (int ty = 0; ty < WORLD_H; ty++)
        for (int tx = 0; tx < WORLD_W; tx++)
            if (w->tiles[ty][tx].type == TILE_TORCH) torchesPlaced++;

    ql->quests[0].done = (coal          >= 1);
    ql->quests[1].done = (iron          >= 2);
    ql->quests[2].done = (torchesPlaced >= 1);
    ql->quests[3].done = (playerTY       > surfaceTY + 10);

    ql->progress[0] = (float)coal          / 1.0f;
    ql->progress[1] = (float)iron          / 2.0f;
    ql->progress[2] = (float)torchesPlaced / 1.0f;
    ql->progress[3] = (float)(playerTY - surfaceTY) / 10.0f;

    for (int i = 0; i < ql->count; i++) {
        if (ql->progress[i] > 1.0f) ql->progress[i] = 1.0f;
        if (ql->progress[i] < 0.0f) ql->progress[i] = 0.0f;
    }

    for (int i = 0; i < ql->count; i++) {
        if (ql->quests[i].done && !ql->wasDone[i]) {
            snprintf(ql->notifText, sizeof(ql->notifText),
                     "Task complete: %s", ql->quests[i].title);
            ql->notifTimer = 3.0f;
        }
        ql->wasDone[i] = ql->quests[i].done;
    }

    if (ql->notifTimer > 0.0f)
        ql->notifTimer -= dt;
}

void QuestDraw(const QuestLog *ql) {
    if (ql->fadeAlpha <= 0.0f) return;

    unsigned char a = (unsigned char)(ql->fadeAlpha * 255);

    int panelW = 300;
    int panelH = 30 + ql->count * 68;
    int panelX = SCREEN_W / 2 - panelW / 2;
    int panelY = SCREEN_H / 2 - panelH / 2;

    DrawRectangle(panelX, panelY, panelW, panelH,
                  (Color){ 10, 10, 20, (unsigned char)(ql->fadeAlpha * 200) });
    DrawRectangleLines(panelX, panelY, panelW, panelH,
                       (Color){ 100, 90, 70, a });

    const char *title = "Survival Tasks";
    int tw = MeasureText(title, 16);
    DrawText(title, panelX + panelW / 2 - tw / 2, panelY + 8, 16,
             (Color){ 255, 220, 120, a });

    int doneCount = 0;
    for (int i = 0; i < ql->count; i++)
        if (ql->quests[i].done) doneCount++;

    char prog[32];
    snprintf(prog, sizeof(prog), "%d / %d", doneCount, ql->count);
    int pw = MeasureText(prog, 12);
    DrawText(prog, panelX + panelW - pw - 8, panelY + 10, 12,
             (Color){ 180, 180, 180, a });

    for (int i = 0; i < ql->count; i++) {
        const Quest *q = &ql->quests[i];
        int iy = panelY + 32 + i * 68;

        Color rowBg = q->done
            ? (Color){ 20, 60, 20, (unsigned char)(ql->fadeAlpha * 160) }
            : (Color){ 30, 30, 40, (unsigned char)(ql->fadeAlpha * 140) };
        DrawRectangle(panelX + 6, iy, panelW - 12, 62, rowBg);
        DrawRectangleLines(panelX + 6, iy, panelW - 12, 62,
                           (Color){ 70, 65, 55, a });

        Color checkColor = q->done
            ? (Color){ 80, 220, 80, a }
            : (Color){ 100, 100, 100, a };
        DrawText(q->done ? "[X]" : "[ ]", panelX + 12, iy + 6, 14, checkColor);

        Color titleColor = q->done
            ? (Color){ 120, 220, 120, a }
            : (Color){ 230, 220, 200, a };
        DrawText(q->title, panelX + 44, iy + 6, 14, titleColor);

        DrawText(q->desc, panelX + 12, iy + 26, 11,
                 (Color){ 160, 155, 140, a });

        int barX = panelX + 12;
        int barY = iy + 46;
        int barW = panelW - 24;
        DrawRectangle(barX, barY, barW, 7, (Color){ 30, 30, 30, a });
        Color fillColor = q->done
            ? (Color){ 60, 200, 60, a }
            : (Color){ 200, 160, 40, a };
        DrawRectangle(barX, barY, (int)(barW * ql->progress[i]), 7, fillColor);
        DrawRectangleLines(barX, barY, barW, 7, (Color){ 80, 75, 60, a });
    }

    const char *hint = "TAB - close";
    int hw = MeasureText(hint, 11);
    DrawText(hint, panelX + panelW / 2 - hw / 2, panelY + panelH - 18, 11,
             (Color){ 120, 120, 120, a });
}

void QuestDrawNotif(const QuestLog *ql) {
    if (ql->notifTimer <= 0.0f) return;

    float t  = ql->notifTimer < 1.0f ? ql->notifTimer : 1.0f;
    unsigned char na = (unsigned char)(t * 255);

    int fontSize = 16;
    int textW    = MeasureText(ql->notifText, fontSize);
    int padH = 10, padV = 8;
    int bw = textW + padH * 2;
    int bh = fontSize + padV * 2;
    int bx = SCREEN_W / 2 - bw / 2;
    int by = SCREEN_H - 90;

    DrawRectangle(bx + 2, by + 2, bw, bh, (Color){ 0, 0, 0, (unsigned char)(na * 0.5f) });
    DrawRectangle(bx, by, bw, bh, (Color){ 15, 50, 15, na });
    DrawRectangleLines(bx, by, bw, bh, (Color){ 70, 210, 70, na });
    DrawText(ql->notifText, bx + padH, by + padV, fontSize, (Color){ 130, 255, 130, na });
}
