#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dungeon.h"
#include "errorHandle.h"
#include "fibonacciHeap.h"
#include "pathFinding.h"

#define MESSAGE_LINE 0
#define STATUS_LINE1 22
#define STATUS_LINE2 23

void printLine(int line, char *format, ...) {
    char buffer[MAX_WIDTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, MAX_WIDTH, format, args);
    va_end(args);

    move(line, 0);
    clrtoeol();
    printw("%s", buffer);
    refresh();
}

void printDungeon() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                int personality = 1 * monsterAt[i][j]->intelligent +
                                  2 * monsterAt[i][j]->telepathic +
                                  4 * monsterAt[i][j]->tunneling +
                                  8 * monsterAt[i][j]->erratic;
                mvaddch(i + 1, j, personality < 10 ? '0' + personality : 'A' + (personality - 10));
            }
            else if (player.x == j && player.y == i) {
                mvaddch(i + 1, j, '@');
            }
            else {
                mvaddch(i + 1, j, dungeon[i][j].type);
            }
        }
    }
    printLine(MESSAGE_LINE, "Press any key to continue...");
    refresh();
}

void cleanup(int numMonsters, FibHeap *heap) {
    destroyFibHeap(heap);
    freeAll(numMonsters);
}

int playGame(int numMonsters, int autoFlag) {
    int time = 0;
    int monstersAlive = numMonsters;
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};
    FibHeap *heap = createFibHeap();
    if (!heap) {
        endwin();
        printf("1\n");
        return 1;
    }
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                nodes[i][j] = insert(heap, 1000 / monsterAt[i][j]->speed, (Pos){j, i});
                if (!nodes[i][j]) {
                    cleanup(numMonsters, heap); 
                    printf("2\n");
                    return 1;
                }
            }
        }
    }
    nodes[player.y][player.x] = insert(heap, 100, player);
    if (!nodes[player.y][player.x]) {
        cleanup(numMonsters, heap);
        printf("3\n");
        return 1;
    }

    printLine(MESSAGE_LINE, "Welcome adventurer! Press any key to continue...");
    getch();
    while (1) {
        FibNode *node = extractMin(heap);
        if (!node) {
            cleanup(numMonsters, heap);
            printf("4\n");
            return 1;
        }
        time = node->key;

        if (node->pos.x == player.x && node->pos.y == player.y) {
            printDungeon();
            int turnEnd = 0;
            while (!turnEnd) {
                int xDir = 0;
                int yDir = 0;
                if (autoFlag) { //error i think
                    xDir = rand() % 3 - 1;
                    yDir = rand() % 3 - 1;
                }
                else {
                    int ch = getch();
                    switch (ch) {
                        case KEY_HOME:
                        case '7':
                        case 'y':
                            xDir = -1;
                            yDir = -1;
                            turnEnd++;
                            break;

                        case KEY_UP:
                        case '8':
                        case 'k':
                            xDir = 0;
                            yDir = -1;
                            turnEnd++;
                            break;
                        
                        case KEY_PPAGE:
                        case '9':
                        case 'u':
                            xDir = 1;
                            yDir = -1;
                            turnEnd++;
                            break;

                        case KEY_RIGHT:
                        case '6':
                        case 'l':
                            xDir = 1;
                            yDir = 0;
                            turnEnd++;
                            break;

                        case KEY_NPAGE:
                        case '3':
                        case 'n':
                            xDir = 1;
                            yDir = 1;
                            turnEnd++;
                            break;

                        case KEY_DOWN:
                        case '2':
                        case 'j':
                            xDir = 0;
                            yDir = 1;
                            turnEnd++;
                            break;

                        case KEY_END:
                        case '1':
                        case 'b':
                            xDir = -1;
                            yDir = 1;
                            turnEnd++;
                            break;

                        case KEY_LEFT:
                        case '4':
                        case 'h':
                            xDir = -1;
                            yDir = 0;
                            turnEnd++;
                            break;

                        case KEY_B2:
                        case ' ':
                        case '.':
                        case '5':
                            turnEnd++;
                            break;

                        case '>':
                            // "go down stairs"
                            break;

                        case '<':
                            // "go up stairs"
                            break;

                        case 'c':
                            // "character info"
                            break;
                        
                        case 'd':
                            // "drop item"
                            break;

                        case 'e':
                            // "display equipment"
                            break;
                        
                        case 'f':
                            // "toggle fog of war"
                            break;

                        case 'g':
                            // "teleport (goto)"
                            break;
                        
                        case 'i':
                            // "display inventory"
                            break;

                        case 'm':
                            // "display monster list"
                            break;

                        case 's':
                            // "display the default (terrain) map"
                            break;

                        case 't':
                            // "take off item"
                            break;

                        case 'w':
                            // "wear item"
                            break;

                        case 'x':
                            // "expunge item"
                            break;

                        case 'D':
                            // "display the non-tunneling distance map"
                            break;
                        
                        case 'E':
                            // "inspect equipped item"
                            break;

                        case 'H':
                            // "display the hardness map"
                            break;

                        case 'I':
                            // "inspect inventory item"
                            break;

                        case 'L':
                            // "look at monster"
                            break;

                        case 'Q':
                            // "quit"
                            printLine(MESSAGE_LINE, "Goodbye!");
                            usleep(1000000);
                            cleanup(numMonsters, heap);
                            printf("5\n");
                            return 0;

                        case 'T':
                            // "display the tunneling distance map"
                            break;

                        case '?':
                            // "displays key bindings"
                            break;

                        default:
                            // needs to go back to the top of switch
                    }
                }
                
                if (dungeon[player.y + yDir][player.x + xDir].hardness == 0) {
                    int oldX = player.x;
                    int oldY = player.y;
                    player.x += xDir;
                    player.y += yDir;
                    if (monsterAt[player.y][player.x]) {
                        FibHeap *tempHeap = createFibHeap();
                        if (!tempHeap) {
                            cleanup(numMonsters, heap);
                            printf("6\n");
                            return 1;
                        }
                        FibNode *tempNode = extractMin(heap);
                        if (!tempNode) {
                            cleanup(numMonsters, heap);
                            printf("7\n");
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
                            cleanup(numMonsters, heap);
                            printf("8\n");
                            return 1;
                        }
                        if (monstersAlive <= 0) {
                            printDungeon();
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press any key to exit)");
                            getch();

                            cleanup(numMonsters, heap);
                            printf("9");
                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", --monstersAlive);
                        }

                    }
                    else {
                        nodes[oldY][oldX] = NULL;
                        nodes[player.y][player.x] = insert(heap, time + 100, player);
                        if (!nodes[player.y][player.x]) {
                            cleanup(numMonsters, heap);
                            printf("10");
                            return 1;
                        }
                    }
                    generateDistances();
                }
                else {
                    printLine(MESSAGE_LINE, "There's a wall there, adventurer!");
                    turnEnd--;
                }
            }
        }
        else {
            Mon *mon = monsterAt[node->pos.y][node->pos.x];
            if (!mon) {
                errorHandle("Error: Monster not found");
                printf("11 eleven");
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
                                errorHandle("Error: Failed to allocate memory for possibleDir");
                                printf("12");
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
                                errorHandle("Error: Failed to reallocate memory for possibleDir");
                                printf("13");
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
                            cleanup(numMonsters, heap);
                            printf("14");
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
                        cleanup(numMonsters, heap);
                        printf("15");
                        return 1;
                    }
                }
                generateDistances();
            }
            else {
                if (monsterAt[newY][newX]) {
                    FibHeap *tempHeap = createFibHeap();
                    if (!tempHeap) {
                        cleanup(numMonsters, heap);
                        printf("16");
                        return 1;
                    }
                    FibNode *tempNode = extractMin(heap);
                    if (!tempNode) {
                        errorHandle("Error: Failed to extract min from heap");
                        cleanup(numMonsters, heap);
                        printf("18");
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
                        cleanup(numMonsters, heap);
                        printf("19");
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
                    printLine(STATUS_LINE1, "Player killed by monster, gg");
                    printLine(STATUS_LINE2, "You lose! (Press any key to exit)");
                    getch();

                    cleanup(numMonsters, heap);
                    printf("20");
                    return 0;
                }
                else {
                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[newY][newX] = mon;
                    monsterAt[y][x] = NULL;
                    nodes[y][x] = NULL;
                    nodes[newY][newX] = insert(heap, time + 1000 / mon->speed, mon->pos);
                    if (!nodes[newY][newX]) {
                        cleanup(numMonsters, heap);
                        printf("21");
                        return 1;
                    }
                }
            }
        }
        if (node) {
            free(node);
        }
    }

    printf("22");
    return 0;
}