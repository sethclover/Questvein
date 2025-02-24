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

typedef struct FibNode {
    Pos pos;
    int key;
    int degree;
    struct FibNode *parent;
    struct FibNode *child;
    struct FibNode *left;
    struct FibNode *right;
    int marked;
} FibNode;

typedef struct FibHeap {
    int numNodes;
    FibNode *min;
} FibHeap;

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern Room *rooms;
extern Pos player;
extern Pos *upStairs;
extern int upStairsCount;
extern Pos *downStairs;
extern int downStairsCount;

// Perlin
void generateHardness();

// Dungeon Generation
void initDungeon();
void printDungeon();
void printHardness();
void fillDungeon();
void printTunnelingDistances();
void printNonTunnelingDistances();

// Load Save
void loadDungeon(char *filename);
void saveDungeon(char *filename);

// Pathfinding
void generateDistances();

// Fibonacci Heap
FibHeap *createFibHeap();
FibNode *insert(FibHeap *heap, int key, Pos pos);
FibNode *getMin(FibHeap *heap);
FibNode *extractMin(FibHeap *heap);
void decreaseKey(FibHeap *heap, FibNode *node, int newKey);
void destroyFibHeap(FibHeap *heap);
