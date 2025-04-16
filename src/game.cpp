#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>

#include "display.hpp"
#include "dungeon.hpp"
#include "fibonacciHeap.hpp"
#include "game.hpp"
#include "pathFinding.hpp"

static bool fogOfWarToggle = true;

int checkCorridor(int x, int y, int visited[MAX_HEIGHT][MAX_WIDTH]) {
    if (x < 0 || x >= MAX_WIDTH || y < 0 || y >= MAX_HEIGHT) {
        return 0;
    }
    else if ((visited[y][x] || dungeon[y][x].type != CORRIDOR)) {
        return 0;
    }
    else if (player.pos.x == x && player.pos.y == y) {
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

void updateAroundPlayer() {
    for (int i = player.pos.y - 2; i <= player.pos.y + 2; i++) {
        for (int j = player.pos.x - 2; j <= player.pos.x + 2; j++) {
            if (((i == player.pos.y - 2 || i == player.pos.y + 2) && (j == player.pos.x - 2 || j == player.pos.x + 2)) ||
                (i < 0 || i >= MAX_HEIGHT || j < 0 || j >= MAX_WIDTH)) {
                continue;
                }
            dungeon[i][j].visible = dungeon[i][j].type;
        }
    }
}

void cleanup(int numMonsters, FibHeap *heap) {
    destroyFibHeap(heap);
    freeAll(numMonsters);
}

int playGame(int numMonsters, bool autoFlag, bool godmodeFlag, bool supportsColor) {
    int time = 0;
    int monstersAlive = numMonsters;

    FibHeap *heap = new FibHeap();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                insert(heap, 1000 / monsterAt[i][j]->getSpeed(), (Pos){j, i});
            }
            
            dungeon[i][j].visible = FOG;
        }
    }
    insert(heap, 100, player.pos);

    if (autoFlag) {
        fogOfWarToggle = false;
    }
     
    while (1) {
        FibNode *node = extractMin(heap);
        time = node->key;

        if (node->pos.x == player.pos.x && node->pos.y == player.pos.y) {
            updateAroundPlayer();
            printDungeon(supportsColor, fogOfWarToggle);
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
                            if (dungeon[player.pos.y][player.pos.x].type == STAIR_DOWN) {
                                printLine(MESSAGE_LINE, "Going down stairs...");
                                napms(1000);
                                cleanup(numMonsters, heap);
                                delete node;

                                clear();
                                initDungeon();
                                generateStructures();
                                player.pos.x = upStairs[0].x;
                                player.pos.y = upStairs[0].y;
                                spawnMonsters(numMonsters, player.pos.x, player.pos.y);
                                
                                return 1;
                            }
                            else {
                                printLine(MESSAGE_LINE, "There are no stairs down here.");
                            }
                            break;

                        case '<':
                            if (dungeon[player.pos.y][player.pos.x].type == STAIR_UP) {
                                printLine(MESSAGE_LINE, "Going up stairs...");
                                napms(1000);
                                cleanup(numMonsters, heap);
                                delete node;

                                clear();
                                initDungeon();
                                generateStructures();
                                player.pos.x = downStairs[0].x;
                                player.pos.y = downStairs[0].y;
                                spawnMonsters(numMonsters, player.pos.x, player.pos.y);

                                return 1;
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
                            fogOfWarToggle = !fogOfWarToggle;
                            printDungeon(supportsColor, fogOfWarToggle);
                            {
                                const char* fogStatus = fogOfWarToggle ? "on" : "off";
                                printLine(STATUS_LINE2, "Fog of war toggled %s", fogStatus);
                            }
                                
                            break;

                        case 'g':
                            {
                                int replaceFogOfWar = fogOfWarToggle;
                                fogOfWarToggle = false;
                                printDungeon(supportsColor, fogOfWarToggle);

                                int drop = 0;
                                int x = player.pos.x;
                                int y = player.pos.y;
                                while (!drop) {
                                    int oldX = x;
                                    int oldY = y;
                                    mvaddch(y + 1, x, '!');
                                    refresh();
                            
                                    int ch;
                                    ch = getch();
                                    switch (ch) {
                                        case 'r':
                                            x = rand() % (MAX_WIDTH - 2) + 1;
                                            y = rand() % (MAX_HEIGHT - 2) + 1;
                                            drop = 1;
                                            break;
                            
                                        case 'g':
                                            drop = 1;
                                            break;
                            
                                        case KEY_HOME:
                                        case '7':
                                        case 'y':
                                            x -= 1;
                                            if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y -= 1;
                                            if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_UP:
                                        case '8':
                                        case 'k':
                                            y -= 1;
                                            if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                                        
                                        case KEY_PPAGE:
                                        case '9':
                                        case 'u':
                                            x += 1;
                                            if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            } 
                                            y -= 1;
                                            if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_RIGHT:
                                        case '6':
                                        case 'l':
                                            x += 1;
                                            if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_NPAGE:
                                        case '3':
                                        case 'n':
                                            x += 1;
                                            if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y += 1;
                                            if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_DOWN:
                                        case '2':
                                        case 'j':
                                            y += 1;
                                            if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_END:
                                        case '1':
                                        case 'b':
                                            x -= 1;
                                            if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y += 1;
                                            if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        case KEY_LEFT:
                                        case '4':
                                        case 'h':
                                            x -= 1;
                                            if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            break;
                            
                                        default:
                                            printLine(MESSAGE_LINE, "Use movement keys to move and 'g' to finalize, or 'r' to be placed randomly.");
                                            break;
                                    }
                                    if (monsterAt[oldY][oldX]) {
                                        mvaddch(oldY + 1, oldX, monsterAt[oldY][oldX]->getSymbol());
                                    }
                                    else {
                                        mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].type);
                                    }
                                }
                            
                                if (dungeon[y][x].type == ROCK) {
                                    dungeon[y][x].hardness = 0;
                                    dungeon[y][x].type = CORRIDOR;
                                }

                                player.pos.x = x;
                                player.pos.y = y;
                                mvaddch(y + 1, x, '@');

                                if (monsterAt[player.pos.y][player.pos.x]) {
                                    FibHeap *tempHeap = new FibHeap();
                                    FibNode *tempNode = extractMin(heap);
                                    while (tempNode->pos.x != monsterAt[player.pos.y][player.pos.x]->pos.x ||
                                            tempNode->pos.y != monsterAt[player.pos.y][player.pos.x]->pos.y) {
                                        insert(tempHeap, tempNode->key, tempNode->pos);
                                        delete tempNode;
                                        tempNode = extractMin(heap);
                                    }
                                    if (tempNode) {
                                        delete tempNode;
                                    }
                                    while (tempHeap->min) {
                                        tempNode = extractMin(tempHeap);
                                        insert(heap, tempNode->key, tempNode->pos);
                                        delete tempNode;
                                    }
                                    destroyFibHeap(tempHeap);

                                    monsterAt[player.pos.y][player.pos.x] = NULL;
            
                                    monstersAlive--;
                                    if (monstersAlive <= 0) {
                                        printDungeon(supportsColor, fogOfWarToggle);
                                        printLine(STATUS_LINE1, "Player killed all monsters!\n");
                                        printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                                        while (getch() != 'Q')
                                            ;
            
                                        return 0;
                                    }
                                    else {
                                        printLine(STATUS_LINE1, "Player stomped monster, Monsters alive: %d\n", monstersAlive);
                                    }
                                }
                                fogOfWarToggle = replaceFogOfWar;
                                updateAroundPlayer();
                                printDungeon(supportsColor, fogOfWarToggle);
                            }
                            break;
                        
                        case 'i':
                            // "display inventory"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'm':
                            monsterList(monstersAlive, supportsColor, fogOfWarToggle);
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
                            return 0;

                        case 'T':
                            // "display the tunneling distance map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case '?':
                            commandList(supportsColor, fogOfWarToggle);
                            break;

                        default:
                            printLine(MESSAGE_LINE, "Invalid key... Press '?' for help.");
                            break;
                            
                    }
                }
                
                if (xDir == 0 && yDir == 0 && !(ch == KEY_B2 || ch == ' ' || ch == '.' || ch == '5')) {
                    turnEnd = 0;
                }
                else if (dungeon[player.pos.y + yDir][player.pos.x + xDir].hardness == 0) {
                    player.pos.x += xDir;
                    player.pos.y += yDir;
                    if (monsterAt[player.pos.y][player.pos.x]) {
                        FibHeap *tempHeap = new FibHeap();
                        FibNode *tempNode = extractMin(heap);
                        while (tempNode->pos.x != monsterAt[player.pos.y][player.pos.x]->pos.x ||
                                tempNode->pos.y != monsterAt[player.pos.y][player.pos.x]->pos.y) {
                            insert(tempHeap, tempNode->key, tempNode->pos);
                            delete tempNode;
                            tempNode = extractMin(heap);
                        }
                        if (tempNode) {
                            delete tempNode;
                        }
                        while (tempHeap->min) {
                            tempNode = extractMin(tempHeap);
                            insert(heap, tempNode->key, tempNode->pos);
                            delete tempNode;
                        }
                        destroyFibHeap(tempHeap);

                        monsterAt[player.pos.y][player.pos.x] = NULL;

                        insert(heap, time + 100, player.pos);

                        monstersAlive--;
                        if (monstersAlive <= 0) {
                            printDungeon(supportsColor, fogOfWarToggle);
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                            while (getch() != 'Q')
                                ;

                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", monstersAlive);
                        }

                    }
                    else {
                        insert(heap, time + 100, player.pos);
                    }
                }
                else {
                    printLine(MESSAGE_LINE, "There's a wall there, adventurer!");
                    turnEnd--;
                }
            }
        }
        else {
            Monster *mon = monsterAt[node->pos.y][node->pos.x];

            int x = mon->getPos().x;
            int y = mon->getPos().y;

            int directions[9][2] = {
                {-1, 1}, {0, 1}, {1, 1},
                {-1, 0}, {0, 0}, {1, 0},
                {-1, -1}, {0, -1}, {1, -1}};
            int sameRoom = 0;
            for (int i = 0; i < roomCount; i++) {
                if (x >= rooms[i].x && x <= rooms[i].x + rooms[i].width - 1 &&
                    y >= rooms[i].y && y <= rooms[i].y + rooms[i].height - 1 &&
                    player.pos.x >= rooms[i].x && player.pos.x <= rooms[i].x + rooms[i].width - 1 &&
                    player.pos.y >= rooms[i].y && player.pos.y <= rooms[i].y + rooms[i].height - 1) {
                    sameRoom = 1;
                    break;
                }
            }
            int visited[MAX_HEIGHT][MAX_WIDTH] = {0};

            int sameCorridor = checkCorridor(x, y, visited);

            int hasLastSeen = (mon->getLastSeen().x != -1 && mon->getLastSeen().y != -1);

            int canSee = (mon->isTelepathic() || sameRoom || sameCorridor);
            if (canSee) {
                mon->setLastSeen(player.pos);
            }

            int newX = x;
            int newY = y;
            if (mon->isErratic() && rand() % 2) {
                for (int i = 0; i < ATTEMPTS; i++) {
                    int dir = rand() % 9;
                    newX = x + directions[dir][0];
                    newY = y + directions[dir][1];
                    
                    if ((mon->isTunneling() && dungeon[newY][newX].tunnelingDist != UNREACHABLE) ||
                       (!mon->isTunneling() && dungeon[newY][newX].nonTunnelingDist != UNREACHABLE)) {
                        break;
                    }
                }
            }
            else if (canSee || hasLastSeen) {
                if (canSee) {
                    generateDistances(player.pos.x, player.pos.y);
                }
                else {
                    generateDistances(mon->getLastSeen().x, mon->getLastSeen().y);
                }
                
                if (mon->isIntelligent()) {
                    int minDist = UNREACHABLE;
                    int possibleDir[9] = {0};
                    int numPossible = 0;
                    for (int i = 0; i < 9; i++) {
                        int newX = x + directions[i][0];
                        int newY = y + directions[i][1];
                        if ((mon->isTunneling() && dungeon[newY][newX].tunnelingDist < minDist) ||
                           (!mon->isTunneling() && dungeon[newY][newX].nonTunnelingDist < minDist)) {
                            numPossible = 1;
                            minDist = (mon->isTunneling()) ? dungeon[newY][newX].tunnelingDist : dungeon[newY][newX].nonTunnelingDist;
                            possibleDir[0] = i;
                        }
                        else if ((mon->isTunneling() && dungeon[newY][newX].tunnelingDist == minDist) ||
                                (!mon->isTunneling() && dungeon[newY][newX].nonTunnelingDist == minDist)) {
                            numPossible++;
                            possibleDir[numPossible - 1] = i;
                        }
                    }
                    int dir = possibleDir[rand() % numPossible];

                    newX = x + directions[dir][0];
                    newY = y + directions[dir][1];
                }
                else {
                    int targetX = mon->getLastSeen().x;
                    int targetY = mon->getLastSeen().y;
                    if (canSee) {
                        targetX = player.pos.x;
                        targetY = player.pos.y;
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
                    if ((mon->isTunneling() && dungeon[newY][newX].tunnelingDist == UNREACHABLE) ||
                        (!mon->isTunneling() && dungeon[newY][newX].nonTunnelingDist == UNREACHABLE)) {
                        insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                        continue;
                    }
                }
            } 
            else {
                insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                if (node) {
                    delete node;
                }
                continue;
            }   

            if (dungeon[newY][newX].type == ROCK) {
                if (dungeon[newY][newX].hardness > 85) {
                    dungeon[newY][newX].hardness -= 85;
                    insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                }
                else {
                    dungeon[newY][newX].hardness = 0;
                    dungeon[newY][newX].type = CORRIDOR;

                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[y][x] = NULL;
                    monsterAt[newY][newX] = mon;
                    insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                }
            }
            else {
                if (monsterAt[newY][newX]) {
                    FibHeap *tempHeap = new FibHeap();
                    FibNode *tempNode = extractMin(heap);
                    while (tempNode->pos.x != monsterAt[newY][newX]->pos.x ||
                            tempNode->pos.y != monsterAt[newY][newX]->pos.y) {
                        insert(tempHeap, tempNode->key, tempNode->pos);
                        delete tempNode;
                        tempNode = extractMin(heap);
                        if (!tempNode) {
                            break;
                        }
                    }
                    if (tempNode) {
                        delete tempNode;
                    }
                    while (tempHeap->min) {
                        tempNode = extractMin(tempHeap);
                        insert(heap, tempNode->key, tempNode->pos);
                        delete tempNode;
                    }
                    destroyFibHeap(tempHeap);

                    mon->pos.x = newX;
                    mon->pos.y = newY;
                    monsterAt[y][x] = NULL;
                    monsterAt[newY][newX] = mon;
                    insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                    
                    if (newX != x || newY != y) {
                        monstersAlive--;
                    }
                }
                else if (newX == player.pos.x && newY == player.pos.y) {
                    if (godmodeFlag) {
                        monsterAt[y][x] = NULL;

                        monstersAlive--;
                        if (monstersAlive <= 0) {
                            updateAroundPlayer();
                            printDungeon(supportsColor, fogOfWarToggle);
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                            while (getch() != 'Q')
                                ;

                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", monstersAlive);
                        }
                    }
                    else {
                        mon->setPos((Pos){newX, newY});
                        monsterAt[newY][newX] = mon;
                        monsterAt[y][x] = NULL;
    
                        updateAroundPlayer();
                        printDungeon(supportsColor, fogOfWarToggle);
                        printLine(MESSAGE_LINE, "");
                        printLine(STATUS_LINE1, "Player killed by monster, gg");
                        printLine(STATUS_LINE2, "You lose! (Press 'Q' to exit)");
                        char ch;
                        while ((ch = getch()) != 'Q')
                            ;
    
                        return 0;
                    }
                    
                }
                else {
                    mon->setPos((Pos){newX, newY});
                    monsterAt[newY][newX] = mon;
                    monsterAt[y][x] = NULL;
                    insert(heap, time + 1000 / mon->getSpeed(), mon->getPos());
                }
            }
        }
        delete node;
    }
}
