#pragma once

#define MESSAGE_LINE 0
#define STATUS_LINE1 22
#define STATUS_LINE2 23

void printLine(int line, char *format, ...);
int playGame(int numMonsters, int autoFlag, int godmodeFlag);
