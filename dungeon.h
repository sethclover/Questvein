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

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];

void generateHardness(Tile dungeon[MAX_HEIGHT][MAX_WIDTH]);