#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "dungeon.h"
#include "errorHandle.h"
#include "game.h"
#include "pathFinding.h"
#include "saveLoad.h"

int ncursesFlag;

typedef struct SwitchInfo {
    const char *shortOp;
    const char *longOp;
    const char *desc;
} SwitchInfo;

static const SwitchInfo switches[] = {
    {"-h", "--help", "Display this help message and exit"},
    {"-hb", "--printhardb", "Print hardness before dungeon generation"},
    {"-ha", "--printharda", "Print hardness after dungeon generation"},
    {"-d", "--printdist", "Print tunneling and non-tunneling distances"},
    {"-s", "--save", "Save the dungeon to a file (requires filename)"},
    {"-l", "--load", "Load a dungeon from a file (requires filename)"},
    {"-n", "--nummon", "Set the number of monsters (requires positive integer)"},
    {"-t", "--montype", "Set a specific monster type (requires a single Hex character)"},
    {"-a", "--auto", "Run the game in automatic (random) movement mode"}
};

static const int numSwitches = sizeof(switches) / sizeof(SwitchInfo);

int main(int argc, char *argv[]) {
    int printhardbFlag = 0;
    int printhardaFlag = 0;
    int printdistFlag = 0;
    int saveFlag = 0;
    int loadFlag = 0;
    int monTypeFlag = 0;
    int autoFlag = 0;

    ncursesFlag = 0;    

    srand(time(NULL));

    char filename[256];
    char monType = '0';
    int numMonsters = rand() % 14 + 7;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: Dungeon Crawler [options]\n"
                    "\nAvailable options:\n");
            for (int i = 0; i < numSwitches; i++) {
                printf("  %-4s, %-13s:  %s\n", switches[i].shortOp, switches[i].longOp, switches[i].desc);
            }
            printf("\nExamples:\n"
                    "    ./dungeon --save test1.rlg327 -a\n"
                    "    ./dungeon -l test1.rlg327 --nummon 10 -d\n"
                    "\nTry the print hardness before option!\n\n");
            return 0;
        }
        else if (!strcmp(argv[i], "-hb") || !strcmp(argv[i], "--printhardb")) {
            printhardbFlag = 1;
        }
        else if (!strcmp(argv[i], "-ha") || !strcmp(argv[i], "--printharda")) {
            printhardaFlag = 1;
        }
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--printdist")) {
            printdistFlag = 1;
        }
        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--save")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    errorHandle("Error: Argument after '--save/-s' must be file name");
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    saveFlag = 1;
                }
            }
            else {
                errorHandle("Error: Argument '--save/-s' requires a file name");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--load")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    errorHandle("Error: Argument after '--load/-l' must be file name");
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    loadFlag = 1;
                } 
            }
            else {
                errorHandle("Error: Argument '--load/-l' requires a file name");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--nummon")) {
            if (i < argc - 1) {
                char *next = argv[i + 1];

                int isNum = 1;
                for (int j = 0; next[j] != '\0'; j++) {
                    if (!isdigit(next[j])) {
                        isNum = 0;
                        break;
                    }
                }
                
                if (isNum && strlen(next) > 0) {
                    numMonsters = atoi(next);
                    if (numMonsters < 0) {
                        errorHandle("Error: Number of monsters must be positive");
                        return 1;
                    }
                } else {
                    errorHandle("Error: Argument after '--nummon/-n' must be a positive integer");
                    return 1;
                }
            } else {
                errorHandle("Error: Argument '--nummon/-n' requires a positive integer");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--montype")) {
            if (i < argc - 1) {
                if (strlen(argv[i + 1]) == 1) {
                    monType = argv[i + 1][0];
                    numMonsters = 1;
                    monTypeFlag = 1;
                }
                else {
                    errorHandle("Error: Argument after '--montype/-t' must be a single character");
                    return 1;
                }
            }
            else {
                errorHandle("Error: Argument '--montype/-t' requires a single character");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--auto")) {
            autoFlag = 1;
        }
        else {
            errorHandle("Error: Unrecognized argument, use '--help/-h' for usage information");
            return 1;
        }
    }
    
    if (loadFlag) {
        if (printhardbFlag) {
            errorHandle("Error: Argument '--printhardb/-hb' cannot be used with '--load/-l'");
            return 1;
        }
        loadDungeon(filename);

        if (monTypeFlag) {
            if (populateDungeonWithMonType(monType)) {
                return 1;
            }
        }
        else {
            if (populateDungeon(numMonsters)) {
                return 1;
            }
        }
    }
    else {
        initDungeon();
        if (printhardbFlag) {
            printHardness();
        }

        if (monTypeFlag) {
            if (fillDungeonWithMonType(monType)) {
                return 1;
            }
        }
        else {
            if (fillDungeon(numMonsters)) {
                return 1;
            }
        }
    }

    if (printhardaFlag) {
        printHardness();
    }

    if (saveFlag && !loadFlag) {
        saveDungeon(filename);
    }

    if (printdistFlag) {
        printTunnelingDistances();
        printNonTunnelingDistances();       
    }

    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    ncursesFlag = 1;

    printLine(MESSAGE_LINE, "Welcome adventurer! Press any key to begin...");
    getch();

    if (playGame(numMonsters, autoFlag)) {
        return 1;
    }

    endwin();
    return 0;
}
