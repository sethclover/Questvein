#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "dungeon.h"
#include "saveLoad.h"
#include "pathFinding.h"
#include "fibonacciHeap.h"

int main(int argc, char *argv[]) {
    int hardnessBeforeFlag = 0;
    int hardnessAfterFlag = 0;
    int saveFlag = 0;
    int loadFlag = 0;

    char filename[256];
    int numMonsters = DEFAULT_MONSTER_COUNT;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-hb")) {
            hardnessBeforeFlag = 1;
        }
        else if (!strcmp(argv[i], "-ha")) {
            hardnessAfterFlag = 1;
        }
        else if (!strcmp(argv[i], "--save")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    fprintf(stderr, "Error: Argument after '--save' must be file name\n");
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    saveFlag = 1;
                }
            }
            else {
                fprintf(stderr, "Error: Argument '--save' requires a file name\n");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "--load")) {
            if (i < argc - 1) {
                if (argv[i + 1][0] == '-') {
                    fprintf(stderr, "Error: Argument after '--load' must be file name\n");
                    return 1;
                }
                else {
                    strncpy(filename, argv[i + 1], sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    loadFlag = 1;
                } 
            }
            else {
                fprintf(stderr, "Error: Argument '--load' requires a file name\n");
                return 1;
            }

            i++;
        }
        else if (!strcmp(argv[i], "--nummon")) {
            if (i < argc - 1) {
                char *next = argv[i + 1];

                int is_number = 1;
                for (int j = 0; next[j] != '\0'; j++) {
                    if (!isdigit(next[j])) {
                        is_number = 0;
                        break;
                    }
                }
                
                if (is_number && strlen(next) > 0) {
                    numMonsters = atoi(next);
                    if (numMonsters < 0) {
                        fprintf(stderr, "Error: Number of monsters must be positive\n");
                        return 1;
                    }
                } else {
                    fprintf(stderr, "Error: Argument after '--nummon' must be a positive integer\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: Argument '--nummon' requires a positive integer\n");
                return 1;
            }

            i++;
        }
        else {
            fprintf(stderr, "Error: Unrecognized argument '%s'\n", argv[i]);
            return 1;
        }
    }
    
    srand(time(NULL));

    if (loadFlag) {
        if (hardnessBeforeFlag) {
            fprintf(stderr, "Error: Argument '--load' and '-hb' cannot be used together\n");
            return 1;
        }
        loadDungeon(filename);

        if (populateDungeon(numMonsters)) {
            return 1;
        }
    }
    else {
        initDungeon();
        if (hardnessBeforeFlag) {
            printHardness();
        }

        if (fillDungeon(numMonsters)) {
            return 1;
        }
    }

    if (hardnessAfterFlag) {
        printHardness();
    }

    if (saveFlag && !loadFlag) {
        saveDungeon(filename);
    }

    printTunnelingDistances();
    printNonTunnelingDistances();

    //FibHeap *heap = createFibHeap();
    // int i = 10;
    // while (i) {
    printDungeon();
    //     //update positions
    //     usleep(50000);

    //     i--;
    // }

    cleanup(numMonsters);
    return 0;
}
