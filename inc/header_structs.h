#ifndef HEADER_STRUCTS_H
#define HEADER_STRUCTS_H

#include "../resource/raylib/include/raylib.h"
#include "../resource/raylib/include/raymath.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_W         1280
#define SCREEN_H         720
#define TARGET_FPS       60

#define TILE_SIZE        16
#define WORLD_W          500
#define WORLD_H          250

#define SURFACE_LEVEL    60
#define SURFACE_RANGE    18
#define DIRT_DEPTH       20

#define CAVE_THRESHOLD   0.54f
#define CAVE_OCTAVES     4

#define ORE_COAL_THR     0.62f
#define ORE_IRON_THR     0.65f
#define ORE_GOLD_THR     0.68f
#define ORE_MIN_DEPTH    10

#define TREE_MIN_HEIGHT  4
#define TREE_MAX_HEIGHT  7
#define TREE_MIN_GAP     4
#define LAKE_MIN_WIDTH   6
#define LAKE_MAX_WIDTH   14
#define LAKE_DEPTH       3

#define PLAYER_W         12
#define PLAYER_H         24
#define PLAYER_SPEED     120.0f
#define JUMP_FORCE       -280.0f
#define GRAVITY          500.0f
#define MAX_FALL_SPEED   600.0f
#define PLAYER_SPRITE_W  16
#define PLAYER_SPRITE_H  16
#define PLAYER_MAX_HP    100
#define PLAYER_IFRAMES   1.2f

#define SWORD_DAMAGE     20
#define SWORD_REACH      28.0f
#define SWORD_ARC        0.25f
#define SWORD_COOLDOWN   0.45f

#define MINE_REACH       5
#define MINE_COOLDOWN    0.25f

#define TORCH_RADIUS     120.0f
#define TORCH_LIGHT_R    140.0f
#define PLAYER_LIGHT_R   55.0f

#define HOTBAR_SIZE      9
#define INV_ROWS         3
#define INV_COLS         9
#define INV_SIZE         (INV_ROWS * INV_COLS)
#define STACK_MAX        999
#define DRAG_NONE        -1

#define DAY_DURATION     1.0f
#define NIGHT_DURATION   150.0f
#define CYCLE_DURATION   (DAY_DURATION + NIGHT_DURATION)
#define HOURS_IN_DAY     24
#define TRANSITION_HOURS 1.5f

#define MAX_MONSTERS           64
#define MONSTER_IFRAMES        0.5f
#define MONSTER_BREAK_TIME     2.2f
#define MONSTER_DIG_RANGE      320.0f
#define MONSTER_BREAK_COOLDOWN 0.3f

#define MONSTER_W        14
#define MONSTER_H        20
#define MONSTER_SPEED    55.0f
#define MONSTER_HP       30
#define MONSTER_DAMAGE   10
#define MONSTER_SIGHT    200.0f

#define SPIDER_W         12
#define SPIDER_H         10
#define SPIDER_SPEED     80.0f
#define SPIDER_HP        15
#define SPIDER_DAMAGE    8
#define SPIDER_SIGHT     240.0f

#define GIANT_W          22
#define GIANT_H          32
#define GIANT_SPEED      30.0f
#define GIANT_HP         90
#define GIANT_DAMAGE     25
#define GIANT_SIGHT      180.0f
#define GIANT_BREAK_TIME 1.2f

#define MAX_PARTICLES    256
#define QUEST_COUNT      5

#define HUNGER_MAX        100.0f
#define HUNGER_DRAIN      1.2f
#define HUNGER_DMG_RATE   2.0f
#define HUNGER_DMG_TICK   1.0f
#define FOOD_DROP_CHANCE  60

#define PATH_LEN            32
#define BFS_MAX             4096
#define BFS_W               80
#define BFS_H               60
#define PATH_RETARGET       1.2f
#define JUMP_COOLDOWN_TIME  1.0f

#define MAX_MOBS         32
#define MOB_IFRAMES      0.4f

