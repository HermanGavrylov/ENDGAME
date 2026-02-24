#include "header.h"

static float DarknessAlpha(const DayNight *dn) {
    float t  = dn->elapsed;
    float hd = DAY_DURATION;
    float hn = NIGHT_DURATION;
    if (t <= hd) return 0.88f * (t / hd);
    float f = (t - hd) / hn;
    return (f <= 0.5f) ? 0.88f : 0.88f * (1.0f - (f - 0.5f) / 0.5f);
}

static void DrawLight(Vector2 worldPos, float radius, Color inner, const Camera2D *cam) {
    Vector2 screen = GetWorldToScreen2D(worldPos, *cam);
    DrawCircleGradient((int)screen.x, (int)screen.y,
                       radius * cam->zoom, inner, (Color){ 0, 0, 0, 0 });
}

void LightingDraw(const World *w, const Camera2D *cam,
                  const Player *p, const Inventory *inv, const DayNight *dn) {

    float alpha = DarknessAlpha(dn);
    if (alpha <= 0.01f) return;

    unsigned char darkA = (unsigned char)(alpha * 255.0f);

    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 20, darkA });

    BeginBlendMode(BLEND_ADDITIVE);

    bool holdingTorch = (inv->hotbar[inv->activeSlot].type == TILE_TORCH);

    if (holdingTorch) {
        Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };
        Color torchHand = { 255, 200, 80, (unsigned char)(alpha * 230.0f) };
        DrawLight(pCenter, TORCH_LIGHT_R, torchHand, cam);
    } else {
        Vector2 pCenter = { p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };
        Color ambient   = { 180, 140, 80, (unsigned char)(alpha * 90.0f) };
        DrawLight(pCenter, PLAYER_LIGHT_R, ambient, cam);
    }

    float visHalfW = (SCREEN_W / cam->zoom) * 0.5f;
    float visHalfH = (SCREEN_H / cam->zoom) * 0.5f;
    int startX = (int)floorf((cam->target.x - visHalfW) / TILE_SIZE) - 1;
    int startY = (int)floorf((cam->target.y - visHalfH) / TILE_SIZE) - 1;
    int endX   = (int)floorf((cam->target.x + visHalfW) / TILE_SIZE) + 2;
    int endY   = (int)floorf((cam->target.y + visHalfH) / TILE_SIZE) + 2;
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > WORLD_W) endX = WORLD_W;
    if (endY > WORLD_H) endY = WORLD_H;

    Color torchCol = { 255, 200, 80, (unsigned char)(alpha * 220.0f) };

    for (int ty = startY; ty < endY; ty++) {
        for (int tx = startX; tx < endX; tx++) {
            if (!TileEmitsLight(w->tiles[ty][tx].type)) continue;
            Vector2 tileCenter = {
                tx * TILE_SIZE + TILE_SIZE * 0.5f,
                ty * TILE_SIZE + TILE_SIZE * 0.5f
            };
            DrawLight(tileCenter, TORCH_RADIUS, torchCol, cam);
        }
    }

    EndBlendMode();
}
