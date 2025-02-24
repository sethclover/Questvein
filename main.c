#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dungeon.h"

int main(int argc, char *argv[]) {
    int hbFlag = 0;
    int haFlag = 0;
    int saveFlag = 0;
    int loadFlag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-hb") == 0) {
            hbFlag = 1;
        }
        else if (strcmp(argv[i], "-ha") == 0) {
            haFlag = 1;
        }
        else if (strcmp(argv[i], "--save") == 0) {
            saveFlag = 1;
        }
        else if (strcmp(argv[i], "--load") == 0) {
            loadFlag = 1;
        }
    }
    
    srand(time(NULL));
    
    if (loadFlag) {
        char filename[256];
        strncpy(filename, argv[argc - 1], sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';

        loadDungeon(filename);
        generateDistances();
    }
    else {
        initDungeon();
        if (hbFlag) {
            printHardness();
        }

        fillDungeon();
    }

    if (haFlag) {
        printHardness();
    }

    if (saveFlag && !loadFlag) {
        char filename[256];
        strncpy(filename, argv[argc - 1], sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';

        saveDungeon(filename);
    }

    printDungeon();
    printTunnelingDistances();
    printNonTunnelingDistances();

    free(rooms);
    free(upStairs);
    free(downStairs);
    return 0;
}
