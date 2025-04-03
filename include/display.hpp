#pragma once

#include "dungeon.hpp"

void printLine(int line, const char *format, ...);
const char personalityToChar(Monster *mon);
void printDungeon(bool supportsColor, bool fogOfWarToggle);
int monsterList(int monstersAlive, bool supportsColor, bool fogOfWarToggle);
void commandList(bool supportsColor, bool fogOfWarToggle);