#define PIG_W            14
#define PIG_H            12
#define PIG_HP           15
#define PIG_SPEED        45.0f
#define PIG_FLEE_RANGE   80.0f
#define PIG_MEAT_DROP    2

#define RABBIT_W         10
#define RABBIT_H         10
#define RABBIT_HP        6
#define RABBIT_SPEED     90.0f
#define RABBIT_FLEE_RANGE 100.0f
#define RABBIT_MEAT_DROP  1
#define RABBIT_JUMP_INTERVAL 1.2f

typedef enum {
    CHAR_WARRIOR = 0,
    CHAR_SCOUT,
    CHAR_TANK,
    CHAR_COUNT
} CharClass;

typedef struct {
    CharClass   cls;
    const char *name;
    const char *desc;
    Color       tint;
    int         bonusHp;
    float       speedMult;
    float       damageMult;
    float       defenceMult;
    int         startTorches;
} CharDef;

static inline CharDef GetCharDef(CharClass cls) {
    switch (cls) {
        case CHAR_WARRIOR:
            return (CharDef){ CHAR_WARRIOR, "Warrior",
                "  +20 HP  |  +25% sword dmg  ",
                (Color){ 255, 110, 110, 255 }, 20, 1.0f, 1.25f, 1.0f, 5 };
        case CHAR_SCOUT:
            return (CharDef){ CHAR_SCOUT, "Scout",
                "  +30% speed  |  x20 torches  ",
                (Color){ 100, 230, 100, 255 }, 0, 1.3f, 1.0f, 1.0f, 20 };
        case CHAR_TANK:
            return (CharDef){ CHAR_TANK, "Tank",
                "  +40 HP  |  -15% dmg taken  ",
                (Color){ 100, 160, 255, 255 }, 40, 0.9f, 1.0f, 0.85f, 5 };
        default:
            return (CharDef){ CHAR_WARRIOR, "Warrior",
                "  +20 HP  |  +25% sword dmg  ",
                WHITE, 20, 1.0f, 1.25f, 1.0f, 5 };
    }
}

typedef enum {
    TILE_AIR    = 0,
    TILE_GRASS,
    TILE_DIRT,
    TILE_STONE,
    TILE_COAL,
    TILE_IRON,
    TILE_GOLD,
    TILE_WOOD,
    TILE_LEAVES,
    TILE_WATER,
    TILE_TORCH,
    TILE_MEAT,
    TILE_SWORD,
    TILE_COUNT
} TileType;

typedef enum {
    MONSTER_TYPE_ZOMBIE = 0,
    MONSTER_TYPE_SPIDER,
    MONSTER_TYPE_GIANT,
} MonsterType;

typedef enum {
    MOB_PIG = 0,
    MOB_RABBIT,
} MobType;

typedef enum {
    PARTICLE_BLOOD = 0,
    PARTICLE_DUST,
    PARTICLE_SPARK,
} ParticleType;

typedef struct { TileType type; } Tile;

typedef struct {
    Tile tiles[WORLD_H][WORLD_W];
} World;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool    onGround;
    bool    facingLeft;
    int     hp;
    int     maxHp;
    float   iframes;
    float   swordTimer;
    float   hunger;
    float   hungerDmgTimer;
    bool    attacking;
    int     kills;
} Player;

typedef struct {
    Vector2     pos;
    Vector2     vel;
    int         hp;
    int         maxHp;
    float       iframes;
    bool        alive;
    bool        facingLeft;
    MonsterType kind;
    float       breakTimer;
    float       breakCooldown;
    int         breakTX;
    int         breakTY;
    Vector2     path[32];
    int         pathLen;
    int         pathStep;
    float       retargetTimer;
    float       jumpCooldown;
    bool        onWall;
    float       climbDir;
} Monster;

typedef struct {
    Monster list[MAX_MONSTERS];
    int     count;
} Monsters;

