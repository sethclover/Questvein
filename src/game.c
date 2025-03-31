#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dungeon.h"
#include "errorHandle.h"
#include "fibonacciHeap.h"
#include "game.h"
#include "pathFinding.h"

typedef struct CommandInfo {
    const char *buttons;
    const char *desc;
} CommandInfo;

static const CommandInfo switches[] = {
    {"HOME/ 7 / y", "Move up-left"},
    {"UP / 8 / k", "Move up"},
    {"PAGE UP / 9 / u", "Move up-right"},
    {"RIGHT / 6 / l", "Move right"},
    {"PAGE DOWN / 3 / n", "Move down-right"},
    {"DOWN / 2 / j", "Move down"},
    {"END / 1 / b", "Move down-left"},
    {"LEFT / 4 / h", "Move left"},
    {"B2 / SPACE / . / 5", "Rest"},
    {">", "Go down stairs"},
    {"<", "Go up stairs"},
    {"Q", "Quit the game"},
    {"?", "Show help"}
};

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
    printLine(MESSAGE_LINE, "Press a key to continue...");
    refresh();
}

static char *personalityToString(int personality) {
    static char buf[8];
    buf[0] = '\0';
    if (personality & 1) strcat(buf, "I ");
    else strcat(buf, "* ");
    if (personality & 2) strcat(buf, "T ");
    else strcat(buf, "* ");
    if (personality & 4) strcat(buf, "U ");
    else strcat(buf, "* ");
    if (personality & 8) strcat(buf, "E");
    else strcat(buf, "*");
    buf[7] = '\0';
    return buf;
}

int monsterList(int monstersAlive) {
    Mon **monList = malloc(monstersAlive * sizeof(Mon*));
    if (!monList) {
        errorHandle("Error: Failed to allocate memory for monster list");
        return 1;
    }

    int count = 0;
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                monList[count++] = monsterAt[i][j];
            }
        }
    }

    int cols = 55;
    int rows = 24;
    int leftCol = (MAX_WIDTH - cols) / 2;
    if (leftCol < 0) leftCol = 0;
    int top = 0;

    clear();
    while (1) {
        mvhline(0, leftCol, '-', cols);
        mvaddch(0, leftCol + cols / 2, '+');
        mvhline(rows - 1, leftCol, '-', cols);
        mvaddch(rows - 1, leftCol + cols / 2, '+');
        for (int row = 0; row < rows; row++) {
            mvaddch(row, leftCol, '|');
            mvaddch(row, leftCol + cols - 1, '|');
        }

        char *title = "Monster List";
        int titleCol = leftCol + (cols - strlen(title)) / 2;
        mvprintw(1, titleCol, "%s", title);
        mvprintw(3, leftCol + 2, "Monsters alive: %d", count);

        move(4, leftCol + cols / 2);
        if (top > 0) {
            printw("^");
        }
        else {
            printw(" ");
        }

        int maxDisplay = rows - 7;
        for (int i = top; i < top + maxDisplay && i < count; i++) {
            Mon *mon = monList[i];
            int personality = 1 * mon->intelligent +
                              2 * mon->telepathic +
                              4 * mon->tunneling +
                              8 * mon->erratic;
            char *traits = personalityToString(personality);

            int x = mon->pos.x - player.x;
            int y = mon->pos.y - player.y;
            char* nsDir = (y >= 0) ? "South" : "North";
            char* ewDir = (x >= 0) ? "East" : "West";
            int nsDist = abs(y);
            int ewDist = abs(x);

            move(5 + (i - top), leftCol + 2);
            clrtoeol();
            if (nsDist == 0) {
                printw("Monster of type %c (%s) is %d %s",
                       personality < 10 ? '0' + personality : 'A' + (personality - 10),
                       traits, ewDist, ewDir);
            } 
            else if (ewDist == 0) {
                printw("Monster of type %c (%s) is %d %s",
                       personality < 10 ? '0' + personality : 'A' + (personality - 10),
                       traits, nsDist, nsDir);
            } 
            else {
                printw("Monster of type %c (%s) is %d %s and %d %s",
                       personality < 10 ? '0' + personality : 'A' + (personality - 10), 
                       traits, nsDist, nsDir, ewDist, ewDir);
            }
            mvaddch(5 + (i - top), leftCol + cols - 1, '|');
        }

        move(rows - 2, leftCol + cols / 2);
        if (top + maxDisplay < count) {
            printw("v");
        }
        else {
            printw(" ");    
        }

        refresh();

        int ch;
        do {
            ch = getch();
        } while (ch != KEY_UP && ch != KEY_DOWN && ch != 27);

        switch (ch) {
            case KEY_UP:
                if (top > 0) {
                    top--;
                }
                break;

            case KEY_DOWN:
                if (top + maxDisplay < count) {
                    top++;
                }
                break;

            case 27:
                clear();
                printDungeon();
                free(monList);
                return 0;
        }
    }
}

