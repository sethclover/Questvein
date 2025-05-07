#pragma once

#include <cmath>
#include <array>
#include <memory>
#include <ncurses.h>
#include <string>
#include <utility>
#include <vector>

#include "parser.hpp"

static const char FLOOR = '.';
static const char CORRIDOR = '#';
static const char STAIR_UP = '<';
static const char STAIR_DOWN = '>';
static const char ROCK = ' ';
static const char FOG = ' ';

static const int MAX_WIDTH = 80;
static const int MAX_HEIGHT = 21;
static const int MAX_HARDNESS = 255;
static const int ATTEMPTS = 1000;
static const int UNREACHABLE = 9999;

// change with class/race
static const int BASE_HP = 100;
static const int BASE_HIT_BONUS = 0;
static const int BASE_DODGE_BONUS = 0;
static const int BASE_DEFENSE = 0;
static const int BASE_SPEED = 10;

static const int INVENTORY_SIZE = 10;
static const int DEFENSE_SCALE = 50;
static const int HIT_SCALE = 75;

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

enum class Equip {
    Weapon,
    Offhand,
    Ranged,
    Armor,
    Helmet,
    Cloak,
    Gloves,
    Boots,
    Amulet,
    Light,
    Ring1,
    Ring2,
    Count,
    None
};

struct Pos {
    int x;
    int y;

    bool operator==(const Pos& other) const {
        return x == other.x && y == other.y;
    }
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

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern std::vector<Room> rooms;
extern std::vector<Pos> upStairs;
extern int upStairsCount;
extern std::vector<Pos> downStairs;
extern int downStairsCount;

class Object {
private:
    int objTypeIndex;
    std::string name;
    std::string description;
    std::vector<std::string> types;
    std::string typeString;
    Equip equipIndex;
    std::vector<Color> colors;
    int colorCount;
    int colorIndex;
    int hitBonus;
    Dice damageBonus;
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
    std::string getTypeString() { return typeString; }

    bool isTwoHanded() {
        return types.size() == 2;
    }
    Equip getEquipmentIndex() {
        return equipIndex;
    }
    void setEquipmentIndex(Equip index) {
        equipIndex = index;
    }

    Color getColor() {
        if (colorIndex >= colorCount) {
            colorIndex = 0;
        }
        return colors[colorIndex++];
    }

    int getHitBonus() { return hitBonus; }
    Dice getDamageBonus() { return damageBonus; }
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
        damageBonus = Dice(objType->dam.base, objType->dam.rolls, objType->dam.sides);
        dodgeBonus = objType->dodge.base + objType->dodge.rolls * (rand() % objType->dodge.sides + 1);
        defenseBonus = objType->def.base + objType->def.rolls * (rand() % objType->def.sides + 1);
        weight = objType->weight.base + objType->weight.rolls * (rand() % objType->weight.sides + 1);
        speedBonus = objType->speed.base + objType->speed.rolls * (rand() % objType->speed.sides + 1);
        specialAttribute = objType->attr.base + objType->attr.rolls * (rand() % objType->attr.sides + 1);
        value = objType->val.base + objType->val.rolls * (rand() % objType->val.sides + 1);
        artifact = objType->art;
        if (types.size() == 2) {
            symbol = ')'; equipIndex = Equip::Weapon; typeString = "Weapon";
        }
        if (types.size() == 1) {
            if (types.front() == "WEAPON") { symbol = '|'; equipIndex = Equip::Weapon; typeString = "Weapon"; }
            else if (types.front() == "OFFHAND") { symbol = ')'; equipIndex = Equip::Offhand; typeString = "Offhand"; }
            else if (types.front() == "RANGED") { symbol = '}'; equipIndex = Equip::Ranged; typeString = "Ranged"; }
            else if (types.front() == "ARMOR") { symbol = '['; equipIndex = Equip::Armor; typeString = "Armor"; }
            else if (types.front() == "HELMET") { symbol = ']'; equipIndex = Equip::Helmet; typeString = "Helmet"; }
            else if (types.front() == "CLOAK") { symbol = '('; equipIndex = Equip::Cloak; typeString = "Cloak"; }
            else if (types.front() == "GLOVES") { symbol = '{'; equipIndex = Equip::Gloves; typeString = "Gloves"; }
            else if (types.front() == "BOOTS") { symbol = '\\'; equipIndex = Equip::Boots; typeString = "Boots"; }
            else if (types.front() == "RING") { symbol = '='; equipIndex = Equip::Ring1; typeString = "Ring"; }
            else if (types.front() == "AMULET") { symbol = '"'; equipIndex = Equip::Amulet; typeString = "Ring"; }
            else if (types.front() == "LIGHT") { symbol = '_'; equipIndex = Equip::Light; typeString = "Light"; }
            else if (types.front() == "SCROLL") { symbol = '~'; equipIndex = Equip::None; typeString = "Scroll"; }
            else if (types.front() == "BOOK") { symbol = '?'; equipIndex = Equip::None; typeString = "Book"; }
            else if (types.front() == "FLASK") { symbol = '!'; equipIndex = Equip::None; typeString = "Flask"; }
            else if (types.front() == "GOLD") { symbol = '$'; equipIndex = Equip::None; typeString = "Gold"; }
            else if (types.front() == "AMMUNITION") { symbol = '/'; equipIndex = Equip::None; typeString = "Ammunition"; }
            else if (types.front() == "FOOD") { symbol = ','; equipIndex = Equip::None; typeString = "Food"; }
            else if (types.front() == "WAND") { symbol = '-'; equipIndex = Equip::None; typeString = "Wand"; }
            else if (types.front() == "CONTAINER") { symbol = '%'; equipIndex = Equip::None; typeString = "Container"; }
            else { symbol = '*'; }
        }
        rarity = objType->rarity;
        this->pos = pos;
    }
    Object() = delete;
    ~Object() = default;
};

