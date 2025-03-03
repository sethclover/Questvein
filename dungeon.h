#pragma once

#define MAX_WIDTH 80
#define MAX_HEIGHT 21
#define MAX_HARDNESS 255

#define ATTEMPTS 1000
#define DEFAULT_MONSTER_COUNT 10

#define CORNER '+'
#define VERT_WALL '|'
#define HORZ_WALL '-'
#define FLOOR '.'
#define CORRIDOR '#'
#define STAIR_UP '<'
#define STAIR_DOWN '>'
#define ROCK ' ' 
#define UNREACHABLE 9999

typedef struct Room {
    int width;
    int height;
    int x;
    int y;
} Room;

typedef struct Tile {
    char type;
    int hardness;
    int tunnelingDist;
    int nonTunnelingDist;
} Tile;

typedef struct Position {
    int x;
    int y;
} Pos;

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern Room *rooms;
extern Pos player;
extern Pos *upStairs;
extern int upStairsCount;
extern Pos *downStairs;
extern int downStairsCount;

void initDungeon();
void printDungeon();
void printHardness();
void printTunnelingDistances();
void printNonTunnelingDistances();
void populateDungeon(int numMonsters);
void fillDungeon(int numMonsters);
