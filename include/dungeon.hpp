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

struct Pos {
    int x;
    int y;
};    

class Room {
private:
    const Pos pos;
    const int width;
    const int height;

public:
    Pos getPos() { return pos; }
    int getWidth() { return width; }
    int getHeight() { return height; }

    Room(Pos pos, int width, int height) : pos(pos), width(width), height(height) {}
    ~Room() = default;
};

class Tile {
public:
    char type;
    int hardness;
    int tunnelingDist;
    int nonTunnelingDist;
    char visible;
};

class Character {
protected:
    Pos pos;
};

class Player : public Character {
public:
    Pos getPos() { return pos; }
    void setPos(Pos p) { pos = p; }

    Player() = default;
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

class Object {
private:
    int objTypeIndex;

    std::string name;

    std::string description;

    std::vector<std::string> types;

    std::vector<Color> colors;
    int colorCount;
    int colorIndex;

    int hitBonus;

    int damageBonus;

    int dodgeBonus;

    int defenseBonus;

    int weight;

    int speedBonus;

    int specialAttribute;

    int value;

    bool artifact;

    char symbol;

    int rarity;

    Pos pos;

public:
    int getObjTypeIndex() { return objTypeIndex; }

    std::string getName() { return name; }

    std::string getDescription() { return description; }

    std::vector<std::string> getTypes() { return types; }

    Color getColor() {
        if (colorIndex >= colorCount) {
            colorIndex = 0;
        }
        return colors[colorIndex++];
    }

    int getHitBonus() { return hitBonus; }

    int getDamageBonus() { return damageBonus; }

    int getDodgeBonus() { return dodgeBonus; }

    int getDefenseBonus() { return defenseBonus; }

    int getWeight() { return weight; }

    int getSpeedBonus() { return speedBonus; }

    int getSpecialAttribute() { return specialAttribute; }

    int getValue() { return value; }

    bool isArtifact() { return artifact; }

    char getSymbol() { return symbol; }

    int getRarity() { return rarity; }

    Pos getPos() { return pos; }
    void setPos(Pos p) { pos = p; }

    Object(ObjectType* objType, int objTypeIndex, Pos pos) {
        this->objTypeIndex = objTypeIndex;

        name = objType->name;

        description = objType->desc;

        types = objType->types;

        for (const auto& color : objType->colors) {
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

        hitBonus = objType->hit.base + objType->hit.rolls * (rand() % objType->hit.sides + 1);

        damageBonus = objType->dam.base + objType->dam.rolls * (rand() % objType->dam.sides + 1);

        dodgeBonus = objType->dodge.base + objType->dodge.rolls * (rand() % objType->dodge.sides + 1);

        defenseBonus = objType->def.base + objType->def.rolls * (rand() % objType->def.sides + 1);

        weight = objType->weight.base + objType->weight.rolls * (rand() % objType->weight.sides + 1);

        speedBonus = objType->speed.base + objType->speed.rolls * (rand() % objType->speed.sides + 1);

        specialAttribute = objType->attr.base + objType->attr.rolls * (rand() % objType->attr.sides + 1);

        value = objType->val.base + objType->val.rolls * (rand() % objType->val.sides + 1);

        artifact = objType->art;

        if (types.size() == 2) {
            symbol = ')';
        }
        if (types.size() == 1) {
            if (types.front() == "WEAPON") { symbol = '|'; }
            else if (types.front() == "OFFHAND") { symbol = ')'; }
            else if (types.front() == "RANGED") { symbol = '}'; }
            else if (types.front() == "ARMOR") { symbol = '['; }
            else if (types.front() == "HELMET") { symbol = ']'; }
            else if (types.front() == "CLOAK") { symbol = '('; }
            else if (types.front() == "GLOVES") { symbol = '{'; }
            else if (types.front() == "BOOTS") { symbol = '\\'; }
            else if (types.front() == "RING") { symbol = '='; }
            else if (types.front() == "AMULET") { symbol = '"'; }
            else if (types.front() == "LIGHT") { symbol = '_'; }
            else if (types.front() == "SCROLL") { symbol = '~'; }
            else if (types.front() == "BOOK") { symbol = '?'; }
            else if (types.front() == "FLASK") { symbol = '!'; }
            else if (types.front() == "GOLD") { symbol = '$'; }
            else if (types.front() == "AMMUNITION") { symbol = '/'; }
            else if (types.front() == "FOOD") { symbol = ','; }
            else if (types.front() == "WAND") { symbol = '-'; }
            else if (types.front() == "CONTAINER") { symbol = '%'; }
            else { symbol = '*'; }
        }

        rarity = objType->rarity;

        this->pos = pos;
    }
    Object() = delete;
    ~Object() = default;
};

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern std::vector<Room> rooms;
extern std::vector<Pos> upStairs;
extern int upStairsCount;
extern std::vector<Pos> downStairs;
extern int downStairsCount;

extern Player player;

extern Monster *monsterAt[MAX_HEIGHT][MAX_WIDTH];
extern std::vector<Monster> monsters;

extern std::vector<Object*> objectAt[MAX_HEIGHT][MAX_WIDTH];
extern std::vector<Object> objects;

void initDungeon();
void spawnPlayer();
void printHardness();
int spawnMonsterWithMonType(char monType);
int spawnMonsters(int numMonsters, int playerX, int playerY);
int spawnObjects(int numObjects);
int generateStructures();
void clearAll();
