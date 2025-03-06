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
    int monTypeFlag = 0;

    srand(time(NULL));

    char filename[256];
    char monType = '0';
    int numMonsters = rand() % 14 + 7;

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
        else if (!strcmp(argv[i], "--montype")) {
            if (i < argc - 1) {
                if (strlen(argv[i + 1]) == 1) {
                    monType = argv[i + 1][0];
                    numMonsters = 1;
                    monTypeFlag = 1;
                }
                else {
                    fprintf(stderr, "Error: Argument after '--montype' must be a single character\n");
                    return 1;
                }
            }
            else {
                fprintf(stderr, "Error: Argument '--montype' requires a single character\n");
                return 1;
            }

            i++;
        }
        else {
            fprintf(stderr, "Error: Unrecognized argument '%s'\n", argv[i]);
            return 1;
        }
    }
    
    if (loadFlag) {
        if (hardnessBeforeFlag) {
            fprintf(stderr, "Error: Argument '--load' and '-hb' cannot be used together\n");
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
        if (hardnessBeforeFlag) {
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

    if (hardnessAfterFlag) {
        printHardness();
    }

    if (saveFlag && !loadFlag) {
        saveDungeon(filename);
    }

    printTunnelingDistances();
    printNonTunnelingDistances();

    int time = 0;
    int monstersAlive = numMonsters;
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};
    FibHeap *heap = createFibHeap();
    if (!heap) {
        return 1;
    }
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                nodes[i][j] = insert(heap, 1000 / monsterAt[i][j]->speed, (Pos){j, i});
                if (!nodes[i][j]) {
                    destroyFibHeap(heap);
                    cleanup(numMonsters);
                    return 1;
                }
            }
        }
    }
    nodes[player.y][player.x] = insert(heap, 100, player);
    if (!nodes[player.y][player.x]) {
        destroyFibHeap(heap);
        cleanup(numMonsters);
        return 1;
    }

    printDungeon();
    while (1) {
        FibNode *node = extractMin(heap);
        if (!node) {
            destroyFibHeap(heap);
            cleanup(numMonsters);
            return 1;
        }
        time = node->key;

        if (node->pos.x == player.x && node->pos.y == player.y) {
            for (int i = 0; i < ATTEMPTS; i++) {
                int xDir = rand() % 3 - 1;
                int yDir = rand() % 3 - 1;
                
                if (dungeon[player.y + yDir][player.x + xDir].hardness == 0) {
                    int oldX = player.x;
                    int oldY = player.y;
                    player.x += xDir;
                    player.y += yDir;
                    if (monsterAt[player.y][player.x]) {
                        FibHeap *tempHeap = createFibHeap();
                        if (!tempHeap) {
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 1;
                        }
                        FibNode *tempNode = extractMin(heap);
                        if (!tempNode) {
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 1;
                        }
                        while (tempNode->pos.x != monsterAt[player.y][player.x]->pos.x ||
                               tempNode->pos.y != monsterAt[player.y][player.x]->pos.y) {
                            insert(tempHeap, tempNode->key, tempNode->pos);
                            free(tempNode);
                            tempNode = extractMin(heap);
                        }
                        if (tempNode) {
                            free(tempNode);
                        }
                        while (tempHeap->min) {
                            tempNode = extractMin(tempHeap);
                            insert(heap, tempNode->key, tempNode->pos);
                            free(tempNode);
                        }
                        destroyFibHeap(tempHeap);

                        monsterAt[player.y][player.x] = NULL;

                        nodes[oldY][oldX] = NULL;
                        nodes[player.y][player.x] = insert(heap, time + 100, player);
                        if (!nodes[player.y][player.x]) {
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 1;
                        }
                        printf("Player killed monster, Monsters alive: %d\n", --monstersAlive);
                        if (monstersAlive <= 0) {
                            printDungeon();
                            printf("Player killed all monsters\n");
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 0;
                        }
                    }
                    else {
                        nodes[oldY][oldX] = NULL;
                        nodes[player.y][player.x] = insert(heap, time + 100, player);
                        if (!nodes[player.y][player.x]) {
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 1;
                        }
                    }
                    generateDistances();
                    break;
                }
            }

            usleep(200000);
            printDungeon();
        }
        else {
            Mon *mon = monsterAt[node->pos.y][node->pos.x];
            if (!mon) {
                fprintf(stderr, "Error: Monster not found at %d, %d\n", node->pos.x, node->pos.y);
                destroyFibHeap(heap);
                cleanup(numMonsters);
                return 1;
            }

            int isIntelligent = mon->intelligent;
            int isTunneling = mon->tunneling;
            int isTelepathic = mon->telepathic;
            int isErratic = mon->erratic;

            int x = mon->pos.x;
            int y = mon->pos.y;

            int directions[9][2] = {
                {-1, 1}, {0, 1}, {1, 1},
                {-1, 0}, {0, 0}, {1, 0},
                {-1, -1}, {0, -1}, {1, -1}};

            int sameRoom = 0;
            for (int i = 0; i < roomCount; i++) {
                if (x >= rooms[i].x && x <= rooms[i].x + rooms[i].width - 1 &&
                    y >= rooms[i].y && y <= rooms[i].y + rooms[i].height - 1 &&
                    player.x >= rooms[i].x && player.x <= rooms[i].x + rooms[i].width - 1 &&
                    player.y >= rooms[i].y && player.y <= rooms[i].y + rooms[i].height - 1) {
                    sameRoom = 1;
                    break;
                }
            }

            int newX = x;
            int newY = y;
            if (isErratic && rand() % 2) {
                for (int i = 0; i < ATTEMPTS; i++) {
                    int dir = rand() % 9;
                    newX = x + directions[dir][0];
                    newY = y + directions[dir][1];
                    
                    if ((isTunneling && dungeon[newY][newX].tunnelingDist != UNREACHABLE) ||
                       (!isTunneling && dungeon[newY][newX].nonTunnelingDist != UNREACHABLE)) {
                        break;
                    }
                }
            }
            else if (isTelepathic || sameRoom) {
                if (isIntelligent) {
                    int minDist = UNREACHABLE;
                    int *possibleDir = NULL;
                    int numPossible = 0;
                    for (int i = 0; i < 9; i++) {
                        int newX = x + directions[i][0];
                        int newY = y + directions[i][1];
                        if ((isTunneling && dungeon[newY][newX].tunnelingDist < minDist) ||
                           (!isTunneling && dungeon[newY][newX].nonTunnelingDist < minDist)) {
                            numPossible = 1;
                            possibleDir = malloc(sizeof(int));
                            if (!possibleDir) {
                                fprintf(stderr, "Error: Failed to allocate memory for possibleDir\n");
                                return 1;
                            }
                            minDist = (isTunneling) ? dungeon[newY][newX].tunnelingDist : dungeon[newY][newX].nonTunnelingDist;
                            possibleDir[0] = i;
                        }
                        else if ((isTunneling && dungeon[newY][newX].tunnelingDist == minDist) ||
                                (!isTunneling && dungeon[newY][newX].nonTunnelingDist == minDist)) {
                            numPossible++;
                            possibleDir = realloc(possibleDir, numPossible * sizeof(int));
                            if (!possibleDir) {
                                fprintf(stderr, "Error: Failed to reallocate memory for possibleDir\n");
                                return 1;
                            }
                            possibleDir[numPossible - 1] = i;
                        }
                    }
                    int dir = possibleDir[rand() % numPossible];
                    free(possibleDir);

                    newX = x + directions[dir][0];
                    newY = y + directions[dir][1];
                }
                else {
                    int xDist = abs(player.x - x);
                    int yDist = abs(player.y - y);

                    int xDir = 0;
                    int yDir = 0;
                    if (xDist >= 2 * yDist) {
                        xDir = player.x > x ? 1 : -1;
                    }
                    else if (yDist >= 2 * xDist) {
                        yDir = player.y > y ? 1 : -1;
                    }
                    else {
                        if (x == player.x) {
                            yDir = player.y > y ? 1 : -1;
                        }
                        else if (y == player.y) {
                            xDir = player.x > x ? 1 : -1;
                        }
                        else {
                            xDir = player.x > x ? 1 : -1;
                            yDir = player.y > y ? 1 : -1;
                        }
                    }
                    newX = x + xDir;
                    newY = y + yDir;
                    if ((isTunneling && dungeon[newY][newX].tunnelingDist == UNREACHABLE) ||
                        (!isTunneling && dungeon[newY][newX].nonTunnelingDist == UNREACHABLE)) {
                        nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                        if (!nodes[y][x]) {
                            free(node);
                            destroyFibHeap(heap);
                            cleanup(numMonsters);
                            return 1;
                        }
                        free(node);
                        continue;
                    }
                }
            } 
            else {
                nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                if (node) {
                    free(node);
                }
                continue;
            }   

            if (dungeon[newY][newX].type == ROCK) {
                if (dungeon[newY][newX].hardness > 85) {
                    dungeon[newY][newX].hardness -= 85;
                    nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                }
                else {
                    dungeon[newY][newX].hardness = 0;
                    dungeon[newY][newX].type = CORRIDOR;

                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[y][x] = NULL;
                    monsterAt[newY][newX] = mon;
                    nodes[y][x] = NULL;
                    nodes[newY][newX] = insert(heap, time + 1000 / mon->speed, mon->pos);
                    if (!nodes[newY][newX]) {
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                }
                generateDistances();
            }
            else {
                if (monsterAt[newY][newX]) {
                    FibHeap *tempHeap = createFibHeap();
                    if (!tempHeap) {
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                    FibNode *tempNode = extractMin(heap);
                    if (!tempNode) {
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                    if (!tempNode) {
                        fprintf(stderr, "Error: Failed to create heap\n");
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                    while (tempNode->pos.x != monsterAt[newY][newX]->pos.x ||
                            tempNode->pos.y != monsterAt[newY][newX]->pos.y) {
                        insert(tempHeap, tempNode->key, tempNode->pos);
                        free(tempNode); 
                        tempNode = extractMin(heap);
                        if (!tempNode) {
                            break;
                        }
                    }
                    if (tempNode) {
                        free(tempNode);
                    }
                    while (tempHeap->min) {
                        tempNode = extractMin(tempHeap);
                        insert(heap, tempNode->key, tempNode->pos);
                        free(tempNode);
                    }
                    destroyFibHeap(tempHeap);

                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[y][x] = NULL;
                    monsterAt[newY][newX] = mon;
                    nodes[y][x] = NULL;
                    nodes[newY][newX] = insert(heap, time + 1000 / mon->speed, mon->pos);
                    if (!nodes[newY][newX]) {
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                }
                else if (newX == player.x && newY == player.y) {
                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[newY][newX] = mon;
                    monsterAt[y][x] = NULL;
                    nodes[y][x] = NULL;
                    printDungeon();
                    printf("Player killed by monster, gg\n");
                    destroyFibHeap(heap);
                    cleanup(numMonsters);
                    return 1;
                }
                else {
                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[newY][newX] = mon;
                    monsterAt[y][x] = NULL;
                    nodes[y][x] = NULL;
                    nodes[newY][newX] = insert(heap, time + 1000 / mon->speed, mon->pos);
                    if (!nodes[newY][newX]) {
                        destroyFibHeap(heap);
                        cleanup(numMonsters);
                        return 1;
                    }
                }
            }
        }
        if (node) {
            free(node);
        }
    }

    return 0;
}
