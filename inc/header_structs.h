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

#define DAY_DURATION     150.0f
#define NIGHT_DURATION   150.0f
#define CYCLE_DURATION   (DAY_DURATION + NIGHT_DURATION)
#define HOURS_IN_DAY     24
#define TRANSITION_HOURS 1.5f

#define MAX_MONSTERS     64
#define MONSTER_W        14
#define MONSTER_H        20
#define MONSTER_SPEED    55.0f
#define MONSTER_HP       30
#define MONSTER_DAMAGE   10
#define MONSTER_SIGHT    200.0f
#define MONSTER_IFRAMES  0.5f

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
    TILE_COUNT
} TileType;

typedef struct {
    TileType type;
} Tile;

typedef struct {
    Tile tiles[WORLD_H][WORLD_W];
} World;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool    onGround;
    bool    facingLeft;
    int     hp;
    float   iframes;
} Player;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    int     hp;
    float   iframes;
    bool    alive;
    bool    facingLeft;
} Monster;

typedef struct {
    Monster list[MAX_MONSTERS];
    int     count;
} Monsters;

typedef struct {
    TileType type;
    int      count;
} ItemStack;

typedef struct {
    ItemStack hotbar[HOTBAR_SIZE];
    ItemStack bag[INV_SIZE];
    int       activeSlot;
    bool      open;
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
    World      world;
    Player     player;
    Camera2D   camera;
    InputState input;
    Inventory  inv;
    DayNight   daynight;
    Monsters   monsters;
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
        default:          return "Air";
    }
}

static inline bool TileIsLiquid(TileType t)   { return t == TILE_WATER; }
static inline bool TileIsPassable(TileType t)  { return t == TILE_WOOD || t == TILE_LEAVES || t == TILE_TORCH; }
static inline bool TileEmitsLight(TileType t)  { return t == TILE_TORCH; }

#endif
