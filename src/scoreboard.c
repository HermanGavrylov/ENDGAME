#include "header.h"

void ScoreboardLoad(Scoreboard *sb) {
    sb->count = 0;
    FILE *f = fopen(SCORE_FILE, "rb");
    if (!f) return;
    fread(&sb->count, sizeof(int), 1, f);
    if (sb->count > SCORE_MAX_ENTRIES) sb->count = SCORE_MAX_ENTRIES;
    fread(sb->entries, sizeof(ScoreEntry), sb->count, f);
    fclose(f);
}

void ScoreboardSave(const Scoreboard *sb) {
    FILE *f = fopen(SCORE_FILE, "wb");
    if (!f) return;
    fwrite(&sb->count, sizeof(int), 1, f);
    fwrite(sb->entries, sizeof(ScoreEntry), sb->count, f);
    fclose(f);
}

void ScoreboardAdd(Scoreboard *sb, const char *name, int kills) {
    ScoreEntry entry;
    strncpy(entry.name, name, SCORE_NAME_LEN - 1);
    entry.name[SCORE_NAME_LEN - 1] = '\0';
    entry.kills = kills;
    entry.score = kills * SCORE_MULTIPLIER;

    if (sb->count < SCORE_MAX_ENTRIES) {
        sb->entries[sb->count++] = entry;
    } else {
        int minIdx = 0;
        for (int i = 1; i < sb->count; i++)
            if (sb->entries[i].score < sb->entries[minIdx].score)
                minIdx = i;
        if (entry.score > sb->entries[minIdx].score)
            sb->entries[minIdx] = entry;
    }

    for (int i = 0; i < sb->count - 1; i++) {
        for (int j = i + 1; j < sb->count; j++) {
            if (sb->entries[j].score > sb->entries[i].score) {
                ScoreEntry tmp   = sb->entries[i];
                sb->entries[i]   = sb->entries[j];
                sb->entries[j]   = tmp;
            }
        }
    }

    ScoreboardSave(sb);
}

void ScoreboardDraw(Scoreboard *sb, int *currentState, int prevState) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    DrawRectangle(0, 0, (int)sw, (int)sh, (Color){ 0, 0, 0, 210 });

    int panelW = 420;
    int rowH   = 44;
    int panelH = 80 + SCORE_MAX_ENTRIES * rowH + 70;
    int px = (int)(sw / 2 - panelW / 2);
    int py = (int)(sh / 2 - panelH / 2);

    DrawRectangle(px, py, panelW, panelH, (Color){ 10, 10, 20, 230 });
    DrawRectangleLines(px, py, panelW, panelH, (Color){ 255, 220, 120, 255 });

    const char *title = "SCOREBOARD";
    int tf = 28;
    int tw = MeasureText(title, tf);
    DrawText(title, (int)(sw / 2 - tw / 2), py + 16, tf, (Color){ 255, 220, 120, 255 });

    int headerY = py + 56;
    DrawText("#",      px + 20,          headerY, 13, (Color){ 150, 150, 150, 255 });
    DrawText("NAME",   px + 55,          headerY, 13, (Color){ 150, 150, 150, 255 });
    DrawText("KILLS",  px + panelW - 160, headerY, 13, (Color){ 150, 150, 150, 255 });
    DrawText("SCORE",  px + panelW - 80,  headerY, 13, (Color){ 150, 150, 150, 255 });
    DrawLine(px + 10, headerY + 18, px + panelW - 10, headerY + 18,
             (Color){ 80, 75, 55, 255 });

    Color rowColors[] = {
        (Color){ 255, 215,   0, 255 },
        (Color){ 192, 192, 192, 255 },
        (Color){ 205, 127,  50, 255 },
        (Color){ 200, 200, 200, 200 },
        (Color){ 200, 200, 200, 200 },
    };

    for (int i = 0; i < SCORE_MAX_ENTRIES; i++) {
        int ry = headerY + 24 + i * rowH;
        Color c = rowColors[i];

        if (i % 2 == 0)
            DrawRectangle(px + 10, ry, panelW - 20, rowH - 4,
                          (Color){ 255, 255, 255, 8 });

        char rank[4];
        snprintf(rank, sizeof(rank), "%d", i + 1);
        DrawText(rank, px + 20, ry + 12, 14, c);

        if (i < sb->count) {
            DrawText(sb->entries[i].name,  px + 55,           ry + 12, 14, c);

            char kills[16], score[16];
            snprintf(kills, sizeof(kills), "%d",  sb->entries[i].kills);
            snprintf(score, sizeof(score), "%d",  sb->entries[i].score);

            int kw = MeasureText(kills, 14);
            int scw = MeasureText(score, 14);
            DrawText(kills, px + panelW - 160 + 20 - kw / 2, ry + 12, 14, c);
            DrawText(score, px + panelW - 80  + 20 - scw / 2, ry + 12, 14,
                     (Color){ 120, 230, 120, 255 });
        } else {
            DrawText("---", px + 55, ry + 12, 14, (Color){ 60, 60, 60, 255 });
            DrawText("-",   px + panelW - 150, ry + 12, 14, (Color){ 60, 60, 60, 255 });
            DrawText("-",   px + panelW - 70,  ry + 12, 14, (Color){ 60, 60, 60, 255 });
        }

        DrawLine(px + 10, ry + rowH - 4, px + panelW - 10, ry + rowH - 4,
                 (Color){ 50, 50, 50, 150 });
    }

    int btnW = 200, btnH = 40;
    int btnX = (int)(sw / 2 - btnW / 2);
    int btnY = py + panelH - 56;

    Vector2 mouse = GetMousePosition();
    Rectangle btnR = { (float)btnX, (float)btnY, (float)btnW, (float)btnH };
    bool hov = CheckCollisionPointRec(mouse, btnR);
    Color bc = hov ? (Color){ 255, 220, 120, 255 } : (Color){ 180, 160, 80, 255 };
    DrawRectangleRec(btnR, (Color){ 20, 20, 10, 200 });
    DrawRectangleLinesEx(btnR, 1, bc);
    const char *bl = "MAIN MENU";
    int blw = MeasureText(bl, 14);
    DrawText(bl, btnX + btnW / 2 - blw / 2, btnY + btnH / 2 - 7, 14, bc);
    if (hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *currentState = prevState;
}
