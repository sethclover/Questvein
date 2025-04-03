#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>

#include "dungeon.hpp"
#include "fibonacciHeap.hpp"
#include "game.hpp"
#include "pathFinding.hpp"

class CommandInfo {
public:
    const char *buttons;
    const char *desc;
};

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
    {"f", "Toggle fog of war"},
    {"Q", "Quit the game"},
    {"?", "Show help"}
};

static int fogOfWarToggle = 1;

void printLine(int line, const char* format, ...) {
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

char *personalityToString(int personality) {
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

const char personalityToChar(Monster *mon) {
    int personality = 1 * mon->intelligent +
                      2 * mon->telepathic +
                      4 * mon->tunneling +
                      8 * mon->erratic;
    return (personality < 10) ? '0' + personality : 'A' + (personality - 10);
}

void printDungeon() {
    if (fogOfWarToggle) {
        for (int i = 0; i < MAX_HEIGHT; i++) {
            for (int j = 0; j < MAX_WIDTH; j++) {
                if (((i >= player.pos.y - 2 && i <= player.pos.y + 2) && (j >= player.pos.x - 2 && j <= player.pos.x + 2)) &&
                   !((i == player.pos.y - 2 || i == player.pos.y + 2) && (j == player.pos.x - 2 || j == player.pos.x + 2))) {
                    Monster *mon = monsterAt[i][j];
                    if (mon) {
                        mvaddch(i + 1, j, personalityToChar(mon));
                    }
                    else {
                        if (has_colors()) {
                            start_color();
                            init_pair(2, COLOR_YELLOW, COLOR_BLACK);
                            attron(COLOR_PAIR(2));

                            mvaddch(i + 1, j, dungeon[i][j].visible);

                            attroff(COLOR_PAIR(2));
                        }
                        else {
                            mvaddch(i + 1, j, dungeon[i][j].visible);
                        }
                    }
                }
                else {
                    if (has_colors()) {
                        start_color();
                        init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
                        attron(COLOR_PAIR(1));

                        mvaddch(i + 1, j, dungeon[i][j].visible);

                        attroff(COLOR_PAIR(1));
                    }
                    else {
                        mvaddch(i + 1, j, dungeon[i][j].visible);
                    }
                    
                }
            }
        }
        mvaddch(player.pos.y + 1, player.pos.x, '@');
    }
    else {
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
            attron(COLOR_PAIR(1));

            mvhline(1, 0, '-', MAX_WIDTH - 1);
            mvhline(MAX_HEIGHT, 0, '-', MAX_WIDTH - 1);
            mvvline(1, 0, '|', MAX_HEIGHT - 1);
            mvvline(1, MAX_WIDTH - 1, '|', MAX_HEIGHT - 1);

            mvaddch(1, 0, '+');
            mvaddch(1, MAX_WIDTH - 1, '+');
            mvaddch(MAX_HEIGHT, 0, '+');
            mvaddch(MAX_HEIGHT, MAX_WIDTH - 1, '+');

            attroff(COLOR_PAIR(1));
        }
        else {
            mvhline(1, 0, '-', MAX_WIDTH - 1);
            mvhline(MAX_HEIGHT, 0, '-', MAX_WIDTH - 1);
            mvvline(1, 0, '|', MAX_HEIGHT - 1);
            mvvline(1, MAX_WIDTH - 1, '|', MAX_HEIGHT - 1);

            mvaddch(1, 0, '+');
            mvaddch(1, MAX_WIDTH - 1, '+');
            mvaddch(MAX_HEIGHT, 0, '+');
            mvaddch(MAX_HEIGHT, MAX_WIDTH - 1, '+');
        }

        for (int i = 1; i < MAX_HEIGHT - 1; i++) {
            for (int j = 1; j < MAX_WIDTH - 1; j++) {
                if (monsterAt[i][j]) {
                    int personality = 1 * monsterAt[i][j]->intelligent +
                                      2 * monsterAt[i][j]->telepathic +
                                      4 * monsterAt[i][j]->tunneling +
                                      8 * monsterAt[i][j]->erratic;
                    mvaddch(i + 1, j, personality < 10 ? '0' + personality : 'A' + (personality - 10));
                }
                else if (player.pos.x == j && player.pos.y == i) {
                    mvaddch(i + 1, j, '@');
                }
                else {
                    if (has_colors()) {
                        start_color();
                        init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
                        attron(COLOR_PAIR(1));

                        mvaddch(i + 1, j, dungeon[i][j].type);
                        
                        attroff(COLOR_PAIR(1));
                    }
                    else {
                        mvaddch(i + 1, j, dungeon[i][j].type);
                    }
                }
            }
        }
    }
    
    printLine(MESSAGE_LINE, "Press a key to continue...");

    move(23, 0);
    clrtoeol();

    refresh();
}

int monsterList(int monstersAlive) {
    Monster *monList[monstersAlive];

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
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
            attron(COLOR_PAIR(1));

            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');

            attroff(COLOR_PAIR(1));
        }
        else {
            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');
        }

        const char title[13] = "Monster List";
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
            int row = 5 + (i - top);

            Monster *mon = monList[i];
            int personality = 1 * mon->intelligent +
                              2 * mon->telepathic +
                              4 * mon->tunneling +
                              8 * mon->erratic;
            char *traits = personalityToString(personality);

            int x = mon->pos.x - player.pos.x;
            int y = mon->pos.y - player.pos.y;
            const char* nsDir = (y >= 0) ? "South" : "North";
            const char* ewDir = (x >= 0) ? "East" : "West";
            int nsDist = abs(y);
            int ewDist = abs(x);

            move(row, leftCol + 2);
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
            if (has_colors()) {
                start_color();
                init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
                attron(COLOR_PAIR(1));

                mvaddch(row, leftCol + cols - 1, '|');

                attroff(COLOR_PAIR(1));
            }
            else {
                mvaddch(row, leftCol + cols - 1, '|');
            }
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
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_YELLOW, COLOR_BLACK);
            attron(COLOR_PAIR(1));

            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');

            attroff(COLOR_PAIR(1));
        }
        else {
            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');
        }

        const char title[13] = "Command List";
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

            if (has_colors()) {
                start_color();
                init_pair(1, COLOR_YELLOW, COLOR_BLACK);
                attron(COLOR_PAIR(1));

                mvaddch(row, leftCol + cols - 1, '|');

                attroff(COLOR_PAIR(1));
            }
            else {
                mvaddch(row, leftCol + cols - 1, '|');
            }
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
            
            dungeon[i][j].visible = FOG;
        }
    }
    nodes[player.pos.y][player.pos.x] = insert(heap, 100, player.pos);
    if (!nodes[player.pos.y][player.pos.x]) {
        cleanup(numMonsters, heap);

        return 1;
    }

    if (autoFlag) {
        fogOfWarToggle = 0;
    }
     
    while (1) {
        FibNode *node = extractMin(heap);
        if (!node) {
            cleanup(numMonsters, heap);

            return 1;
        }
        time = node->key;

        if (node->pos.x == player.pos.x && node->pos.y == player.pos.y) {
            updateAroundPlayer();
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
                        delete node;

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
                                if (generateStructures(numMonsters)) {
                                    return 1;
                                }
                                player.pos.x = upStairs[0].x;
                                player.pos.y = upStairs[0].y;
                                if (spawnMonsters(numMonsters, player.pos.x, player.pos.y)) {
                                    delete[] rooms;
                                    delete[] upStairs;
                                    delete[] downStairs;

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
                            if (dungeon[player.pos.y][player.pos.x].type == STAIR_UP) {
                                printLine(MESSAGE_LINE, "Going up stairs...");
                                napms(1000);
                                cleanup(numMonsters, heap);
                                delete node;

                                clear();
                                initDungeon();
                                if (generateStructures(numMonsters)) {
                                    return 1;
                                }
                                player.pos.x = downStairs[0].x;
                                player.pos.y = downStairs[0].y;
                                if (spawnMonsters(numMonsters, player.pos.x, player.pos.y)) {
                                    delete[] rooms;
                                    delete[] upStairs;
                                    delete[] downStairs;

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
                            fogOfWarToggle = !fogOfWarToggle;
                            printDungeon();
                            {
                                const char* fogStatus = fogOfWarToggle ? "on" : "off";
                                printLine(STATUS_LINE2, "Fog of war toggled %s", fogStatus);
                            }
                                
                            break;

                        case 'g':
                            {
                                int replaceFogOfWar = fogOfWarToggle;
                                fogOfWarToggle = 0;
                                printDungeon();

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
                                        mvaddch(oldY + 1, oldX, personalityToChar(monsterAt[oldY][oldX]));
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
                                    FibHeap *tempHeap = createFibHeap();
                                    if (!tempHeap) {
                                        cleanup(numMonsters, heap);
                                        delete node;
            
                                        return 1;
                                    }
                                    FibNode *tempNode = extractMin(heap);
                                    if (!tempNode) {
                                        cleanup(numMonsters, heap);
                                        delete node;
            
                                        return 1;
                                    }
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
                                        printDungeon();
                                        printLine(STATUS_LINE1, "Player killed all monsters!\n");
                                        printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                                        while (getch() != 'Q')
                                            ;
            
                                        cleanup(numMonsters, heap);
                                        delete node;
            
                                        return 0;
                                    }
                                    else {
                                        printLine(STATUS_LINE1, "Player stomped monster, Monsters alive: %d\n", monstersAlive);
                                    }
                                }
                                fogOfWarToggle = replaceFogOfWar;
                                updateAroundPlayer();
                                printDungeon();
                            }
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
                            delete node;
                            return 0;

                        case 'T':
                            // "display the tunneling distance map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case '?':
                            commandList();
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
                    int oldX = player.pos.x;
                    int oldY = player.pos.y;
                    player.pos.x += xDir;
                    player.pos.y += yDir;
                    if (monsterAt[player.pos.y][player.pos.x]) {
                        FibHeap *tempHeap = createFibHeap();
                        if (!tempHeap) {
                            cleanup(numMonsters, heap);
                            delete node;

                            return 1;
                        }
                        FibNode *tempNode = extractMin(heap);
                        if (!tempNode) {
                            cleanup(numMonsters, heap);
                            delete node;

                            return 1;
                        }
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

                        nodes[oldY][oldX] = NULL;
                        nodes[player.pos.y][player.pos.x] = insert(heap, time + 100, player.pos);
                        if (!nodes[player.pos.y][player.pos.x]) {
                            cleanup(numMonsters, heap);
                            delete node;

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
                            delete node;

                            return 0;
                        }
                        else {
                            printLine(STATUS_LINE1, "Player killed monster, Monsters alive: %d\n", monstersAlive);
                        }

                    }
                    else {
                        nodes[oldY][oldX] = NULL;
                        nodes[player.pos.y][player.pos.x] = insert(heap, time + 100, player.pos);
                        if (!nodes[player.pos.y][player.pos.x]) {
                            cleanup(numMonsters, heap);
                            delete node;

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
            Monster *mon = monsterAt[node->pos.y][node->pos.x];

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
                    player.pos.x >= rooms[i].x && player.pos.x <= rooms[i].x + rooms[i].width - 1 &&
                    player.pos.y >= rooms[i].y && player.pos.y <= rooms[i].y + rooms[i].height - 1) {
                    sameRoom = 1;
                    break;
                }
            }
            int visited[MAX_HEIGHT][MAX_WIDTH] = {0};

            int sameCorridor = checkCorridor(x, y, visited);

            int hasLastSeen = (mon->lastSeen.x != -1 && mon->lastSeen.y != -1);

            int canSee = (isTelepathic || sameRoom || sameCorridor);
            if (canSee) {
                mon->lastSeen = player.pos;
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
                    generateDistances(player.pos.x, player.pos.y);
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
                    if ((isTunneling && dungeon[newY][newX].tunnelingDist == UNREACHABLE) ||
                        (!isTunneling && dungeon[newY][newX].nonTunnelingDist == UNREACHABLE)) {
                        nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                        if (!nodes[y][x]) {
                            cleanup(numMonsters, heap);
                            delete node;

                            return 1;
                        }
                        delete node;
                        continue;
                    }
                }
            } 
            else {
                nodes[y][x] = insert(heap, time + 1000 / mon->speed, mon->pos);
                if (node) {
                    delete node;
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
                        delete node;

                        return 1;
                    }
                }
            }
            else {
                if (monsterAt[newY][newX]) {
                    FibHeap *tempHeap = createFibHeap();
                    if (!tempHeap) {
                        cleanup(numMonsters, heap);
                        delete node;

                        return 1;
                    }
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
                    nodes[y][x] = NULL;
                    nodes[newY][newX] = insert(heap, time + 1000 / mon->speed, mon->pos);
                    if (!nodes[newY][newX]) {
                        cleanup(numMonsters, heap);
                        delete node;

                        return 1;
                    }

                    if (newX != x || newY != y) {
                        monstersAlive--;
                    }
                }
                else if (newX == player.pos.x && newY == player.pos.y) {
                    if (godmodeFlag) {
                        monsterAt[y][x] = NULL;
                        nodes[y][x] = NULL;

                        monstersAlive--;
                        if (monstersAlive <= 0) {
                            updateAroundPlayer();
                            printDungeon();
                            printLine(STATUS_LINE1, "Player killed all monsters!\n");
                            printLine(STATUS_LINE2, "You win! (Press 'Q' to exit)");
                            while (getch() != 'Q')
                                ;

                            cleanup(numMonsters, heap);
                            delete node;

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
    
                        updateAroundPlayer();
                        printDungeon();
                        printLine(MESSAGE_LINE, "");
                        printLine(STATUS_LINE1, "Player killed by monster, gg");
                        printLine(STATUS_LINE2, "You lose! (Press 'Q' to exit)");
                        char ch;
                        while ((ch = getch()) != 'Q')
                            ;
    
                        cleanup(numMonsters, heap);
                        delete node;

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
                        delete node;

                        return 1;
                    }
                }
            }
        }
        delete node;
    }
}