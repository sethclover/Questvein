#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>

#include "dungeon.hpp"
#include "game.hpp"

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
    {"g", "Teleport (goto)"},
    {"Q", "Quit the game"},
    {"?", "Show help"}
};

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

void printDungeon(bool supportsColor, bool fogOfWarToggle) {
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
                        if (supportsColor) {
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
                    if (supportsColor && dungeon[i][j].visible == FOG) {
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
        if (supportsColor) {
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
                    if (supportsColor) {
                        if (((i >= player.pos.y - 2 && i <= player.pos.y + 2) && (j >= player.pos.x - 2 && j <= player.pos.x + 2)) &&
                           !((i == player.pos.y - 2 || i == player.pos.y + 2) && (j == player.pos.x - 2 || j == player.pos.x + 2))) {
                            attron(COLOR_PAIR(2));
                            mvaddch(i + 1, j, dungeon[i][j].type);
                            attroff(COLOR_PAIR(2));
                        }
                        else {
                            mvaddch(i + 1, j, dungeon[i][j].type);
                        }
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

int monsterList(int monstersAlive, bool supportsColor, bool fogOfWarToggle) {
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
        if (supportsColor) {
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
            if (supportsColor) {
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
                printDungeon(supportsColor, fogOfWarToggle);
                return 0;
        }
    }
}

void commandList(bool supportsColor, bool fogOfWarToggle) {
    int count = sizeof(switches) / sizeof(CommandInfo);

    int cols = 55;
    int rows = 24;
    int leftCol = (MAX_WIDTH - cols) / 2;
    if (leftCol < 0) leftCol = 0;
    int top = 0;

    clear();
    while (1) {
        if (supportsColor) {
            attron(COLOR_PAIR(2));

            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');

            attroff(COLOR_PAIR(2));
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

            if (supportsColor) {
                attron(COLOR_PAIR(2));
                mvaddch(row, leftCol + cols - 1, '|');
                attroff(COLOR_PAIR(2));
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
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}
