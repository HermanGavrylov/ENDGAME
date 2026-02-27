#include "header.h"

static RainDrop gRain[RAIN_COUNT];
static float    gRainTimer  = 0.0f;
static float    gRainAlpha  = 0.0f;
static bool     gRainDone   = false;

void RainInit(void) {
    for (int i = 0; i < RAIN_COUNT; i++) {
        gRain[i].x     = (float)(rand() % SCREEN_W);
        gRain[i].y     = (float)(rand() % SCREEN_H);
        gRain[i].speed = RAIN_SPEED_MIN +
                         (rand() % (int)(RAIN_SPEED_MAX - RAIN_SPEED_MIN));
    }
    gRainTimer = 0.0f;
    gRainAlpha = 0.0f;
    gRainDone  = false;
}

void RainUpdate(float dt) {
    if (gRainDone) return;

    gRainTimer += dt;

    if (gRainTimer < RAIN_FADE_TIME)
        gRainAlpha = gRainTimer / RAIN_FADE_TIME;
    else if (gRainTimer < RAIN_DURATION - RAIN_FADE_TIME)
        gRainAlpha = 1.0f;
    else if (gRainTimer < RAIN_DURATION)
        gRainAlpha = (RAIN_DURATION - gRainTimer) / RAIN_FADE_TIME;
    else {
        gRainAlpha = 0.0f;
        gRainDone  = true;
        return;
    }

    for (int i = 0; i < RAIN_COUNT; i++) {
        gRain[i].x += RAIN_WIND * dt;
        gRain[i].y += gRain[i].speed * dt;
        if (gRain[i].y > SCREEN_H || gRain[i].x > SCREEN_W) {
            gRain[i].x     = (float)(rand() % SCREEN_W) - 100;
            gRain[i].y     = (float)(rand() % -50);
            gRain[i].speed = RAIN_SPEED_MIN +
                             (rand() % (int)(RAIN_SPEED_MAX - RAIN_SPEED_MIN));
        }
    }
}

void RainDraw(void) {
    if (gRainAlpha <= 0.01f) return;

    unsigned char overlayA = (unsigned char)(gRainAlpha * 40.0f);
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 30, 50, 80, overlayA });

    for (int i = 0; i < RAIN_COUNT; i++) {
        float x0 = gRain[i].x;
        float y0 = gRain[i].y;
        float x1 = x0 + RAIN_WIND * 0.03f;
        float y1 = y0 + RAIN_LEN;
        unsigned char alpha = (unsigned char)(
            gRainAlpha * (120 + gRain[i].speed / RAIN_SPEED_MAX * 100));
        DrawLineEx((Vector2){ x0, y0 }, (Vector2){ x1, y1 }, 1.2f,
                   (Color){ 160, 200, 255, alpha });
    }
}
