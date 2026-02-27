#include "header.h"

void RainInit(void) {
    for (int i = 0; i < RAIN_COUNT; i++) {
        gRain[i].x     = (float)(rand() % SCREEN_W);
        gRain[i].y     = (float)(rand() % SCREEN_H);
        gRain[i].speed = RAIN_SPEED_MIN +
                         (rand() % (int)(RAIN_SPEED_MAX - RAIN_SPEED_MIN));
    }
}

void RainUpdate(float dt) {
    for (int i = 0; i < RAIN_COUNT; i++) {
        gRain[i].x += RAIN_WIND  * dt;
        gRain[i].y += gRain[i].speed * dt;
        if (gRain[i].y > SCREEN_H || gRain[i].x > SCREEN_W) {
            gRain[i].x = (float)(rand() % SCREEN_W) - 100;
            gRain[i].y = (float)(rand() % -50);
            gRain[i].speed = RAIN_SPEED_MIN +
                             (rand() % (int)(RAIN_SPEED_MAX - RAIN_SPEED_MIN));
        }
    }
}

void RainDraw(void) {
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 30, 50, 80, 40 });

    for (int i = 0; i < RAIN_COUNT; i++) {
        float x0 = gRain[i].x;
        float y0 = gRain[i].y;
        float x1 = x0 + RAIN_WIND  * 0.03f;
        float y1 = y0 + RAIN_LEN;

        unsigned char alpha = 120 + (unsigned char)(gRain[i].speed / RAIN_SPEED_MAX * 100);
        Color col = { 160, 200, 255, alpha };
        DrawLineEx((Vector2){ x0, y0 }, (Vector2){ x1, y1 }, 1.2f, col);
    }
}
