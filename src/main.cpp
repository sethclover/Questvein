#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ncurses.h>

#include "dungeon.hpp"
#include "game.hpp"
#include "pathFinding.hpp"
#include "saveLoad.hpp"


class SwitchInfo {
public:
    const char *shortOp;
    const char *longOp;
    const char *desc;
};

static const SwitchInfo switches[] = {
    {"-h", "--help", "Display this help message and exit"},
    {"-hb", "--printhardb", "After the game ends, print the hardness map before the dungeon structures are added"},
    {"-ha", "--printharda", "After the game ends, print the hardness map after dungeon structures are added"},
    {"-d", "--printdist", "After the game ends, print tunneling and non-tunneling distances"},
    {"-s", "--save", "Save the dungeon to a file (requires filename)"},
    {"-l", "--load", "Load a dungeon from a file (requires filename)"},
    {"-n", "--nummon", "Set the number of monsters (requires positive integer)"},
    {"-t", "--typemon", "Set a specific monster type (requires a single Hex character)"},
    {"-a", "--auto", "Run the game in automatic (random) movement mode"},
    {"-g", "--godmode", "Enable god mode (invincible player)"}
};

static const int numSwitches = sizeof(switches) / sizeof(SwitchInfo);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    bool printhardbFlag = false;
    bool printhardaFlag = false;
    bool printdistFlag = false;
    bool saveFlag = false;
    bool loadFlag = false;
    bool monTypeFlag = false;
    bool autoFlag = false;
    bool godmodeFlag = false;

    bool supportsColor = false;

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
            printhardbFlag = true;
        }
        else if (!strcmp(argv[i], "-ha") || !strcmp(argv[i], "--printharda")) {
            printhardaFlag = true;
        }
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--printdist")) {
            printdistFlag = true;
        }
        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--save")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    std::cout << "Error: Argument after '--save/-s' must be file name" << std::endl;
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    saveFlag = true;
                }
            }
            else {
                std::cout << "Error: Argument '--save/-s' requires a file name" << std::endl;
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--load")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    std::cout << "Error: Argument after '--load/-l' must be file name" << std::endl;
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    loadFlag = true;
                } 
            }
            else {
                std::cout << "Error: Argument '--load/-l' requires a file name" << std::endl;
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
                        std::cout << "Error: Number of monsters must be positive" << std::endl;
                        return 1;
                    }
                } else {
                    std::cout << "Error: Argument after '--nummon/-n' must be a positive integer" << std::endl;
                    return 1;
                }
            } else {
                std::cout << "Error: Argument '--nummon/-n' requires a positive integer" << std::endl;
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--typemon")) {
            if (i < argc - 1) {
                if (strlen(argv[i + 1]) == 1) {
                    monType = argv[i + 1][0];
                    numMonsters = 1;
                    monTypeFlag = true;
                }
                else {
                    std::cout << "Error: Argument after '--typemon/-t' must be a single character" << std::endl;
                    return 1;
                }
            }
            else {
                std::cout << "Error: Argument '--typemon/-t' requires a single character" << std::endl;
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--auto")) {
            autoFlag = true;
        }
        else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--godmode")) {
            godmodeFlag = true;
        }
        else {
            std::cout << "Error: Unrecognized argument, use '--help/-h' for usage information" << std::endl;
            return 1;
        }
    }
    
    if (loadFlag) {
        if (printhardbFlag) {
            std::cout << "Error: Argument '--printhardb/-hb' cannot be used with '--load/-l'" << std::endl;
            return 1;
        }
        loadDungeon(filename);

        if (monTypeFlag) {
            if (spawnMonsterWithMonType(monType)) {
                return 1;
            }
        }
        else {
            if (spawnMonsters(numMonsters, player.pos.x, player.pos.y)) {
                return 1;
            }
        }
    }
    else {
        initDungeon();
        if (printhardbFlag) {
            printHardness();
        }

        if (generateStructures(numMonsters)) {
            return 1;
        }
        spawnPlayer();
        if (monTypeFlag) {
            if (spawnMonsterWithMonType(monType)) {
                delete[] rooms;
                delete[] upStairs;
                delete[] downStairs;

                return 1;
            }
        }
        else {
            if (spawnMonsters(numMonsters, player.pos.x, player.pos.y)) {
                delete[] rooms;
                delete[] upStairs;
                delete[] downStairs;

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
        generateDistances(player.pos.x, player.pos.y);
        printTunnelingDistances();
        printNonTunnelingDistances();       
    }

    initscr();
    if (autoFlag) {
        nodelay(stdscr, TRUE);
    }
    if (has_colors()) {
        start_color();
        supportsColor = true;

        init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    }
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    printLine(MESSAGE_LINE, "Welcome adventurer! Press any key to begin...");
    getch();

    if (playGame(numMonsters, autoFlag, godmodeFlag, supportsColor)) {
        return 1;
    }

    endwin();
    return 0;
}
