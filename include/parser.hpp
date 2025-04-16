#pragma once

#include <set>
#include <string>
#include <vector>

class Dice {
public:
    int base;
    int rolls;
    int sides;

    Dice() : base(0), rolls(0), sides(0) {}
    ~Dice() = default;
};

class MonsterType {
public:
    std::string name;
    std::string desc;
    std::vector<std::string> colors;
    Dice speed;
    std::vector<std::string> abils;
    Dice hp;
    Dice dam;
    char symbol = '\0';
    int rarity = 0;
    bool eligible = true;

    bool valid = true;
    std::set<std::string> fields;

    MonsterType() = default;
    ~MonsterType() = default;
};

class ObjectType { // next
public:
    std::string name, desc;
    char symbol = '\0';
    std::vector<std::string> colors, type;
    Dice hit, dam, dodge, def, weight, speed, attr, val;
    bool art;
    int rarity = 0;

    bool valid = true;
    std::set<std::string> fields;

    ObjectType() = default;
    ~ObjectType() = default;
};

extern std::vector<MonsterType> monsterTypeList;
extern std::vector<ObjectType> objectTypeList;

int parse(const char *filename);
