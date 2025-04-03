#pragma once

#include <vector>

static const int MAX_WIDTH = 80;
static const int MAX_HEIGHT = 21;
static const int MAX_HARDNESS = 255;
static const int ATTEMPTS = 1000;
static const int UNREACHABLE = 9999;

static const char FLOOR = '.';
static const char CORRIDOR = '#';
static const char STAIR_UP = '<';
static const char STAIR_DOWN = '>';
static const char ROCK = ' ';
static const char FOG = '*';

class Room {
public:
    int width;
    int height;
    int x;
    int y;
};

class Tile {
public:
    char type;
    int hardness;
    int tunnelingDist;
    int nonTunnelingDist;
    char visible;
};

class Pos {
public:
    int x;
    int y;
};

class Character {
public:
    Pos pos;
};

class Player : public Character {
public:
    Player() = default;
    Player(int x, int y) {
        pos.x = x;
        pos.y = y;
    }
    ~Player() = default;
};

class Monster : public Character {
public:
    int intelligent;
    int telepathic;
    int tunneling;
    int erratic;
    int speed;
    Pos lastSeen;

    Monster() = default;
    Monster(Pos pos);
    Monster(Pos pos, char monType);
    ~Monster() = default;
};

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern Room *rooms;
extern Pos *upStairs;
extern int upStairsCount;
extern Pos *downStairs;
extern int downStairsCount;

extern Player player;

extern Monster *monsterAt[MAX_HEIGHT][MAX_WIDTH];
extern Monster *monsters;

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
