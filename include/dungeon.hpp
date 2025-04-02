#pragma once

static const int MAX_WIDTH = 80;
static const int MAX_HEIGHT = 21;
static const int MAX_HARDNESS = 255;
static const int ATTEMPTS = 1000;
static const int UNREACHABLE = 9999;

static const int CORNER = '+';
static const int VERT_WALL = '|';
static const int HORZ_WALL = '-';
static const int FLOOR = '.';
static const int CORRIDOR = '#';
static const int STAIR_UP = '<';
static const int STAIR_DOWN = '>';
static const int ROCK = ' ';

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

typedef struct Monster {
    int intelligent;
    int telepathic;
    int tunneling;
    int erratic;
    int speed;
    Pos pos;
    Pos lastSeen;
} Mon;

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern Room *rooms;
extern Pos player;
extern Pos *upStairs;
extern int upStairsCount;
extern Pos *downStairs;
extern int downStairsCount;
extern Mon *monsterAt[MAX_HEIGHT][MAX_WIDTH];
extern Mon **monsters;

void initDungeon();
void initRoom(Room *roomsLoaded);
void spawnPlayer();
void printHardness();
void printTunnelingDistances();
void printNonTunnelingDistances();
int spawnMonsterWithMonType(char monType);
int spawnMonsters(int numMonsters, int playerX, int playerY);
int generateStructures(int numMonsters);
void freeAll(int numMonsters);
