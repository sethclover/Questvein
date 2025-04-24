#pragma once

#include "dungeon.hpp"

static const int MESSAGE_LINE = 0;
static const int STATUS_LINE1 = 22;
static const int STATUS_LINE2 = 23;

void printParsedMonsters();
void printParsedObjects();
void printLine(int line, const char *format, ...);
void printStatus(bool supportsColor);
void openEquipment(bool supportsColor, bool fogOfWarToggle);
void openInventory(bool supportsColor, bool fogOfWarToggle);
void printDungeon(bool supportsColor, bool fogOfWarToggle);
void monsterList(bool supportsColor, bool fogOfWarToggle);
void objectList(bool supportsColor, bool fogOfWarToggle);
void commandList(bool supportsColor, bool fogOfWarToggle);
void tunnelingDistMap(bool supportsColor, bool fogOfWarToggle);
void showObjectDescription(bool supportsColor, bool fogOfWarToggle);
void nonTunnelingDistMap(bool supportsColor, bool fogOfWarToggle);
void lossScreen(bool supportsColor);
void winScreen(bool supportsColor);
