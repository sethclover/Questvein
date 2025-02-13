#pragma once

#define MAX_WIDTH 80
#define MAX_HEIGHT 21
#define MAX_HARDNESS 255

#define ATTEMPTS 1000

#define CORNER '+'
#define VERT_WALL '|'
#define HORZ_WALL '-'
#define FLOOR '.'
#define CORRIDOR '#'
#define STAIR_UP '<'
#define STAIR_DOWN '>'
#define ROCK ' ' 

typedef struct {
    int width;
    int height;
    int x;
    int y;
} Room;

typedef struct {
    char type;
    int hardness;
} Tile;

typedef struct {
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

void generateHardness();

void initDungeon();
void printDungeon();
void printHardness();
void fillDungeon();

void loadDungeon(char *filename);
void saveDungeon(char *filename);