extern std::vector<std::unique_ptr<Object>> objectsAt[MAX_HEIGHT][MAX_WIDTH];

class Character {
protected:
    Pos pos;
    int maxHitpoints;
    int hitpoints;
    int hitBonus;
    int dodgeBonus;
    int defense;
    int speed;
    std::array<std::unique_ptr<Object>, static_cast<int>(Equip::Count)> equipment;
    std::array<std::unique_ptr<Object>, static_cast<int>(Equip::Count)> inventory;

public:
    Pos getPos() { return pos; }
    void setPos(Pos p) { pos = p; }

    int getHitpoints() { return hitpoints; }
    int getMaxHitpoints() { return maxHitpoints; }
    int takeDamage(int damage) {
        int damageTaken = std::round(damage * (1.0 - static_cast<double>(defense) / (defense + DEFENSE_SCALE))); 

        hitpoints -= damageTaken;
        if (hitpoints < 0) {
            hitpoints = 0;
        }
        return damageTaken;
    }

    bool attemptHit(int defendingDodgeBonus) {
        int hitCheck = rand() % 100 + 1;
        int effectiveBonus = this->hitBonus - defendingDodgeBonus;
        return hitCheck < HIT_SCALE + effectiveBonus;
    }

    int getHitBonus() { return hitBonus; }

    int getDodgeBonus() { return dodgeBonus; }

    int getDefense() { return defense > 0 ? defense : 0; }

    int getSpeed() { return speed > 0 ? speed : 1; }

    Object *getEquipmentItem(Equip e) {
        return equipment[static_cast<int>(e)].get();
    }
    const char* getEquipmentName(int index) {
        if (index == 0) { return "Weapon"; }
        else if (index == 1) { return "Offhand"; }
        else if (index == 2) { return "Ranged"; }
        else if (index == 3) { return "Armor"; }
        else if (index == 4) { return "Helmet"; }
        else if (index == 5) { return "Cloak"; }
        else if (index == 6) { return "Gloves"; }
        else if (index == 7) { return "Boots"; }
        else if (index == 8) { return "Amulet"; }
        else if (index == 9) { return "Light"; }
        else if (index == 10) { return "Left Ring"; }
        else if (index == 11) { return "Right Ring"; }
        else { return "Unknown"; }
    }
    void equip(int index) {
        Equip e = inventory[index].get()->getEquipmentIndex();
        equipment[static_cast<int>(e)] = std::move(inventory[index]);
        Object *obj = equipment[static_cast<int>(e)].get();

        hitBonus += obj->getHitBonus();
        dodgeBonus += obj->getDodgeBonus();
        defense += obj->getDefenseBonus();
        // weight
        speed += obj->getSpeedBonus();
    }
    void swapEquipment(int index) {
        std::unique_ptr<Object> tmp = std::move(equipment[static_cast<int>(inventory[index]->getEquipmentIndex())]);
        equip(index);
        inventory[index] = std::move(tmp);
    }
    bool unequip(Equip e) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventory[i] == nullptr) {
                inventory[i] = std::move(equipment[static_cast<int>(e)]);

                Object *obj = inventory[i].get();
                hitBonus -= obj->getHitBonus();
                dodgeBonus -= obj->getDodgeBonus();
                defense -= obj->getDefenseBonus();
                // weight
                speed -= obj->getSpeedBonus();

                return true;
            }
        }
        return false;
    }

    Object *getInventoryItem(int index) {
        return inventory[index].get();
    }
    bool inventoryFull() {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventory[i] == nullptr) {
                return false;
            }
        }
        return true;
    }
    bool addToInventory(Pos pos) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventory[i] == nullptr) {
                inventory[i] = std::move(objectsAt[pos.y][pos.x].back());
                objectsAt[pos.y][pos.x].pop_back();
                return true;
            }
        }
        return false;
    }
    void dropFromInventory(int index) {
        objectsAt[pos.y][pos.x].push_back(std::move(inventory[index]));
    }
    void expungeFromInventory(int index) {
        inventory[index] = nullptr;
    }
};

