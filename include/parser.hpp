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
    std::string name, desc;
    char symbol = '\0';
    std::vector<std::string> color, abils;
    Dice speed, hp, dam;
    int rarity = 0;

    bool valid = true;
    std::set<std::string> fields;

    MonsterType() = default;
    ~MonsterType() = default;
};

struct ObjectType {
public:
    std::string name, desc;
    char symbol = '\0';
    std::vector<std::string> color, type;
    Dice hit, dam, dodge, def, weight, speed, attr, val;
    bool art;
    int rarity = 0;

    bool valid = true;
    std::set<std::string> fields;

    ObjectType() = default;
    ~ObjectType() = default;
};

int parse(const char *filename);
