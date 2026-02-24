#ifndef HEADER_H
#define HEADER_H

#include "header_structs.h"

void  TexturesLoad(void);
void  TexturesUnload(void);
void  TileDraw(TileType type, int px, int py);
void  TileDrawScaled(TileType type, int px, int py, int size);
void  PlayerSpriteDraw(float px, float py, bool facingLeft);

void  WorldGenerate(World *w);
bool  WorldIsSolid(const World *w, int tx, int ty);
bool  WorldInBounds(int tx, int ty);
void  WorldDraw(const World *w, const Camera2D *cam);

void  PlayerInit(Player *p, const World *w);
void  PlayerUpdate(Player *p, const World *w, float dt);
void  PlayerDraw(const Player *p);
void  PlayerDrawHUD(const Player *p);

void  MonstersInit(Monsters *ms);
void  MonstersUpdate(Monsters *ms, Player *p, const World *w, float dt);
void  MonstersDraw(const Monsters *ms);
void  MonstersSpawnNight(Monsters *ms, const Player *p, const World *w, const DayNight *dn);

void  InvInit(Inventory *inv);
bool  InvAddItem(Inventory *inv, TileType type);
bool  InvConsumeActive(Inventory *inv);
void  InvDraw(const Inventory *inv);

void  InputInit(InputState *inp);
void  InputUpdate(InputState *inp, World *w, const Player *p,
                  const Camera2D *cam, Inventory *inv, float dt);
void  InputDrawCursor(const InputState *inp);

void  DayNightInit(DayNight *dn);
void  DayNightUpdate(DayNight *dn, float dt);
Color DayNightSkyColor(const DayNight *dn);
void  DayNightDrawClock(const DayNight *dn);

void  LightingDraw(const World *w, const Camera2D *cam,
                   const Player *p, const Inventory *inv, const DayNight *dn);

void  CameraUpdate(Camera2D *cam, const Player *p);

#endif
