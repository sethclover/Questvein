#pragma once

#include <vector>

#include "parser.hpp"

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

enum class Color {
    Black = 1,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

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

// private this
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
private:
    int monTypeIndex;

    std::string name;

    std::string description;

    std::vector<Color> colors;
    int colorCount;
    int colorIndex;

    int speed;

    bool intelligent;
    bool telepathic;
    bool tunneling;
    bool erratic;
    bool pass;
    bool pickup;
    bool destroy;
    bool unique;
    bool boss;
    
    int hitpoints;

    Dice damage;

    char symbol;
    
    int rarity;

    Pos lastSeen;

public:
    int getMonTypeIndex() { return monTypeIndex; }

    std::string getName() { return name; }

    std::string getDescription() { return description; }

    Color getColor() {
        if (colorIndex >= colorCount) {
            colorIndex = 0;
        }
        return colors[colorIndex++];
    }

    int getSpeed() { return speed; }

    bool isIntelligent() { return intelligent; }
    bool isTelepathic() { return telepathic; }
    bool isTunneling() { return tunneling; }
    bool isErratic() { return erratic; }
    bool canPass() { return pass; }
    bool canPickup() { return pickup; }
    bool canDestroy() { return destroy; }
    bool isUnique() { return unique; }
    bool isBoss() { return boss; }

    int getHitpoints() { return hitpoints; }

    int getDamage() {
        return damage.base + damage.rolls * (rand() % damage.sides + 1);
    }

    char getSymbol() { return symbol; }

    int getRarity() { return rarity; }

    Pos getLastSeen() { return lastSeen; }
    void setLastSeen(Pos p) { lastSeen = p; }

    Pos getPos() { return pos; }
    void setPos(Pos p) { pos = p; }

    Monster(MonsterType* monType, int monTypeIndex, Pos pos) {
        this->monTypeIndex = monTypeIndex;

        name = monType->name;

        description = monType->desc;
        
        for (const auto& color : monType->colors) {
            if (color == "BLACK") { colors.push_back(Color::Black); }
            else if (color == "RED") { colors.push_back(Color::Red); }
            else if (color == "GREEN") { colors.push_back(Color::Green); }
            else if (color == "YELLOW") { colors.push_back(Color::Yellow); }
            else if (color == "BLUE") { colors.push_back(Color::Blue); }
            else if (color == "MAGENTA") { colors.push_back(Color::Magenta); }
            else if (color == "CYAN") { colors.push_back(Color::Cyan); }
            else if (color == "WHITE") { colors.push_back(Color::White); }
        }
        colorCount = colors.size();
        colorIndex = 0;

        speed = monType->speed.base + monType->speed.rolls * (rand() % monType->speed.sides + 1);

        intelligent = false;
        telepathic = false;
        tunneling = false;
        erratic = false;
        pass = false;
        pickup = false;
        destroy = false;
        unique = false;
        boss = false;
        for (const auto& abil : monType->abils) {
            if (abil == "SMART") { intelligent = true; }
            else if (abil == "TELE") { telepathic = true; }
            else if (abil == "TUNNEL") { tunneling = true; }
            else if (abil == "ERRATIC") { erratic = true; }
            else if (abil == "PASS") { pass = true; }
            else if (abil == "PICKUP") { pickup = true; }
            else if (abil == "DESTROY") { destroy = true; }
            else if (abil == "UNIQ") { unique = true; }
            else if (abil == "BOSS") { boss = true; }
        }

        hitpoints = monType->hp.base + monType->hp.rolls * (rand() % monType->hp.sides + 1);

        damage = monType->dam;

        symbol = monType->symbol;

        rarity = monType->rarity;

        lastSeen = {-1, -1};

        this->pos = pos;
    }
    Monster() = delete;
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
extern std::vector<Monster> monsters;

void initDungeon();
void initRoom(Room *roomsLoaded);
void spawnPlayer();
void printHardness();
void printTunnelingDistances();
void printNonTunnelingDistances();
int spawnMonsterWithMonType(char monType);
int spawnMonsters(int numMonsters, int playerX, int playerY);
int generateStructures();
void freeAll(int numMonsters);
