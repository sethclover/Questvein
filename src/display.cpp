#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ncurses.h>

#include "display.hpp"
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

void printParsedMonsters() {
    std::cout << "MONSTER LIST:" << std::endl;
    for (const auto& m : monsterTypeList) {
        if (m.valid) {
            std::cout << "Name: " << m.name << std::endl;
            std::cout << "Description: " << m.desc;
            std::cout << "Color: ";
            for (const auto& c : m.colors) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
            std::cout << "Speed: " << m.speed.base << "+" << m.speed.rolls << "d" << m.speed.sides << std::endl;
            std::cout << "Abilities: ";
            for (const auto& a : m.abils) {
                std::cout << a << " ";
            }
            std::cout << std::endl;
            std::cout << "Hitpoints: " << m.hp.base << "+" << m.hp.rolls << "d" << m.hp.sides << std::endl;
            std::cout << "Attack Damage: " << m.dam.base << "+" << m.dam.rolls << "d" << m.dam.sides << std::endl;
            std::cout << "Symbol: " << m.symbol << std::endl;
            std::cout << "Rarity: " << m.rarity << std::endl << std::endl;
        }
    }
}

void printParsedObjects() {
    std::cout << "OBJECT LIST:" << std::endl;
    for (const auto& o : objectTypeList) {
        if (o.valid) {
            std::cout << "Name: " << o.name << std::endl;
            std::cout << "Description: " << o.desc;
            std::cout << "Type: ";
            for (const auto& t : o.type) {
                std::cout << t << " ";
            }
            std::cout << std::endl;
            std::cout << "Color: ";
            for (const auto& c : o.colors) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
            std::cout << "Hit bonus: " << o.hit.base << "+" << o.hit.rolls << "d" << o.hit.sides << std::endl;
            std::cout << "Damage bonus: " << o.dam.base << "+" << o.dam.rolls << "d" << o.dam.sides << std::endl;
            std::cout << "Dodge bonus: " << o.dodge.base << "+" << o.dodge.rolls << "d" << o.dodge.sides << std::endl;
            std::cout << "Defense bonus: " << o.def.base << "+" << o.def.rolls << "d" << o.def.sides << std::endl;
            std::cout << "Weight: " << o.weight.base << "+" << o.weight.rolls << "d" << o.weight.sides << std::endl;
            std::cout << "Speed bonus: " << o.speed.base << "+" << o.speed.rolls << "d" << o.speed.sides << std::endl;
            std::cout << "Special Attribute: " << o.attr.base << "+" << o.attr.rolls << "d" << o.attr.sides << std::endl;
            std::cout << "Value: " << o.val.base << "+" << o.val.rolls << "d" << o.val.sides << std::endl;
            std::cout << "Artifact status: " << (o.art ? "true" : "false") << std::endl;
            std::cout << "Rarity: " << o.rarity << std::endl << std::endl;
        }
    }
}

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