class Player : public Character {
public:
    int doDamage() {
        int damage = 0;
        for (std::unique_ptr<Object>& obj : equipment) {
            if (obj == nullptr) { continue; }
            damage += obj.get()->getDamageBonus().base + obj.get()->getDamageBonus().rolls * (rand() % obj.get()->getDamageBonus().sides + 1);
        }
        if (equipment[static_cast<int>(Equip::Weapon)] == nullptr) {
            damage += 0 + 1 * (rand() % 4 + 1);
        }
        return damage;
    }

    int heal(int amount) {
        if (amount < 0) {
            return -takeDamage(-amount);
        }

        int hpBefore = hitpoints;
        hitpoints += amount;
        if (hitpoints > maxHitpoints) {
            hitpoints = maxHitpoints;
        }
        else if (hitpoints < 0) {
            hitpoints = 0;
        }
        return hitpoints - hpBefore;
    }

    Player(Pos pos) {
        this->pos = pos;
        maxHitpoints = BASE_HP;
        hitpoints = maxHitpoints;
        hitBonus = BASE_HIT_BONUS;
        dodgeBonus = BASE_DODGE_BONUS;
        defense = BASE_DEFENSE;
        speed = BASE_SPEED;
    }
    Player() = delete;
    ~Player() = default;
};

extern Player player;

class Monster : public Character {
private:
    int monTypeIndex;
    std::string name;
    std::string description;
    std::vector<Color> colors;
    int colorCount;
    int colorIndex; 
    bool intelligent;
    bool telepathic;
    bool tunneling;
    bool erratic;
    bool pass;
    bool pickup;
    bool destroy;
    bool unique;
    bool boss;
    Dice dam;
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

    int doDamage() {
        int damage = 0;
        for (std::unique_ptr<Object>& obj : equipment) {
            if (obj == nullptr) { continue; }
            damage += obj.get()->getDamageBonus().base + obj.get()->getDamageBonus().rolls * (rand() % obj.get()->getDamageBonus().sides + 1);
        }
        damage += dam.base + dam.rolls * (rand() % dam.sides + 1);
        return damage;
    }

    bool isIntelligent() { return intelligent; }
    bool isTelepathic() { return telepathic; }
    bool isTunneling() { return tunneling; }
    bool isErratic() { return erratic; }
    bool canPass() { return pass; }
    bool canPickup() { return pickup; }
    bool canDestroy() { return destroy; }
    bool isUnique() { return unique; }
    bool isBoss() { return boss; }

    char getSymbol() { return symbol; }
    int getRarity() { return rarity; }

    Pos getLastSeen() { return lastSeen; }
    void setLastSeen(Pos p) { lastSeen = p; }

    Monster(MonsterType* monType, int monTypeIndex, Pos pos) {
        this->pos = pos;
        maxHitpoints = monType->hp.base + monType->hp.rolls * (rand() % monType->hp.sides + 1);
        hitpoints = maxHitpoints;
        hitBonus = BASE_HIT_BONUS;
        dodgeBonus = BASE_DODGE_BONUS;
        defense = BASE_DEFENSE;
        speed = monType->speed.base + monType->speed.rolls * (rand() % monType->speed.sides + 1);

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
        dam = Dice(monType->dam.base, monType->dam.rolls, monType->dam.sides);
        symbol = monType->symbol;
        rarity = monType->rarity;
        lastSeen = {-1, -1};
    }
    Monster() = delete;
    ~Monster() = default;  
};

extern std::unique_ptr<Monster> monsterAt[MAX_HEIGHT][MAX_WIDTH];
extern std::vector<std::unique_ptr<Monster>> monsters;

void initDungeon();
void spawnPlayer();
void printHardness();
int spawnMonsterWithMonType(char monType);
int spawnMonsters(int numMonsters, int playerX, int playerY);
int spawnObjects(int numObjects);
int generateStructures();
void clearAll();