void commandList() {
    int count = sizeof(switches) / sizeof(CommandInfo);

    int cols = 55;
    int rows = 24;
    int leftCol = (MAX_WIDTH - cols) / 2;
    if (leftCol < 0) leftCol = 0;
    int top = 0;

    clear();
    while (1) {
        mvhline(0, leftCol, '-', cols);
        mvaddch(0, leftCol + cols / 2, '+');
        mvhline(rows - 1, leftCol, '-', cols);
        mvaddch(rows - 1, leftCol + cols / 2, '+');
        for (int row = 0; row < rows; row++) {
            mvaddch(row, leftCol, '|');
            mvaddch(row, leftCol + cols - 1, '|');
        }

        char *title = "Command List";
        int titleCol = leftCol + (cols - strlen(title)) / 2;
        mvprintw(1, titleCol, "%s", title);

        move(4, leftCol + cols / 2);
        if (top > 0) {
            printw("^");
        }
        else {
            printw(" ");
        }

        int maxDisplay = rows - 7;
        for (int i = top; i < top + maxDisplay && i < count; i++) {
            int row = 5 + (i - top);
            move(row, leftCol + 2);
            clrtoeol();
            mvprintw(row, leftCol + 8, "%18s - %s", switches[i].buttons, switches[i].desc);
            mvaddch(row, leftCol + cols - 1, '|');
        }

        move(rows - 2, leftCol + cols / 2);
        if (top + maxDisplay < count) {
            printw("v");
        }
        else {
            printw(" ");    
        }

        refresh();

        int ch;
        do {
            ch = getch();
        } while (ch != KEY_UP && ch != KEY_DOWN && ch != 27);

        switch (ch) {
            case KEY_UP:
                if (top > 0) {
                    top--;
                }
                break;

            case KEY_DOWN:
                if (top + maxDisplay < count) {
                    top++;
                }
                break;

            case 27:
                clear();
                printDungeon();
                return;
        }
    }
}

int checkCorridor(int x, int y, int visited[MAX_HEIGHT][MAX_WIDTH]) {
    if (visited[y][x] || dungeon[y][x].type != CORRIDOR) {
        return 0;
    }
    else if (player.x == x && player.y == y) {
        return 1;
    }
    visited[y][x] = 1;

    int directions[9][2] = {
        {-1, 1}, {0, 1}, {1, 1},
        {-1, 0}, {0, 0}, {1, 0},
        {-1, -1}, {0, -1}, {1, -1}};

    for (int i = 0; i < 9; i++) {
        int newX = x + directions[i][0];
        int newY = y + directions[i][1];
        if (checkCorridor(newX, newY, visited)) {
            return 1;
        }
    }

    return 0;
}

void cleanup(int numMonsters, FibHeap *heap) {
    destroyFibHeap(heap);
    freeAll(numMonsters);
}