char *monToPersonalityString(Monster *mon) {
    int personality = 1 * mon->isIntelligent() +
                      2 * mon->isTelepathic() +
                      4 * mon->isTunneling() +
                      8 * mon->isErratic();
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

void printDungeon(bool supportsColor, bool fogOfWarToggle) {
    if (fogOfWarToggle) {
        for (int i = 0; i < MAX_HEIGHT; i++) {
            for (int j = 0; j < MAX_WIDTH; j++) {
                if (((i >= player.pos.y - 2 && i <= player.pos.y + 2) && (j >= player.pos.x - 2 && j <= player.pos.x + 2)) &&
                   !((i == player.pos.y - 2 || i == player.pos.y + 2) && (j == player.pos.x - 2 || j == player.pos.x + 2))) {
                    Monster *mon = monsterAt[i][j];
                    if (mon) {
                        Color c = mon->getColor();
                        if (supportsColor) {
                            attron(COLOR_PAIR(c));
                            mvaddch(i + 1, j, mon->getSymbol());
                            attroff(COLOR_PAIR(c));
                        }
                        else {
                            mvaddch(i + 1, j, mon->getSymbol());
                        }
                    }
                    else {
                        if (supportsColor) {
                            attron(COLOR_PAIR(Color::Yellow));
                            mvaddch(i + 1, j, dungeon[i][j].visible);
                            attroff(COLOR_PAIR(Color::Yellow));
                        }
                        else {
                            mvaddch(i + 1, j, dungeon[i][j].visible);
                        }
                    }
                }
                else {
                    if (supportsColor && dungeon[i][j].visible == FOG) {
                        attron(COLOR_PAIR(Color::Magenta));
                        mvaddch(i + 1, j, dungeon[i][j].visible);
                        attroff(COLOR_PAIR(Color::Magenta));
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
            attron(COLOR_PAIR(Color::Magenta));

            mvhline(1, 0, '-', MAX_WIDTH - 1);
            mvhline(MAX_HEIGHT, 0, '-', MAX_WIDTH - 1);
            mvvline(1, 0, '|', MAX_HEIGHT - 1);
            mvvline(1, MAX_WIDTH - 1, '|', MAX_HEIGHT - 1);

            mvaddch(1, 0, '+');
            mvaddch(1, MAX_WIDTH - 1, '+');
            mvaddch(MAX_HEIGHT, 0, '+');
            mvaddch(MAX_HEIGHT, MAX_WIDTH - 1, '+');

            attroff(COLOR_PAIR(Color::Magenta));
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
                    if (supportsColor) {
                        Color c = monsterAt[i][j]->getColor();
                        attron(COLOR_PAIR(c));
                        mvaddch(i + 1, j, monsterAt[i][j]->getSymbol());
                        attroff(COLOR_PAIR(c));
                    }
                    else {
                        mvaddch(i + 1, j, monsterAt[i][j]->getSymbol());
                    }
                }
                else if (player.pos.x == j && player.pos.y == i) {
                    mvaddch(i + 1, j, '@');
                }
                else {
                    if (supportsColor) {
                        if (((i >= player.pos.y - 2 && i <= player.pos.y + 2) && (j >= player.pos.x - 2 && j <= player.pos.x + 2)) &&
                           !((i == player.pos.y - 2 || i == player.pos.y + 2) && (j == player.pos.x - 2 || j == player.pos.x + 2))) {
                            attron(COLOR_PAIR(Color::Yellow));
                            mvaddch(i + 1, j, dungeon[i][j].type);
                            attroff(COLOR_PAIR(Color::Yellow));
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
    std::vector<std::string> allLines;
    std::vector<Color> colorList;
    int count = 0;
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                Monster *mon = monsterAt[i][j];
                MonsterType *monType = &monsterTypeList[mon->getMonTypeIndex()];

                std::string nameLine = mon->getName() + " (" + std::string(1, mon->getSymbol()) + ")";

                std::vector<std::string> abilityLines;
                int abilityCount = monType->abils.size();
                if (abilityCount <= 0) {
                    abilityLines.push_back("    - Abilities: None");
                } else {
                    std::string abilityLine = "    - Abilities: ";
                    int abilIndex = 0;
                    while (abilIndex < abilityCount) {
                        abilityLine += monType->abils[abilIndex];
                        abilIndex++;

                        if (abilIndex < abilityCount) {
                            abilityLine += ", ";
                        }

                        if (abilIndex % 4 == 0 || abilIndex == abilityCount) {
                            abilityLines.push_back(abilityLine);
                            abilityLine = "                 ";
                        }
                    }
                }
                

                std::string locationLine = "    - Location: ";
                int x = mon->getPos().x - player.pos.x;
                int y = mon->getPos().y - player.pos.y;
                const char* nsDir = (y >= 0) ? "South" : "North";
                const char* ewDir = (x >= 0) ? "East" : "West";
                int nsDist = abs(y);
                int ewDist = abs(x);
                if (nsDist == 0) {
                    locationLine += std::to_string(ewDist) + " " + ewDir;
                }
                else if (ewDist == 0) {
                    locationLine += std::to_string(nsDist) + " " + nsDir;
                }
                else {
                    locationLine += std::to_string(nsDist) + " " + nsDir + " and " + std::to_string(ewDist) + " " + ewDir;
                }

                allLines.push_back(nameLine);
                colorList.push_back(mon->getColor());

                for (const auto& line : abilityLines) {
                    allLines.push_back(line);
                    colorList.push_back(Color::White);
                }

                allLines.push_back(locationLine);
                colorList.push_back(Color::White);

                count++;
            }
        }
    }

    int cols = 55;
    int rows = 24;
    int leftCol = (MAX_WIDTH - cols) / 2;
    if (leftCol < 0) leftCol = 0;
    size_t topLine = 0;
    size_t maxDisplay = rows - 7;

    clear();
    if (supportsColor) {
        attron(COLOR_PAIR(Color::Magenta));

        mvhline(0, leftCol, '-', cols);
        mvhline(rows - 1, leftCol, '-', cols);
        mvvline(0, leftCol, '|', rows);
        mvvline(0, leftCol + cols - 1, '|', rows);

        mvaddch(0, leftCol + cols / 2, '+');
        mvaddch(0, leftCol, '+');
        mvaddch(rows - 1, leftCol + cols / 2, '+');
        mvaddch(rows - 1, leftCol, '+');

        attroff(COLOR_PAIR(Color::Magenta));
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

    while (1) {
        move(4, leftCol + cols / 2);
        if (topLine > 0) {
            printw("^");
        }
        else {
            printw(" ");
        }

        int displayStartRow = 5;
        for (size_t i = 0; i < maxDisplay; i++) {
            int row = displayStartRow + i;
            size_t lineIndex = topLine + i;

            move(row, leftCol + 2);
            clrtoeol();
            if (lineIndex < allLines.size()) {
                std::string line = allLines[lineIndex];
                if (supportsColor) {
                    printw("%s", line.substr(0, line.size() - 2).c_str());
                    attron(COLOR_PAIR(colorList[lineIndex]));
                    addch(line[line.size() - 2]);
                    attroff(COLOR_PAIR(colorList[lineIndex]));
                    addch(line[line.size() - 1]);
                }
                else {
                    printw("%s", line.c_str());
                }
            } 
            if (supportsColor) {
                attron(COLOR_PAIR(Color::Magenta));
                mvaddch(row, leftCol + cols - 1, '|');
                attroff(COLOR_PAIR(Color::Magenta));
            }
            else {
                mvaddch(row, leftCol + cols - 1, '|');
            }
        }

        move(rows - 2, leftCol + cols / 2);
        if (topLine + maxDisplay < allLines.size()) {
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
                if (topLine > 0) {
                    topLine--;
                }
                break;

            case KEY_DOWN:
                if (topLine + maxDisplay < allLines.size()) {
                    topLine++;
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
            attron(COLOR_PAIR(Color::Yellow));

            mvhline(0, leftCol, '-', cols);
            mvhline(rows - 1, leftCol, '-', cols);
            mvvline(0, leftCol, '|', rows);
            mvvline(0, leftCol + cols - 1, '|', rows);

            mvaddch(0, leftCol + cols / 2, '+');
            mvaddch(0, leftCol, '+');
            mvaddch(rows - 1, leftCol + cols / 2, '+');
            mvaddch(rows - 1, leftCol, '+');

            attroff(COLOR_PAIR(Color::Yellow));
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
                attron(COLOR_PAIR(Color::Yellow));
                mvaddch(row, leftCol + cols - 1, '|');
                attroff(COLOR_PAIR(Color::Yellow));
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
