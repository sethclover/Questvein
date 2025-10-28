#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ncurses.h>

#include "display.hpp"
#include "dungeon.hpp"
#include "game.hpp"
#include "globals.hpp"
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
    {"-pm", "--parse-monsters", "Print parsed monster file to stdout"},
    {"-po", "--parse-objects", "Print parsed object file to stdout"},
    {"-hb", "--printhardb", "After the game ends, print the hardness map before the dungeon structures are added"},
    {"-ha", "--printharda", "After the game ends, print the hardness map after dungeon structures are added"},
    {"-s", "--save", "Save the dungeon to a file (requires filename)"},
    {"-l", "--load", "Load a dungeon from a file (requires filename)"},
    {"-m", "--nummon", "Set the number of monsters (requires positive integer)"},
    {"-o", "--numobj", "Set the number of objects (requires positive integer)"},
    {"-a", "--auto", "Run the game in automatic (random) movement mode"},
    {"-g", "--godmode", "Enable god mode (invincible player)"}
};

static const int numSwitches = sizeof(switches) / sizeof(SwitchInfo);

WINDOW *mainWin;

bool autoFlag;
bool godmodeFlag;

bool supportsColor;

int numMonsters;
int numObjects;

int main(int argc, char *argv[]) {
    srand(time(nullptr));

    bool printhardbFlag = false;
    bool printhardaFlag = false;
    bool saveFlag = false;
    bool loadFlag = false;

    autoFlag = false;
    godmodeFlag = false;
    supportsColor = false;

    char filename[256];
    numMonsters = rand() % 9 + 7;
    numObjects = rand() % 3 + 10;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Usage: Dungeon Crawler [options]\n"
                    "\nAvailable options:\n");
            for (int i = 0; i < numSwitches; i++) {
                printf("  %-4s, %-17s:  %s\n", switches[i].shortOp, switches[i].longOp, switches[i].desc);
            }
            printf("\nExamples:\n"
                    "    ./dungeon --save test1.questv -a\n"
                    "    ./dungeon -l test1.questv --nummon 10\n\n");
            return 0;
        }
        else if (!strcmp(argv[i], "-pm") || !strcmp(argv[i], "--parse-monsters")) {
            if (parse("../data/monster_desc.txt")) {
                std::cout << "Attempting to parse example monster description file instead." << std::endl;
                if (parse("../data/monster_desc.example.txt")) {
                    return 1;
                }
            }
            printParsedMonsters();
            return 0;
        }
        else if (!strcmp(argv[i], "-po") || !strcmp(argv[i], "--parse-objects")) {
            if (parse("../data/object_desc.txt")) {
                std::cout << "Attempting to parse example object description file instead." << std::endl;
                if (parse("../data/object_desc.example.txt")) {
                    return 1;
                }
            }
            printParsedObjects();
            return 0;
        }
        else if (!strcmp(argv[i], "-hb") || !strcmp(argv[i], "--printhardb")) {
            printhardbFlag = true;
        }
        else if (!strcmp(argv[i], "-ha") || !strcmp(argv[i], "--printharda")) {
            printhardaFlag = true;
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
        else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--nummon")) {
            if (i < argc - 1) {
                char *next = argv[i + 1];

                bool isNum = true;
                for (int j = 0; next[j] != '\0'; j++) {
                    if (!isdigit(next[j])) {
                        isNum = false;
                        break;
                    }
                }
                
                if (isNum && strlen(next) > 0) {
                    numMonsters = atoi(next);
                    if (numMonsters < 0) {
                        std::cout << "Error: Number of monsters must be positive" << std::endl;
                        return 1;
                    }
                }
                else {
                    std::cout << "Error: Argument after '--nummon/-m' must be a positive integer" << std::endl;
                    return 1;
                }
            }
            else {
                std::cout << "Error: Argument '--nummon/-m' requires a positive integer" << std::endl;
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--numobj")) {
            if (i < argc - 1) {
                char *next = argv[i + 1];

                bool isNum = true;
                for (int j = 0; next[j] != '\0'; j++) {
                    if (!isdigit(next[j])) {
                        isNum = false;
                        break;
                    }
                }

                if (isNum && strlen(next) > 0) {
                    numObjects = atoi(next);
                    if (numObjects < 0) {
                        std::cout << "Error: Number of objects must be positive" << std::endl;
                        return 1;
                    }
                }
                else {
                    std::cout << "Error: Argument after '--numobj/-o' must be a positive integer" << std::endl;
                    return 1;
                }
            }
            else {
                std::cout << "Error: Argument '--numobj/-o' requires a positive integer" << std::endl;
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
    
    if (parse("../data/object_desc.txt")) {
        std::cout << "Attempting to parse example object description file instead." << std::endl;
        if (parse("../data/object_desc.example.txt")) {
            return 1;
        }
    }
    if (parse("../data/monster_desc.txt")) {
        std::cout << "Attempting to parse example monster description file instead." << std::endl;
        if (parse("../data/monster_desc.example.txt")) {
            return 1;
        }
    }
    
    if (loadFlag) {
        if (printhardbFlag) {
            std::cout << "Error: Argument '--printhardb/-hb' cannot be used with '--load/-l'" << std::endl;
            return 1;
        }
        loadDungeon(filename);
        spawnMonsters(numMonsters, player.getPos().x, player.getPos().y);
        spawnObjects(numObjects);
    }
    else {
        initDungeon();
        if (printhardbFlag) {
            printHardness();
        }
        generateStructures();
        spawnPlayer();
        spawnMonsters(numMonsters, player.getPos().x, player.getPos().y);
        spawnObjects(numObjects);
    }

    if (printhardaFlag) {
        printHardness();
    }

    if (saveFlag && !loadFlag) {
        saveDungeon(filename);
    }

    initscr();
    if (autoFlag) {
        nodelay(stdscr, TRUE);
    }
    if (has_colors()) {
        start_color();
        supportsColor = true;
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
        init_pair(5, COLOR_BLUE, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_CYAN, COLOR_BLACK);
        init_pair(8, COLOR_WHITE, COLOR_BLACK);
    }
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    printLine(MESSAGE_LINE, "Welcome adventurer! Press any key to begin...");
    getch();

    while (playGame())
        ;

    endwin();
    return 0;
}