int playGame(int numMonsters, int autoFlag, int godmodeFlag) {
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
                    cleanup(numMonsters, heap); 

                    return 1;
                }
            }
        }
    }
    nodes[player.y][player.x] = insert(heap, 100, player);
    if (!nodes[player.y][player.x]) {
        cleanup(numMonsters, heap);

        return 1;
    }

    while (1) {
        FibNode *node = extractMin(heap);
        if (!node) {
            cleanup(numMonsters, heap);

            return 1;
        }
        time = node->key;

        if (node->pos.x == player.x && node->pos.y == player.y) {
            printDungeon();
            int turnEnd = 0;
            while (!turnEnd) {
                int ch = 0;
                int xDir = 0;
                int yDir = 0;
                if (autoFlag) {
                    ch = getch();
                    if (ch == 'Q') {
                        printLine(MESSAGE_LINE, "Goodbye!");
                        napms(1000);
                        cleanup(numMonsters, heap);
                        free(node);

                        return 0;
                    }

                    xDir = rand() % 3 - 1;
                    yDir = rand() % 3 - 1;
                    napms(500);
                    turnEnd++;
                }
                else {
                    ch = getch();
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
                            if (dungeon[player.y][player.x].type == STAIR_DOWN) {
                                printLine(MESSAGE_LINE, "Going down stairs...");
                                napms(1000);
                                cleanup(numMonsters, heap);
                                free(node);

                                clear();
                                initDungeon();
                                if (generateStructures(numMonsters)) {
                                    return 1;
                                }
                                player.x = upStairs[0].x;
                                player.y = upStairs[0].y;
                                if (spawnMonsters(numMonsters, player.x, player.y)) {
                                    free(rooms);
                                    free(upStairs);
                                    free(downStairs);

                                    return 1;
                                }
                                if (playGame(numMonsters, autoFlag, godmodeFlag)) {
                                    cleanup(numMonsters, heap);

                                    return 1;
                                }

                                return 0;
                            }
                            else {
                                printLine(MESSAGE_LINE, "There are no stairs down here.");
                            }
                            break;

                        case '<':
                            if (dungeon[player.y][player.x].type == STAIR_UP) {
                                printLine(MESSAGE_LINE, "Going up stairs...");
                                napms(1000);
                                cleanup(numMonsters, heap);
                                free(node);

                                clear();
                                initDungeon();
                                if (generateStructures(numMonsters)) {
                                    return 1;
                                }
                                player.x = downStairs[0].x;
                                player.y = downStairs[0].y;
                                if (spawnMonsters(numMonsters, player.x, player.y)) {
                                    free(rooms);
                                    free(upStairs);
                                    free(downStairs);

                                    return 1;
                                }
                                if (playGame(numMonsters, autoFlag, godmodeFlag)) { // move to main
                                    cleanup(numMonsters, heap);

                                    return 1;
                                }

                                return 0;
                            }
                            else {
                                printLine(MESSAGE_LINE, "There are no stairs up here.");
                            }
                            break;

                        case 'c':
                            // "character info"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;
                        
                        case 'd':
                            // "drop item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'e':
                            // "display equipment"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;
                        
                        case 'f':
                            // "toggle fog of war"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'g':
                            // "teleport (goto)"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;
                        
                        case 'i':
                            // "display inventory"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'm':
                            monsterList(monstersAlive);
                            break;

                        case 's':
                            // "display the default (terrain) map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 't':
                            // "take off item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'w':
                            // "wear item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'x':
                            // "expunge item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'D':
                            // "display the non-tunneling distance map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;
                        
                        case 'E':
                            // "inspect equipped item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'H':
                            // "display the hardness map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'I':
                            // "inspect inventory item"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'L':
                            // "look at monster"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'Q':
                            printLine(MESSAGE_LINE, "Goodbye!");
                            napms(1000);
                            cleanup(numMonsters, heap);
                            free(node);
                            return 0;

                        case 'T':
                            // "display the tunneling distance map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case '?':
                            commandList();
                            break;

                        default:
                            printLine(MESSAGE_LINE,  "Invalid key... Press '?' for help.");
                            break;
                            
                    }
                }
                
                if (xDir == 0 && yDir == 0 && !(ch == KEY_B2 || ch == ' ' || ch == '.' || ch == '5')) {
                    turnEnd = 0;
                }
                else if (dungeon[player.y + yDir][player.x + xDir].hardness == 0) {
                    int oldX = player.x;
                    int oldY = player.y;
                    player.x += xDir;
                    player.y += yDir;
                    if (monsterAt[player.y][player.x]) {
                        FibHeap *tempHeap = createFibHeap();
                        if (!tempHeap) {
                            cleanup(numMonsters, heap);
                            free(node);

                            return 1;
                        }
                        FibNode *tempNode = extractMin(heap);
                        if (!tempNode) {
                            cleanup(numMonsters, heap);
                            free(node);

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
                            free(node);

                            return 1;
                        }

                        monstersAlive--;
                        if (monstersAlive <= 0) {
                            printDungeon();
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                            while (getch() != 'Q')
                                ;

                            cleanup(numMonsters, heap);
                            free(node);

                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", monstersAlive);
                        }

                    }
                    else {
                        nodes[oldY][oldX] = NULL;
                        nodes[player.y][player.x] = insert(heap, time + 100, player);
                        if (!nodes[player.y][player.x]) {
                            cleanup(numMonsters, heap);
                            free(node);

                            return 1;
                        }
                    }
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
                errorHandle("Error: Monster not found at %d, %d", node->pos.x, node->pos.y);
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
            int visited[MAX_HEIGHT][MAX_WIDTH] = {0};

            int sameCorridor = checkCorridor(x, y, visited);

            int hasLastSeen = (mon->lastSeen.x != -1 && mon->lastSeen.y != -1);

            int canSee = (isTelepathic || sameRoom || sameCorridor);
            if (canSee) {
                mon->lastSeen = player;
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
            else if (canSee || hasLastSeen) {
                if (canSee) {
                    generateDistances(player.x, player.y);
                }
                else {
                    generateDistances(mon->lastSeen.x, mon->lastSeen.y);
                }
                
                if (isIntelligent) {
                    int minDist = UNREACHABLE;
                    int possibleDir[9] = {0};
                    int numPossible = 0;
                    for (int i = 0; i < 9; i++) {
                        int newX = x + directions[i][0];
                        int newY = y + directions[i][1];
                        if ((isTunneling && dungeon[newY][newX].tunnelingDist < minDist) ||
                           (!isTunneling && dungeon[newY][newX].nonTunnelingDist < minDist)) {
                            numPossible = 1;
                            minDist = (isTunneling) ? dungeon[newY][newX].tunnelingDist : dungeon[newY][newX].nonTunnelingDist;
                            possibleDir[0] = i;
                        }
                        else if ((isTunneling && dungeon[newY][newX].tunnelingDist == minDist) ||
                                (!isTunneling && dungeon[newY][newX].nonTunnelingDist == minDist)) {
                            numPossible++;
                            possibleDir[numPossible - 1] = i;
                        }
                    }
                    int dir = possibleDir[rand() % numPossible];

                    newX = x + directions[dir][0];
                    newY = y + directions[dir][1];
                }
                else {
                    int targetX = mon->lastSeen.x;
                    int targetY = mon->lastSeen.y;
                    if (canSee) {
                        targetX = player.x;
                        targetY = player.y;
                    }

                    int xDist = abs(targetX - x);
                    int yDist = abs(targetY - y);

                    int xDir = 0;
                    int yDir = 0;
                    if (xDist > 0 || yDist > 0) {
                        if (xDist >= 2 * yDist) {
                            xDir = targetX > x ? 1 : -1;
                        }
                        else if (yDist >= 2 * xDist) {
                            yDir = targetY > y ? 1 : -1;
                        }
                        else {
                            if (x == targetX) {
                                yDir = targetY > y ? 1 : -1;
                            }
                            else if (y == targetY) {
                                xDir = targetX > x ? 1 : -1;
                            }
                            else {
                                xDir = targetX > x ? 1 : -1;
                                yDir = targetY > y ? 1 : -1;
                            }
                        }
                    }
    
                    newX = x + xDir;
                    newY = y + yDir;
                    if ((isTunneling && dungeon[newY][newX].tunnelingDist == UNREACHABLE) ||
                        (!isTunneling && dungeon[newY][newX].nonTunnelingDist == UNREACHABLE)) {
                        nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                        if (!nodes[y][x]) {
                            cleanup(numMonsters, heap);
                            free(node);

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
                        free(node);

                        return 1;
                    }
                }
            }
            else {
                if (monsterAt[newY][newX]) {
                    FibHeap *tempHeap = createFibHeap();
                    if (!tempHeap) {
                        cleanup(numMonsters, heap);
                        free(node);

                        return 1;
                    }
                    FibNode *tempNode = extractMin(heap);
                    if (!tempNode) {
                        errorHandle("Error: Failed to extract min from heap");
                        cleanup(numMonsters, heap);
                        free(node);

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
                        free(node);

                        return 1;
                    }

                    if (newX != x || newY != y) {
                        monstersAlive--;
                    }
                }
                else if (newX == player.x && newY == player.y) {
                    if (godmodeFlag) {
                        monsterAt[y][x] = NULL;
                        nodes[y][x] = NULL;

                        monstersAlive--;
                        if (monstersAlive <= 0) {
                            printDungeon();
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                            while (getch() != 'Q')
                                ;

                            cleanup(numMonsters, heap);
                            free(node);

                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", monstersAlive);
                        }
                    }
                    else {
                        mon->pos.x = newX;
                        mon->pos.y = newY;
                        monsterAt[newY][newX] = mon;
                        monsterAt[y][x] = NULL;
                        nodes[y][x] = NULL;
    
                        printDungeon();
                        printLine(MESSAGE_LINE, "");
                        printLine(STATUS_LINE1, "Player killed by monster, gg");
                        printLine(STATUS_LINE2, "You lose! (Press 'Q' to exit)");
                        char ch;
                        while ((ch = getch()) != 'Q')
                            ;
    
                        cleanup(numMonsters, heap);
                        free(node);

                        return 0;
                    }
                    
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
                        free(node);

                        return 1;
                    }
                }
            }
        }
        free(node);
    }
}