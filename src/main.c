#include "header.h"

void CameraUpdate(Camera2D *cam, const Player *p) {
    cam->target = (Vector2){ p->pos.x + PLAYER_W * 0.5f, p->pos.y + PLAYER_H * 0.5f };
    float halfW    = (SCREEN_W * 0.5f) / cam->zoom;
    float halfH    = (SCREEN_H * 0.5f) / cam->zoom;
    float worldPxW = WORLD_W * TILE_SIZE;
    float worldPxH = WORLD_H * TILE_SIZE;
    if (cam->target.x < halfW)            cam->target.x = halfW;
    if (cam->target.y < halfH)            cam->target.y = halfH;
    if (cam->target.x > worldPxW - halfW) cam->target.x = worldPxW - halfW;
    if (cam->target.y > worldPxH - halfH) cam->target.y = worldPxH - halfH;
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "The Dark");
    SetTargetFPS(TARGET_FPS);
    IntroRun();
    TexturesLoad();

    GameState gs = {0};
    WorldGenerate(&gs.world);
    PlayerInit(&gs.player, &gs.world);
    InputInit(&gs.input);
    InvInit(&gs.inv);
    gs.inv.hotbar[1].type  = TILE_SWORD;
    gs.inv.hotbar[1].count = 1;
    DayNightInit(&gs.daynight);
    MonstersInit(&gs.monsters);
    ParticlesInit(&gs.particles);
    QuestInit(&gs.quests);

    gs.camera.offset   = (Vector2){ SCREEN_W * 0.5f, SCREEN_H * 0.5f };
    gs.camera.target   = gs.player.pos;
    gs.camera.rotation = 0.0f;
    gs.camera.zoom     = 2.5f;

    while (!WindowShouldClose() && !gs.daynight.finished && gs.player.hp > 0) {
        float dt    = GetFrameTime();
        float wheel = GetMouseWheelMove();

        if (wheel != 0.0f && !gs.inv.open) {
            gs.camera.zoom += wheel * 0.15f;
            if (gs.camera.zoom < 0.5f) gs.camera.zoom = 0.5f;
            if (gs.camera.zoom > 5.0f) gs.camera.zoom = 5.0f;
        }

        DayNightUpdate(&gs.daynight, dt);
        MonstersSpawnNight(&gs.monsters, &gs.player, &gs.world, &gs.daynight);
        MonstersUpdate(&gs.monsters, &gs.player, &gs.world, &gs.particles, dt);
        InputUpdate(&gs.input, &gs.world, &gs.player, &gs.camera, &gs.inv, dt);
        PlayerUpdate(&gs.player, &gs.world, dt);
        PlayerAttack(&gs.player, &gs.monsters, &gs.particles, &gs.inv, dt);
        ParticlesUpdate(&gs.particles, dt);
        InvHandleDrag(&gs.inv);
        QuestUpdate(&gs.quests, &gs.player, &gs.inv, &gs.world, dt);
        CameraUpdate(&gs.camera, &gs.player);

        BeginDrawing();
        ClearBackground(DayNightSkyColor(&gs.daynight));

        BeginMode2D(gs.camera);
        WorldDraw(&gs.world, &gs.camera);
        InputDrawCursor(&gs.input);
        ParticlesDraw(&gs.particles);
        MonstersDraw(&gs.monsters);
        PlayerDraw(&gs.player);
        EndMode2D();

        LightingDraw(&gs.world, &gs.camera, &gs.player, &gs.inv, &gs.daynight);
        InvDraw(&gs.inv);
        PlayerDrawHUD(&gs.player, &gs.world, &gs.camera);
        DayNightDrawClock(&gs.daynight);
        QuestDraw(&gs.quests);
        EndDrawing();
    }

    TexturesUnload();
    if (gs.daynight.finished && gs.player.hp > 0) OutroRun();
    CloseWindow();
    return 0;
}
