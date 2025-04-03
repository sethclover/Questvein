#pragma once

static const int MESSAGE_LINE = 0;
static const int STATUS_LINE1 = 22;
static const int STATUS_LINE2 = 23;

void printLine(int line, const char *format, ...);
int playGame(int numMonsters, bool autoFlag, bool godmodeFlag, bool supportsColor);
