#pragma once

#include <string>
#include <utility>
#include <vector>

#include "dungeon.hpp"

static const int MESSAGE_LINE = 0;
static const int STATUS_LINE1 = 22;
static const int STATUS_LINE2 = 23;

void printParsedMonsters();
void printParsedObjects();
void fitString(std::string& str, int maxWidth);
void printLine(int line, const char *format, ...);
void printLineColor(int line, Color color, const char *format, ...);
void printStatus();
void redisplayColors();
void redisplayColorsOutsideWindow(int height, int width, int startY, int startX);
void characterInfo();
void openEquipment();
void openInventory();
void printDungeon();
void monsterList();
void objectList();
void showMonsterInfo(Pos pos);
void nonTunnelingDistMap();
void showEquipmentObjectDescription();
void showInventoryObjectDescription();
void viewActions(std::vector<std::pair<std::string, Color>>& actions);
void tunnelingDistMap();
void commandList();
void lossScreen();
void winScreen();
