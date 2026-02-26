#include "header.h"
#include "settings.h"
#include "menu.h"

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

static void ResetGame(GameState *gs) {
    CharDef cd = GetCharDef(gs->selectedChar);
    WorldGenerate(&gs->world);
    PlayerInit(&gs->player, &gs->world, gs->selectedChar);
    InputInit(&gs->input);
    InvInit(&gs->inv);
    gs->inv.hotbar[1].type  = TILE_SWORD;
    gs->inv.hotbar[1].count = 1;
    gs->inv.hotbar[0].type  = TILE_TORCH;
    gs->inv.hotbar[0].count = cd.startTorches;
    DayNightInit(&gs->daynight);
    MonstersInit(&gs->monsters);
    MobsInit(&gs->mobs, &gs->world);
    ParticlesInit(&gs->particles);
    QuestInit(&gs->quests);
    gs->camera.offset   = (Vector2){ SCREEN_W * 0.5f, SCREEN_H * 0.5f };
    gs->camera.target   = gs->player.pos;
    gs->camera.rotation = 0.0f;
    gs->camera.zoom     = 2.5f;
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "The Dark");
    InitAudioDevice();
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    MenuSystemState currentState  = STATE_MENU;
    MenuSystemState previousState = STATE_MENU;
    bool isPaused  = false;
    bool needOutro = false;
    GameSettings settings = LoadSettings();

    TexturesLoad();
    MobsLoadTextures();
    Texture2D bgTexture = LoadTexture("resource/Background.png");

    Scoreboard scoreboard;
    ScoreboardLoad(&scoreboard);

    GameState gs = {0};
    gs.selectedChar = CHAR_WARRIOR;
    WorldGenerate(&gs.world);
    PlayerInit(&gs.player, &gs.world, gs.selectedChar);
    InputInit(&gs.input);
    InvInit(&gs.inv);
    DayNightInit(&gs.daynight);
    MonstersInit(&gs.monsters);
    MobsInit(&gs.mobs, &gs.world);
    ParticlesInit(&gs.particles);
    QuestInit(&gs.quests);
    gs.camera.offset   = (Vector2){ SCREEN_W * 0.5f, SCREEN_H * 0.5f };
    gs.camera.target   = gs.player.pos;
    gs.camera.rotation = 0.0f;
    gs.camera.zoom     = 2.5f;

    while (!WindowShouldClose() && currentState != STATE_EXIT) {

        BeginDrawing();
        ClearBackground(BLACK);

        CharDef cd = GetCharDef(gs.selectedChar);

        switch (currentState) {

            case STATE_MENU:
                DrawMainMenu(&currentState);
                previousState = STATE_MENU;
                break;

            case STATE_CHARSELECT:
                DrawCharSelect(&currentState, &gs.selectedChar);
                if (currentState == STATE_GAMEPLAY) {
                    ResetGame(&gs);
                    isPaused = false;
                    IntroRun();
                }
                break;

            case STATE_SETTINGS:
                DrawSettingsScreen(&settings, (int*)&currentState, previousState);
                break;

            case STATE_GAMEPLAY:
                if (IsKeyPressed(KEY_ESCAPE))
                    isPaused = !isPaused;

                if (!isPaused) {
                    float dt    = GetFrameTime();
                    float wheel = GetMouseWheelMove();
                    HungerUpdate(&gs.player, dt);

                    if (wheel != 0.0f && !gs.inv.open) {
                        gs.camera.zoom += wheel * 0.15f;
                        if (gs.camera.zoom < 0.5f) gs.camera.zoom = 0.5f;
                        if (gs.camera.zoom > 5.0f) gs.camera.zoom = 5.0f;
                    }

                    DayNightUpdate(&gs.daynight, dt);
                    MonstersSpawnNight(&gs.monsters, &gs.player, &gs.world, &gs.daynight);
                    MonstersUpdate(&gs.monsters, &gs.player, &gs.world, &gs.particles,
                                   &gs.inv, cd.defenceMult, dt);
                    MobsSpawnDay(&gs.mobs, &gs.world, &gs.daynight, dt);
                    MobsUpdate(&gs.mobs, &gs.player, &gs.world, &gs.particles, &gs.inv, dt);
                    MobsAttack(&gs.mobs, &gs.player, &gs.particles, &gs.inv, cd.damageMult);
                    InputUpdate(&gs.input, &gs.world, &gs.player, &gs.camera, &gs.inv, dt);
                    PlayerUpdate(&gs.player, &gs.world, dt, cd.speedMult);
                    PlayerAttack(&gs.player, &gs.monsters, &gs.particles, &gs.inv,
                                 cd.damageMult, dt);
                    ParticlesUpdate(&gs.particles, dt);
                    InvHandleDrag(&gs.inv);
                    QuestUpdate(&gs.quests, &gs.player, &gs.inv, &gs.world, dt);
                    CameraUpdate(&gs.camera, &gs.player);

                    if (gs.player.hp <= 0)    currentState = STATE_GAMEOVER;
                    if (gs.daynight.finished) { currentState = STATE_MENU; needOutro = true; }
                }

                ClearBackground(DayNightSkyColor(&gs.daynight));
                DrawTexturePro(
                    bgTexture,
                    (Rectangle){ 0, 0, bgTexture.width, bgTexture.height },
                    (Rectangle){ 0, 0, SCREEN_W, SCREEN_H },
                    (Vector2){ 0, 0 }, 0.0f, WHITE
                );
                BeginMode2D(gs.camera);
                    WorldDraw(&gs.world, &gs.camera);
                    InputDrawCursor(&gs.input);
                    ParticlesDraw(&gs.particles);
                    MonstersDraw(&gs.monsters);
                    MobsDraw(&gs.mobs);
                    PlayerDraw(&gs.player, &gs.inv, cd.tint, gs.selectedChar);
                EndMode2D();

                LightingDraw(&gs.world, &gs.camera, &gs.player, &gs.inv, &gs.daynight);
                InvDraw(&gs.inv);
                PlayerDrawHUD(&gs.player, &gs.world, &gs.camera);
                DayNightDrawClock(&gs.daynight);
                QuestDraw(&gs.quests);
                QuestDrawNotif(&gs.quests);

                if (isPaused) {
                    DrawPauseMenu(&isPaused, &currentState);
                    if (currentState == STATE_SETTINGS)
                        previousState = STATE_GAMEPLAY;
                }
                break;

            case STATE_GAMEOVER:
                ClearBackground(DayNightSkyColor(&gs.daynight));
                DrawTexturePro(
                    bgTexture,
                    (Rectangle){ 0, 0, bgTexture.width, bgTexture.height },
                    (Rectangle){ 0, 0, SCREEN_W, SCREEN_H },
                    (Vector2){ 0, 0 }, 0.0f, WHITE
                );
                BeginMode2D(gs.camera);
                    WorldDraw(&gs.world, &gs.camera);
                    ParticlesDraw(&gs.particles);
                    MonstersDraw(&gs.monsters);
                    MobsDraw(&gs.mobs);
                    PlayerDraw(&gs.player, &gs.inv, cd.tint, gs.selectedChar);
                EndMode2D();
                LightingDraw(&gs.world, &gs.camera, &gs.player, &gs.inv, &gs.daynight);
                DrawGameOver(&currentState, &gs);
                break;

            case STATE_SCOREBOARD:
                ScoreboardDraw(&scoreboard, (int*)&currentState, STATE_MENU);
                break;

            default: break;
        }

        EndDrawing();

        if (needOutro) {
            ScoreboardAdd(&scoreboard, gPlayerName, gs.player.kills);
            OutroRun();
            needOutro    = false;
            currentState = STATE_SCOREBOARD;
        }
    }

    UnloadTexture(bgTexture);
    MobsUnloadTextures();
    TexturesUnload();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