typedef struct {
    Vector2  pos;
    Vector2  vel;
    int      hp;
    int      maxHp;
    float    iframes;
    bool     alive;
    bool     facingLeft;
    MobType  kind;
    float    wanderTimer;
    float    jumpTimer;
    bool     onGround;
} Mob;

typedef struct {
    Mob   list[MAX_MOBS];
    int   count;
    float spawnTimer;
} Mobs;

typedef struct {
    Vector2      pos;
    Vector2      vel;
    float        life;
    float        maxLife;
    Color        color;
    float        size;
    ParticleType kind;
} Particle;

typedef struct {
    Particle list[MAX_PARTICLES];
    int      count;
} Particles;

typedef struct {
    TileType type;
    int      count;
} ItemStack;

typedef struct {
    ItemStack hotbar[HOTBAR_SIZE];
    ItemStack bag[INV_SIZE];
    int       activeSlot;
    bool      open;
    int       dragSlot;
    bool      dragFromHotbar;
    ItemStack dragItem;
} Inventory;

typedef struct {
    float   mineCooldown;
    Vector2 hoveredTile;
    bool    hoverValid;
} InputState;

typedef struct {
    float elapsed;
    float hour;
    bool  isDay;
    bool  finished;
} DayNight;

typedef struct {
    const char *title;
    const char *desc;
    bool        done;
} Quest;

typedef struct {
    Quest quests[QUEST_COUNT];
    float progress[QUEST_COUNT];
    int   count;
    bool  show;
    float fadeAlpha;
    bool  wasDone[QUEST_COUNT];
    char  notifText[64];
    float notifTimer;
} QuestLog;

typedef struct {
    World      world;
    Player     player;
    Camera2D   camera;
    InputState input;
    Inventory  inv;
    DayNight   daynight;
    Monsters   monsters;
    Mobs       mobs;
    Particles  particles;
    QuestLog   quests;
    CharClass  selectedChar;
} GameState;

static inline Color TileColor(TileType t) {
    switch (t) {
        case TILE_GRASS:  return (Color){  86, 169,  48, 255 };
        case TILE_DIRT:   return (Color){ 139,  90,  43, 255 };
        case TILE_STONE:  return (Color){ 128, 128, 128, 255 };
        case TILE_COAL:   return (Color){  40,  40,  40, 255 };
        case TILE_IRON:   return (Color){ 200, 160, 120, 255 };
        case TILE_GOLD:   return (Color){ 255, 210,  30, 255 };
        case TILE_WOOD:   return (Color){ 101,  67,  33, 255 };
        case TILE_LEAVES: return (Color){  34, 139,  34, 255 };
        case TILE_WATER:  return (Color){  64, 164, 223, 180 };
        case TILE_TORCH:  return (Color){ 255, 200,  60, 255 };
        case TILE_MEAT:   return (Color){ 200,  80,  80, 255 };
        case TILE_SWORD:  return (Color){ 200, 210, 230, 255 };
        default:          return (Color){   0,   0,   0,   0 };
    }
}

static inline const char *TileName(TileType t) {
    switch (t) {
        case TILE_GRASS:  return "Grass";
        case TILE_DIRT:   return "Dirt";
        case TILE_STONE:  return "Stone";
        case TILE_COAL:   return "Coal";
        case TILE_IRON:   return "Iron";
        case TILE_GOLD:   return "Gold";
        case TILE_WOOD:   return "Wood";
        case TILE_LEAVES: return "Leaves";
        case TILE_WATER:  return "Water";
        case TILE_TORCH:  return "Torch";
        case TILE_MEAT:   return "Meat";
        case TILE_SWORD:  return "Sword";
        default:          return "Air";
    }
}

static inline bool TileIsLiquid(TileType t)    { return t == TILE_WATER; }
static inline bool TileIsPassable(TileType t)  { return t == TILE_WOOD || t == TILE_LEAVES || t == TILE_TORCH; }
static inline bool TileEmitsLight(TileType t)  { return t == TILE_TORCH; }

#endif
