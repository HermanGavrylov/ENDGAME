#ifndef HEADER_H
#define HEADER_H
#include "header_structs.h"
void  TexturesLoad(void);
void  TexturesUnload(void);
void  TileDraw(TileType type, int px, int py);
void  TileDrawScaled(TileType type, int px, int py, int size);
void  PlayerSpriteDraw(float px, float py, bool facingLeft, Color tint);
void  PlayerSpriteDrawClass(float px, float py, bool facingLeft, Color tint, CharClass cls);
Texture2D PlayerTexGet(CharClass cls);
bool      PlayerTexIsLoaded(CharClass cls);
void  SwordDrawInHand(float cx, float cy, float angle, float reach, bool facingLeft);
void  TorchDrawInHand(float cx, float cy, bool facingLeft);
void  WorldGenerate(World *w);
bool  WorldIsSolid(const World *w, int tx, int ty);
bool  WorldInBounds(int tx, int ty);
void  WorldDraw(const World *w, const Camera2D *cam);
void  PlayerInit(Player *p, const World *w, CharClass cls);
void  PlayerUpdate(Player *p, const World *w, float dt, float speedMult);
void  PlayerDraw(const Player *p, const Inventory *inv, Color tint, CharClass cls);
void  PlayerDrawHUD(const Player *p, const World *w, const Camera2D *cam);
bool  PlayerHasSwordActive(const Inventory *inv);
void  PlayerAttack(Player *p, Monsters *ms, Particles *ps,
                   const Inventory *inv, float damageMult, float dt);
void  MonstersInit(Monsters *ms);
void  MonstersUpdate(Monsters *ms, Player *p, World *w,
                     Particles *ps, Inventory *inv, float defenceMult, float dt);
void  MonstersDraw(const Monsters *ms);
void  MonstersSpawnNight(Monsters *ms, const Player *p,
                         const World *w, const DayNight *dn);
void  MobsLoadTextures(void);
void  MobsUnloadTextures(void);
void  MobsInit(Mobs *mobs, const World *w);
void  MobsSpawnDay(Mobs *mobs, const World *w, const DayNight *dn, float dt);
void  MobsUpdate(Mobs *mobs, Player *p, const World *w,
                 Particles *ps, Inventory *inv, float dt);
void  MobsAttack(Mobs *mobs, Player *p, Particles *ps,
                 const Inventory *inv, float damageMult);
void  MobsDraw(const Mobs *mobs);
void  ParticlesInit(Particles *ps);
void  ParticlesSpawnBlood(Particles *ps, Vector2 pos, int count);
void  ParticlesSpawnDust(Particles *ps, Vector2 pos, int count);
void  ParticlesSpawnSpark(Particles *ps, Vector2 pos, int count);
void  ParticlesUpdate(Particles *ps, float dt);
void  ParticlesDraw(const Particles *ps);
void  InvInit(Inventory *inv);
bool  InvAddItem(Inventory *inv, TileType type);
bool  InvConsumeActive(Inventory *inv);
void  InvHandleDrag(Inventory *inv);
void  InvDraw(const Inventory *inv);
void  InputInit(InputState *inp);
void  InputUpdate(InputState *inp, World *w, Player *p,
                  const Camera2D *cam, Inventory *inv, float dt);
void  InputDrawCursor(const InputState *inp);
void  DayNightInit(DayNight *dn);
void  DayNightUpdate(DayNight *dn, float dt);
Color DayNightSkyColor(const DayNight *dn);
void  DayNightDrawClock(const DayNight *dn);
void  LightingDraw(const World *w, const Camera2D *cam,
                   const Player *p, const Inventory *inv,
                   const DayNight *dn);
void  QuestInit(QuestLog *ql);
void  QuestUpdate(QuestLog *ql, const Player *p, const Inventory *inv,
                  const World *w, float dt);
void  QuestDraw(const QuestLog *ql);
void  QuestDrawNotif(const QuestLog *ql);
void  HungerUpdate(Player *p, float dt);
void  HungerDrawHUD(const Player *p);
void  IntroRun(void);
void  OutroRun(void);
void  CameraUpdate(Camera2D *cam, const Player *p);
#endif
