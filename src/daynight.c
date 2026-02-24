#include "header.h"

void DayNightInit(DayNight *dn) {
    dn->elapsed  = 0.0f;
    dn->hour     = 18.0f;
    dn->isDay    = true;
    dn->finished = false;
}

void DayNightUpdate(DayNight *dn, float dt) {
    if (dn->finished) return;

    dn->elapsed += dt;

    if (dn->elapsed >= CYCLE_DURATION) {
        dn->elapsed  = CYCLE_DURATION;
        dn->hour     = 6.0f;
        dn->isDay    = true;
        dn->finished = true;
        return;
    }

    float t = dn->elapsed;

    if (t < DAY_DURATION) {
        dn->isDay = false;
        dn->hour  = 18.0f + (t / DAY_DURATION) * 6.0f;
        if (dn->hour >= 24.0f) dn->hour -= 24.0f;
    } else {
        dn->isDay = false;
        float nightT = t - DAY_DURATION;
        dn->hour = 0.0f + (nightT / NIGHT_DURATION) * 6.0f;
        if (nightT / NIGHT_DURATION >= 0.5f)
            dn->isDay = true;
    }
}

Color DayNightSkyColor(const DayNight *dn) {
    Color day   = {100, 180, 255, 255};
    Color dusk  = {255, 100,  30, 255};
    Color night = { 10,  10,  35, 255};
    Color dawn  = {255, 140,  60, 255};

    float t  = dn->elapsed;
    float hd = DAY_DURATION;
    float hn = NIGHT_DURATION;

    if (t <= hd) {
        float f = t / hd;
        if (f < 0.25f) {
            float s = f / 0.25f;
            return (Color){
                (unsigned char)(day.r + (dusk.r - day.r) * s),
                (unsigned char)(day.g + (dusk.g - day.g) * s),
                (unsigned char)(day.b + (dusk.b - day.b) * s),
                255
            };
        }
        float s = (f - 0.25f) / 0.75f;
        return (Color){
            (unsigned char)(dusk.r + (night.r - dusk.r) * s),
            (unsigned char)(dusk.g + (night.g - dusk.g) * s),
            (unsigned char)(dusk.b + (night.b - dusk.b) * s),
            255
        };
    } else {
        float nightT = t - hd;
        float f      = nightT / hn;
        if (f < 0.5f)
            return night;
        if (f < 0.75f) {
            float s = (f - 0.5f) / 0.25f;
            return (Color){
                (unsigned char)(night.r + (dawn.r - night.r) * s),
                (unsigned char)(night.g + (dawn.g - night.g) * s),
                (unsigned char)(night.b + (dawn.b - night.b) * s),
                255
            };
        }
        float s = (f - 0.75f) / 0.25f;
        return (Color){
            (unsigned char)(dawn.r + (day.r - dawn.r) * s),
            (unsigned char)(dawn.g + (day.g - dawn.g) * s),
            (unsigned char)(dawn.b + (day.b - dawn.b) * s),
            255
        };
    }
}

void DayNightDrawOverlay(const DayNight *dn) {
    if (dn->finished) return;

    float t  = dn->elapsed;
    float hd = DAY_DURATION;
    float hn = NIGHT_DURATION;
    float alpha = 0.0f;

    if (t <= hd) {
        alpha = 0.88f * (t / hd);
    } else {
        float f = (t - hd) / hn;
        if (f <= 0.5f)
            alpha = 0.88f;
        else
            alpha = 0.88f * (1.0f - (f - 0.5f) / 0.5f);
    }

    if (alpha <= 0.0f) return;
    unsigned char a = (unsigned char)(alpha * 255.0f);
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 20, a });
}

void DayNightDrawClock(const DayNight *dn) {
    int h    = (int)dn->hour % 24;
    int mRaw = (int)((dn->hour - (int)dn->hour) * 60.0f);
    int m    = (mRaw / 10) * 10;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", h, m);

    const char *label = dn->isDay ? "Day" : "Night";
    Color labelCol    = dn->isDay
                        ? (Color){ 255, 240, 100, 220 }
                        : (Color){ 160, 180, 255, 220 };

    int tw = MeasureText(buf, 18);
    int lw = MeasureText(label, 14);
    int bw = (tw > lw ? tw : lw) + 16;
    int bx = SCREEN_W - bw - 8;

    DrawRectangle(bx, 6, bw, 50, (Color){ 0, 0, 0, 120 });
    DrawText(buf,   SCREEN_W - tw - 16, 10, 18, WHITE);
    DrawText(label, SCREEN_W - lw - 16, 34, 14, labelCol);
}
