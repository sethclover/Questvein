#pragma once

#include <set>
#include <string>
#include <vector>

class Dice {
public:
    int base;
    int rolls;
    int sides;

    Dice(int base, int rolls, int sides) : base(base), rolls(rolls), sides(sides) {}
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
    char symbol;
    int rarity;
    bool eligible = true;

    bool valid = true;
    std::set<std::string> fields;

    MonsterType() = default;
    ~MonsterType() = default;
};

class ObjectType {
public:
    std::string name;
    std::string desc;
    std::vector<std::string> types;
    std::vector<std::string> colors;
    Dice hit;
    Dice dam;
    Dice dodge;
    Dice def;
    Dice weight;
    Dice speed;
    Dice attr;
    Dice val;
    bool art;
    int rarity;
    bool eligible = true;

    bool valid = true;
    std::set<std::string> fields;

    ObjectType() = default;
    ~ObjectType() = default;
};

extern std::vector<MonsterType> monsterTypeList;
extern std::vector<ObjectType> objectTypeList;

int parse(const char *filename);
