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
    {"d", "Drop item"},
    {"e", "Show equipment"},
    {"f", "Toggle fog of war"},
    {"g", "Teleport (goto)"},
    {"i", "Show inventory"},
    {"m", "Show monster list"},
    {"o", "Show object list"},
    {"t", "Take off item"},
    {"w", "Wear item"},
    {"x", "Expunge item"},
    {"D", "Show mon-tunneling map"},
    {"E", "Inspect equipment item"},
    {"I", "Inspect inventory item"},
    {"L", "Look at monster"},
    {"Q", "Quit the game"},
    {"T", "Show tunneling map"},
    {",", "Pick up item"},
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
            for (const auto& t : o.types) {
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

void printLineColor(int line, Color color, bool supportsColor, const char* format, ...) {
    char buffer[MAX_WIDTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, MAX_WIDTH, format, args);
    va_end(args);

    move(line, 0);
    clrtoeol();
    if (supportsColor) {
        attron(COLOR_PAIR(color));
        printw("%s", buffer);
        attroff(COLOR_PAIR(color));
    }
    else {
        printw("%s", buffer);
    }

    refresh();
}

void openEquipment(bool supportsColor, bool fogOfWarToggle) {
    clear();

    printLine(MESSAGE_LINE, "Equipment:");
    printLine(STATUS_LINE1, "Press 'e' to return to the game.");

    mvhline(1, 0, '~', MAX_WIDTH - 1);
    mvhline(MAX_HEIGHT, 0, '~', MAX_WIDTH - 1);

    mvaddch(1, 0, '*');
    mvaddch(1, MAX_WIDTH - 1, '*');
    mvaddch(MAX_HEIGHT, 0, '*');
    mvaddch(MAX_HEIGHT, MAX_WIDTH - 1, '*');

    for (int i = 0; i < static_cast<int>(Equip::Count); i++) {
        mvaddch(4, 1 + i * 3, ' ');
        if (player.getEquipmentItem((Equip)i) == nullptr) {
            addch('.');
        }
        else {
            Color c = player.getEquipmentItem((Equip)i)->getColor();
            if (supportsColor) {
                attron(COLOR_PAIR(c));
                addch(player.getEquipmentItem((Equip)i)->getSymbol());
                attroff(COLOR_PAIR(c));
            }
            else {
                addch(player.getEquipmentItem((Equip)i)->getSymbol());
            }
        }
        addch(' ');

        mvaddch(5, 2 + i * 3, (char)('a' + i));
    }

    std::vector<std::vector<std::string>> allLines;
    for (int i = 0; i < static_cast<int>(Equip::Count); i++) {
        std::vector<std::string> lines;
        if (player.getEquipmentItem((Equip)i) != nullptr) {
            Object *obj = player.getEquipmentItem((Equip)i);

            std::string nameLine = "Name: " + player.getEquipmentItem((Equip)i)->getName();
            lines.push_back(nameLine);

            if (obj->getDamageBonus().base > 0 || (obj->getDamageBonus().rolls > 0 && obj->getDamageBonus().sides > 0)) {
                std::string damageBonusLine = "Damage Bonus: " + std::to_string(obj->getDamageBonus().base) + "+";
                damageBonusLine += std::to_string(obj->getDamageBonus().rolls) + "d" + std::to_string(obj->getDamageBonus().sides);
                lines.push_back(damageBonusLine);
            }
            if (obj->getSpeedBonus() > 0) {
                std::string speedBonusLine = "Speed Bonus: " + std::to_string(obj->getSpeedBonus());
                lines.push_back(speedBonusLine);
            }
            if (obj->getHitBonus() > 0) {
                std::string hitBonusLine = "Hit Bonus: " + std::to_string(obj->getHitBonus());
                lines.push_back(hitBonusLine);
            }
            if (obj->getDodgeBonus() > 0) {
                std::string dodgeBonusLine = "Dodge Bonus: " + std::to_string(obj->getDodgeBonus());
                lines.push_back(dodgeBonusLine);
            }
            if (obj->getDefenseBonus() > 0) {
                std::string defenseBonusLine = "Defense Bonus: " + std::to_string(obj->getDefenseBonus());
                lines.push_back(defenseBonusLine);
            }
            if (obj->getWeight() > 0) {
                std::string weightLine = "Weight: " + std::to_string(obj->getWeight());
                lines.push_back(weightLine);
            }
            if (obj->getSpecialAttribute() > 0) {
                std::string specialAttributeLine = "Special Attribute: " + std::to_string(obj->getSpecialAttribute());
                lines.push_back(specialAttributeLine);
            }
            std::string valueLine = "Value: " + std::to_string(obj->getValue());
            lines.push_back(valueLine);
            if (obj->isArtifact()) {
                lines.push_back("Artifact: true");
            }
            else {
                lines.push_back("Artifact: false");
            }

            lines.push_back("Description: ");
            std::string desc = player.getEquipmentItem((Equip)i)->getDescription();
            int j = 0;
            for (size_t i = 0; i < desc.length(); i++) {
                if (desc[i] == '\n') {
                    lines.push_back(desc.substr(j, i - j));
                    j = i + 1;
                }
            }
        }
        allLines.push_back(lines);
    }

    size_t maxDisplay = MAX_HEIGHT - 9;
    size_t topLine = 0;
    int cursor = 0;

    while(true) {
        mvaddch(3, 2 + cursor * 3, 'v');

        for (int i = 6; i < MAX_HEIGHT - 1; i++) {
            move(i, 0);
            clrtoeol();
        }
        mvprintw(6, 0, "Slot: %s", player.getEquipmentName(cursor));

        move(7, 1);
        clrtoeol();
        if (topLine > 0) {
            addch('^');
        }
        else {
            addch(' ');
        }

        for (size_t i = 0; i < maxDisplay; i++) {
            int row = i + 8;
            size_t lineIndex = topLine + i;

            move(row, 0);
            clrtoeol();
            if (lineIndex < allLines[cursor].size()) {
                std::string line = allLines[cursor][lineIndex];
                printw("%s", line.c_str());
            }
        }
        move(MAX_HEIGHT - 1, 1);

        if (topLine + maxDisplay < allLines[cursor].size()) {
            addch('v');
        }
        else {
            addch(' ');
        }

        refresh();

        int ch;
        do {
            ch = getch();
        } while (ch != KEY_RIGHT && ch != '6' && ch != 'l' &&
                 ch != KEY_LEFT && ch != '4' && ch != 'h' &&
                 ch != KEY_UP && ch != KEY_DOWN && ch != 'e' && ch != 27);

        switch (ch) {
            case KEY_RIGHT:
            case '6':
            case 'l':
                if (cursor < static_cast<int>(Equip::Count) - 1) {
                    mvaddch(3, 2 + cursor * 3, ' ');
                    cursor++;
                }
                break;

            case KEY_LEFT:
            case '4':
            case 'h':
                if (cursor > 0) {
                    mvaddch(3, 2 + cursor * 3, ' ');
                    cursor--;
                }
                break;

            case KEY_UP:
                if (topLine > 0) {
                    topLine--;
                }
                break;

            case KEY_DOWN:
                if (topLine + maxDisplay < allLines[cursor].size()) {
                    topLine++;
                }
                break;

            case 'e':
            case 27:
                clear();
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}

void openInventory(bool supportsColor, bool fogOfWarToggle) {
    clear();

    printLine(MESSAGE_LINE, "Inventory:");
    printLine(STATUS_LINE1, "Press 'i' to return to the game.");

    mvhline(1, 0, '-', MAX_WIDTH - 1);
    mvhline(MAX_HEIGHT, 0, '-', MAX_WIDTH - 1);

    mvaddch(1, 0, '+');
    mvaddch(1, MAX_WIDTH - 1, '+');
    mvaddch(MAX_HEIGHT, 0, '+');
    mvaddch(MAX_HEIGHT, MAX_WIDTH - 1, '+');

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        mvaddch(4, 1 + i * 3, ' ');
        if (player.getInventoryItem(i) == nullptr) {
            addch('.');
        }
        else {
            Color c = player.getInventoryItem(i)->getColor();
            if (supportsColor) {
                attron(COLOR_PAIR(c));
                addch(player.getInventoryItem(i)->getSymbol());
                attroff(COLOR_PAIR(c));
            }
            else {
                addch(player.getInventoryItem(i)->getSymbol());
            }
        }
        addch(' ');

        mvaddch(5, 2 + i * 3, (char)('0' + i));
    }

    std::vector<std::vector<std::string>> allLines;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        std::vector<std::string> lines;
        if (player.getInventoryItem(i) != nullptr) {
            Object *obj = player.getInventoryItem(i);

            std::string nameLine = "Name: " + player.getInventoryItem(i)->getName();
            lines.push_back(nameLine);

            if (obj->getDamageBonus().base > 0 || (obj->getDamageBonus().rolls > 0 && obj->getDamageBonus().sides > 0)) {
                std::string damageBonusLine = "Damage Bonus: " + std::to_string(obj->getDamageBonus().base) + "+";
                damageBonusLine += std::to_string(obj->getDamageBonus().rolls) + "d" + std::to_string(obj->getDamageBonus().sides);
                lines.push_back(damageBonusLine);
            }
            if (obj->getSpeedBonus() > 0) {
                std::string speedBonusLine = "Speed Bonus: " + std::to_string(obj->getSpeedBonus());
                lines.push_back(speedBonusLine);
            }
            if (obj->getHitBonus() > 0) {
                std::string hitBonusLine = "Hit Bonus: " + std::to_string(obj->getHitBonus());
                lines.push_back(hitBonusLine);
            }
            if (obj->getDodgeBonus() > 0) {
                std::string dodgeBonusLine = "Dodge Bonus: " + std::to_string(obj->getDodgeBonus());
                lines.push_back(dodgeBonusLine);
            }
            if (obj->getDefenseBonus() > 0) {
                std::string defenseBonusLine = "Defense Bonus: " + std::to_string(obj->getDefenseBonus());
                lines.push_back(defenseBonusLine);
            }
            if (obj->getWeight() > 0) {
                std::string weightLine = "Weight: " + std::to_string(obj->getWeight());
                lines.push_back(weightLine);
            }
            if (obj->getSpecialAttribute() > 0) {
                std::string specialAttributeLine = "Special Attribute: " + std::to_string(obj->getSpecialAttribute());
                lines.push_back(specialAttributeLine);
            }
            std::string valueLine = "Value: " + std::to_string(obj->getValue());
            lines.push_back(valueLine);
            if (obj->isArtifact()) {
                lines.push_back("Artifact: true");
            }
            else {
                lines.push_back("Artifact: false");
            }

            lines.push_back("Description: ");
            std::string desc = player.getInventoryItem(i)->getDescription();
            int j = 0;
            for (size_t i = 0; i < desc.length(); i++) {
                if (desc[i] == '\n') {
                    lines.push_back(desc.substr(j, i - j));
                    j = i + 1;
                }
            }
        }
        allLines.push_back(lines);
    }

    size_t maxDisplay = MAX_HEIGHT - 9;
    size_t topLine = 0;
    int cursor = 0;

    while(true) {
        mvaddch(3, 2 + cursor * 3, 'v');

        for (int i = 6; i < MAX_HEIGHT - 1; i++) {
            move(i, 0);
            clrtoeol();
        }
        
        move(7, 1);
        clrtoeol();
        if (topLine > 0) {
            addch('^');
        }
        else {
            addch(' ');
        }

        for (size_t i = 0; i < maxDisplay; i++) {
            int row = i + 8;
            size_t lineIndex = topLine + i;

            move(row, 0);
            clrtoeol();
            if (lineIndex < allLines[cursor].size()) {
                std::string line = allLines[cursor][lineIndex];
                printw("%s", line.c_str());
            }
        }

        move(MAX_HEIGHT - 1, 1);
        if (topLine + maxDisplay < allLines[cursor].size()) {
            addch('v');
        }
        else {
            addch(' ');
        }

        refresh();

        int ch;
        do {
            ch = getch();
        } while (ch != KEY_RIGHT && ch != '6' && ch != 'l' &&
                 ch != KEY_LEFT && ch != '4' && ch != 'h' && 
                 ch != KEY_UP && ch != KEY_DOWN && ch != 'i' && ch != 27);

        switch (ch) {
            case KEY_RIGHT:
            case '6':
            case 'l':
                if (cursor < INVENTORY_SIZE - 1) {
                    mvaddch(3, 2 + cursor * 3, ' ');
                    cursor++;
                }
                break;

            case KEY_LEFT:
            case '4':
            case 'h':
                if (cursor > 0) {
                    mvaddch(3, 2 + cursor * 3, ' ');
                    cursor--;
                }
                break;

            case KEY_UP:
                if (topLine > 0) {
                    topLine--;
                }
                break;

            case KEY_DOWN:
                if (topLine + maxDisplay < allLines[cursor].size()) {
                    topLine++;
                }
                break;

            case 'i':
            case 27:
                clear();
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}

void printStatus(bool supportsColor) {
    move(23, 0);
    clrtoeol();
    if (supportsColor) {
        
        printw("HP: ");

        double percent = static_cast<double>(player.getHitpoints()) / player.getMaxHitpoints();
        if (percent >= 0.75) {
            attron(COLOR_PAIR(Color::Green));
            printw("%d/%d", player.getHitpoints(), player.getMaxHitpoints());
            attroff(COLOR_PAIR(Color::Green));
        }
        else if (percent >= 0.25) {
            attron(COLOR_PAIR(Color::Yellow));
            printw("%d/%d", player.getHitpoints(), player.getMaxHitpoints());
            attroff(COLOR_PAIR(Color::Yellow));
        }
        else {
            attron(COLOR_PAIR(Color::Red));
            printw("%d/%d", player.getHitpoints(), player.getMaxHitpoints());
            attroff(COLOR_PAIR(Color::Red));
        }
        printw("   Speed: %d   Position: (%d, %d)", player.getSpeed(), player.getPos().x, player.getPos().y);
    }
    else {
        printw("HP: %d/%d   Speed: %d   Position: (%d, %d)", player.getHitpoints(), player.getMaxHitpoints(), 
                                                             player.getSpeed(), player.getPos().x, player.getPos().y);
    }
}

void printDungeon(bool supportsColor, bool fogOfWarToggle) {
    if (fogOfWarToggle) {
        for (int i = 0; i < MAX_HEIGHT; i++) {
            for (int j = 0; j < MAX_WIDTH; j++) {
                if (inLineOfSight((Pos){j, i})) {
                    if (monsterAt[i][j]) {
                        Color c =  monsterAt[i][j].get()->getColor();
                        if (supportsColor) {
                            attron(COLOR_PAIR(c));
                            mvaddch(i + 1, j,  monsterAt[i][j].get()->getSymbol());
                            attroff(COLOR_PAIR(c));
                        }
                        else {
                            mvaddch(i + 1, j,  monsterAt[i][j].get()->getSymbol());
                        }
                    }
                    else if (!objectsAt[i][j].empty()) {
                        Color c = objectsAt[i][j].back()->getColor();
                        if (objectsAt[i][j].size() > 1) {
                            if (supportsColor) {
                                attron(COLOR_PAIR(c));
                                mvaddch(i + 1, j, '&');
                                attroff(COLOR_PAIR(c));
                            }
                            else {
                                mvaddch(i + 1, j, '&');
                            }
                        }
                        else {
                            if (supportsColor) {
                                attron(COLOR_PAIR(c));
                                mvaddch(i + 1, j, objectsAt[i][j].back()->getSymbol());
                                attroff(COLOR_PAIR(c));
                            }
                            else {
                                mvaddch(i + 1, j, objectsAt[i][j].back()->getSymbol());
                            }
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

        Monster *mon = monsterAt[player.getPos().y][player.getPos().x].get();
        if (mon) {
            Color c = mon->getColor();
            if (supportsColor) {
                attron(COLOR_PAIR(c));
                mvaddch(player.getPos().y + 1, player.getPos().x, mon->getSymbol());
                attroff(COLOR_PAIR(c));
            }
            else {
                mvaddch(player.getPos().y + 1, player.getPos().x, mon->getSymbol());
            }
        }
        else {
            mvaddch(player.getPos().y + 1, player.getPos().x, '@');
        }
        
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
                        Color c = monsterAt[i][j].get()->getColor();
                        attron(COLOR_PAIR(c));
                        mvaddch(i + 1, j, monsterAt[i][j].get()->getSymbol());
                        attroff(COLOR_PAIR(c));
                    }
                    else {
                        mvaddch(i + 1, j, monsterAt[i][j].get()->getSymbol());
                    }
                }
                else if (player.getPos().x == j && player.getPos().y == i) {
                    mvaddch(i + 1, j, '@');
                }
                else if (!objectsAt[i][j].empty()) {
                    Color c = objectsAt[i][j].back()->getColor();
                    if (objectsAt[i][j].size() > 1) {
                        if (supportsColor) {
                            attron(COLOR_PAIR(c));
                            mvaddch(i + 1, j, '&');
                            attroff(COLOR_PAIR(c));
                        }
                        else {
                            mvaddch(i + 1, j, '&');
                        }
                    }
                    else {
                        if (supportsColor) {
                            attron(COLOR_PAIR(c));
                            mvaddch(i + 1, j, objectsAt[i][j].back()->getSymbol());
                            attroff(COLOR_PAIR(c));
                        }
                        else {
                            mvaddch(i + 1, j, objectsAt[i][j].back()->getSymbol());
                        }
                    }
                }
                else {
                    if (supportsColor) {
                        if (inLineOfSight((Pos){j, i})) {
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
    
    printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help.");
    printStatus(supportsColor);

    refresh();
}

void monsterList(bool supportsColor, bool fogOfWarToggle) {
    std::vector<std::string> allLines;
    std::vector<Color> colorList;
    int count = 0;
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                Monster *mon = monsterAt[i][j].get();
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
                int x = mon->getPos().x - player.getPos().x;
                int y = mon->getPos().y - player.getPos().y;
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
        attron(COLOR_PAIR(Color::Green));

        mvhline(0, leftCol, '-', cols);
        mvhline(rows - 1, leftCol, '-', cols);
        mvvline(0, leftCol, '|', rows);
        mvvline(0, leftCol + cols - 1, '|', rows);

        mvaddch(0, leftCol + cols / 2, '+');
        mvaddch(0, leftCol, '+');
        mvaddch(rows - 1, leftCol + cols / 2, '+');
        mvaddch(rows - 1, leftCol, '+');
        mvaddch(0, leftCol + cols - 1, '+');
        mvaddch(rows - 1, leftCol + cols - 1, '+');

        attroff(COLOR_PAIR(Color::Green));
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
        mvaddch(0, leftCol + cols - 1, '+');
        mvaddch(rows - 1, leftCol + cols - 1, '+');
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
                attron(COLOR_PAIR(Color::Green));
                mvaddch(row, leftCol + cols - 1, '|');
                attroff(COLOR_PAIR(Color::Green));
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
        } while (ch != KEY_UP && ch != KEY_DOWN && ch != 'm' && ch != 27);

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

            case 'm':
            case 27:
                clear();
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}

void objectList(bool supportsColor, bool fogOfWarToggle) {
    std::vector<std::string> allLines;
    std::vector<Color> colorList;
    int count = 0;
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            for (const auto& obj : objectsAt[i][j]) {
                std::string nameLine = obj->getName() + " (" + std::string(1, obj->getSymbol()) + ")";

                std::string locationLine = "    - Location: ";
                int x = obj->getPos().x - player.getPos().x;
                int y = obj->getPos().y - player.getPos().y;
                const char* nsDir = (y >= 0) ? "South" : "North";
                const char* ewDir = (x >= 0) ? "East" : "West";
                int nsDist = abs(y);
                int ewDist = abs(x);
                if (nsDist == 0 && ewDist == 0) {
                    locationLine += "Here";
                }
                else if (nsDist == 0) {
                    locationLine += std::to_string(ewDist) + " " + ewDir;
                }
                else if (ewDist == 0) {
                    locationLine += std::to_string(nsDist) + " " + nsDir;
                }
                else {
                    locationLine += std::to_string(nsDist) + " " + nsDir + " and " + std::to_string(ewDist) + " " + ewDir;
                }

                allLines.push_back(nameLine);
                colorList.push_back(obj->getColor());

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
        attron(COLOR_PAIR(Color::Cyan));

        mvhline(0, leftCol, '-', cols);
        mvhline(rows - 1, leftCol, '-', cols);
        mvvline(0, leftCol, '|', rows);
        mvvline(0, leftCol + cols - 1, '|', rows);

        mvaddch(0, leftCol + cols / 2, '+');
        mvaddch(0, leftCol, '+');
        mvaddch(rows - 1, leftCol + cols / 2, '+');
        mvaddch(rows - 1, leftCol, '+');
        mvaddch(0, leftCol + cols - 1, '+');
        mvaddch(rows - 1, leftCol + cols - 1, '+');

        attroff(COLOR_PAIR(Color::Cyan));
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
        mvaddch(0, leftCol + cols - 1, '+');
        mvaddch(rows - 1, leftCol + cols - 1, '+');
    }

    const char title[13] = "Object List";
    int titleCol = leftCol + (cols - strlen(title)) / 2;
    mvprintw(1, titleCol, "%s", title);
    mvprintw(3, leftCol + 2, "Objects in Dungeon: %d", count);
    
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
                attron(COLOR_PAIR(Color::Cyan));
                mvaddch(row, leftCol + cols - 1, '|');
                attroff(COLOR_PAIR(Color::Cyan));
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
        } while (ch != KEY_UP && ch != KEY_DOWN && ch != 'o' && ch != 27);

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

            case 'o':
            case 27:
                clear();
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}

void nonTunnelingDistMap(bool supportsColor, bool fogOfWarToggle) {
    clear();
    printLine(MESSAGE_LINE, "Press 'ESC' or 'T' to return");
    printLine(STATUS_LINE2, "Non-tunneling distance map.");

    generateDistances(player.getPos());
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            move(i + 1, j);
            if (dungeon[i][j].nonTunnelingDist == UNREACHABLE) {
                addch(' ');
            }
            else if (dungeon[i][j].nonTunnelingDist == 0) {
                addch('@');
            }
            else {
                if (supportsColor) {
                    if (dungeon[i][j].nonTunnelingDist < 10) {
                        attron(COLOR_PAIR(Color::Red));
                        addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Red));
                    }
                    else if (dungeon[i][j].nonTunnelingDist < 20) {
                        attron(COLOR_PAIR(Color::Yellow));
                        addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Yellow));
                    }
                    else if (dungeon[i][j].nonTunnelingDist < 30) {
                        attron(COLOR_PAIR(Color::Green));
                        addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Green));
                    }
                    else if (dungeon[i][j].nonTunnelingDist < 40) {
                        attron(COLOR_PAIR(Color::Cyan));
                        addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Cyan));
                    }
                    else {
                        attron(COLOR_PAIR(Color::Blue));
                        addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Blue));
                    }
                }
                else {
                    addch(dungeon[i][j].nonTunnelingDist % 10 + '0');
                } 
            }
        }
    }

    refresh();

    int ch;
    do {
        ch = getch();
    } while (ch != 27 && ch != 'T');

    printDungeon(supportsColor, fogOfWarToggle);
}

void showEquipmentObjectDescription(bool supportsColor, bool fogOfWarToggle) {
    printLine(MESSAGE_LINE, "Choose an equipment slot a-l");
    int ch = getch();
    if (ch >= 'a' && ch <= 'l') {
        int index = ch - 'a';
        if (player.getEquipmentItem((Equip)index) == nullptr) {
            printLine(MESSAGE_LINE, "Nothing in that slot.");
            return;
        }
        std::string itemName = player.getEquipmentItem((Equip)index)->getDescription();
        clear();
        mvprintw(0, 0, "%s", itemName.c_str());
        getch();
        printDungeon(supportsColor, fogOfWarToggle);
    }
    else if (ch == 'E' || ch == 27) {
        printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
    }
    else {
        printLine(MESSAGE_LINE, "Not a valid slot.");
    }
}

void showInventoryObjectDescription(bool supportsColor, bool fogOfWarToggle) {
    printLine(MESSAGE_LINE, "Choose an inventory slot 0-9");
    int ch = getch();
    if (ch >= '0' && ch <= '9') {
        int index = ch - '0';
        if (player.getInventoryItem(index) == nullptr) {
            printLine(MESSAGE_LINE, "Nothing in that slot.");
            return;
        }
        std::string itemName = player.getInventoryItem(index)->getDescription();
        clear();
        mvprintw(0, 0, "%s", itemName.c_str());
        getch();
        printDungeon(supportsColor, fogOfWarToggle);
    }
    else if (ch == 'I' || ch == 27) {
        printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
    }
    else {
        printLine(MESSAGE_LINE, "Not a valid slot.");
    }
}

void showMonsterInfo(Pos pos, bool supportsColor, bool fogOfWarToggle) {
    Monster *mon = monsterAt[pos.y][pos.x].get();
    if (mon == nullptr) {
        return;
    }
    clear();
    mvprintw(0, 0, "%s", mon->getName().c_str());
    mvprintw(2, 0, "%s", mon->getDescription().c_str());
    
    getch();
    printDungeon(supportsColor, fogOfWarToggle);    
}

void tunnelingDistMap(bool supportsColor, bool fogOfWarToggle) {
    clear();
    printLine(MESSAGE_LINE, "Press 'ESC' or 'D' to return");
    printLine(STATUS_LINE2, "Tunneling distance map.");

    generateDistances(player.getPos());   
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            move(i + 1, j);
            if (dungeon[i][j].tunnelingDist == UNREACHABLE) {
                addch(' ');
            }
            else if (dungeon[i][j].tunnelingDist == 0) {
                addch('@');
            }
            else {
                if (supportsColor) {
                    if (dungeon[i][j].tunnelingDist < 10) {
                        attron(COLOR_PAIR(Color::Red));
                        addch(dungeon[i][j].tunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Red));
                    }
                    else if (dungeon[i][j].tunnelingDist < 20) {
                        attron(COLOR_PAIR(Color::Yellow));
                        addch(dungeon[i][j].tunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Yellow));
                    }
                    else if (dungeon[i][j].tunnelingDist < 30) {
                        attron(COLOR_PAIR(Color::Green));
                        addch(dungeon[i][j].tunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Green));
                    }
                    else if (dungeon[i][j].tunnelingDist < 40) {
                        attron(COLOR_PAIR(Color::Cyan));
                        addch(dungeon[i][j].tunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Cyan));
                    }
                    else {
                        attron(COLOR_PAIR(Color::Blue));
                        addch(dungeon[i][j].tunnelingDist % 10 + '0');
                        attroff(COLOR_PAIR(Color::Blue));
                    }
                }
                else {
                    addch(dungeon[i][j].tunnelingDist % 10 + '0');
                } 
            }
        }
    }

    refresh();

    int ch;
    do {
        ch = getch();
    } while (ch != 27 && ch != 'D');

    printDungeon(supportsColor, fogOfWarToggle);
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
            mvaddch(0, leftCol + cols - 1, '+');
            mvaddch(rows - 1, leftCol + cols - 1, '+');

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
            mvaddch(0, leftCol + cols - 1, '+');
            mvaddch(rows - 1, leftCol + cols - 1, '+');
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
        } while (ch != KEY_UP && ch != KEY_DOWN && ch != '?' && ch != 27);

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

            case '?':
            case 27:
                clear();
                printDungeon(supportsColor, fogOfWarToggle);
                return;
        }
    }
}

void lossScreen(bool supportsColor) {
    clear();

    std::string youDied =   "                                         ..          .                ..       \n"
                            "  ..                                    dF          @88>             dF        \n"
                            " @L                u.     x            '88bu.       %8P             '88bu.     \n"
                            "9888i   .dL  ...ue888b  .@88k  z88u    '*88888bu     .         .u   '*88888bu  \n"
                            "`Y888k:*888. 888R Y888r \"8888 ^8888      ^\"*8888N  .@88u    ud8888.   ^\"*8888N \n"
                            "  888E  888I 888R I888>  8888  888R     beWE \"888L''888E` :888'8888. beWE \"888L\n"
                            "  888E  888I 888R I888>..8888..888R     888E  888E  888E  d888 '88%\" 888E  888E\n"
                            "  888E  888I 888R.I888>..8888..888R.... 888E  888E  888E  8888.+\"    888E  888E\n"
                            "  888E  888Iu8888cJ888...8888-.888B.....888E  888F  888E  8888L      888E  888F\n"
                            "  x888N><888'\"*888*P\"....\"8888Y-8888\".:-.888N..888  888&  '8888c. .+.888N..888 \n"
                            "  \"88\"  888 .::.'Y\".......`Y\"...'YP:.::::`\"888*\"\"   R888\"  \"88888%   `\"888*\"\"  \n"
                            "        88F.:::..........::.:*@@%*:...+%#*=:\"\"        \"\"      \"YP'       \"\"    \n"
                            "       98\".-:.............:*@@@%%*+:..:#%#*=..                                 \n"
                            "     ./\"  :-:...........:.=@@@%%#++:...++##+:.                                 \n"
                            "    ~`    .-:..........:=.-%%%#++=:...+*+*#+:.                                 \n"
                            "          .--:...:::...:=:.:=++=-:...=%*+=:....                                \n"
                            "          .:--..:-=++=:.:=:..........=%%+=-:.::                                \n"
                            "           .----:=***-...:::..........:-:.:--:.                                \n"
                            "            .---==--=++-:::::::::::......:::.                                  \n"
                            "             .--*##%%++++-:=*=:...:x+:+++:x:..                                 \n"
                            "              .-=**#@@*---:=%+=-:::+x:+:+xxx:.                                 \n"
                            "               .:=+*#%%+--:.++=-:::::..:....::.                                \n"
                            "                  .::::=---..--:.::::.........                                 \n"
                            "                       .:---::::.:.........                         'Q' to quit\n";

std::string colorField =    "                                         rr          r                rr       \n"
                            "  rr                                    rr          rrrt             rr        \n"
                            " rr                rr     r            rrrrrr       rrtt            rrrrrr     \n"
                            "rrrrr   rrr  rrrrrrrrr  rrrrr  rrrr    rrrrrrrrrr    r         rr   rrrrrrrrr  \n"
                            "rrrrrrttrrrr rrrr rrrrt rrrrr rrrrr      rrrrrrrr  rrrrr    rrrrrrr   rrrrrrrt \n"
                            "  rrrr  rrrr rrrr rrrrt  rrrr  rrrr     rrrr rrrrrrrrrrrt rrrrrrrrrt rrrr rrrrt\n"
                            "  rrrr  rrrr rrrr rrrrtwwrrrrwwrrrr     rrrr  rrrt  rrrr  rrrr rrrrt rrrt  rrrt\n"
                            "  rrrr  rrrt rrrrwrrrrtwwrrrrwwrrrtwwww rrrr  rrrt  rrrr  rrrrrtt    rrrt  rrrt\n"
                            "  rrrr  rrrtrrrrrrrrrtwwwrrrrwtrrrtwtwwwrrrr  rrrt  rrrr  rrrrr      rrrt  rrtt\n"
                            "  rrrrtrrrrttrrrrrrttwwwwrrrrrtwrrtttwwwrrrrrttrrt  rrrr  rrrrrrt ttrrrrrttrtt \n"
                            "  rttt  rrt wwwwtttwwwwwwwrrtwwwtttwwwwwwrrrrrttt   trrrt  rrrrrtt   rrrrtttt  \n"
                            "        rrtwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwtt        tt      tttt       tt    \n"
                            "       rttwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                 \n"
                            "     rtt  wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                 \n"
                            "    tt    wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                 \n"
                            "          wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                \n"
                            "          wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                \n"
                            "           wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                \n"
                            "            wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                  \n"
                            "             wwwwwwwwwwwwwwwwwwwwwwwwwwwwyywww                                 \n"
                            "              wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                 \n"
                            "               wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww                                \n"
                            "                  wwwwwwwwwwwwwwwwwwwwwwwwwwww                                 \n"
                            "                       wwwwwwwwwwwwwwwwwwww                         ewe we wwee\n";

    move(0,0);
    for (unsigned int i = 0; i < youDied.length(); i++) {
        if (supportsColor) {
            if (colorField[i] == 'w') {
                attron(COLOR_PAIR(Color::White));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::White));
            }
            else if (colorField[i] == 'e') {
                attron(COLOR_PAIR(Color::Cyan));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::Cyan));
            }
            else if (colorField[i] == 'r') {
                attron(COLOR_PAIR(Color::Red));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::Red));
            }
            else if (colorField[i] == 't') {
                attron(COLOR_PAIR(Color::Magenta));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::Magenta));
            }
            else if (colorField[i] == 'y') {
                attron(COLOR_PAIR(Color::Yellow));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::Yellow));
            }
            else {
                attron(COLOR_PAIR(Color::Green));
                addch(youDied[i]);
                attroff(COLOR_PAIR(Color::Green));
            }
        }
        else {
            addch(youDied[i]);
        }
    }
        

    while (getch() != 'Q')
        ;
}

void winScreen(bool supportsColor) {
    clear();

    std::string youWin =    "  ..                                   x=~              @88>               \n"
                            " @L                u.    x.    .      88x.   .e.   .e.   %8P     u.    u.  \n"
                            "9888i   .dL  ...ue888b .@88k  z88u   '8888X.x888:.x888    .    x@88k u@88c.\n"
                            "`Y888k:*888. 888R Y888r\"8888 ^8888    `8888  888X '888k .@88u ^\"8888\"\"8888\"\n"
                            "  888E  888I 888R I888> 8888  888R     X888  888X  888X''888E`  8888  888R \n"
                            "  888E  888I 888R I888> 8888  888R     X888  888X  888X  888E   8888  888R \n"
                            "  888E  888I 888R I888> 8888  888R+:;:.X888  888X  888X  888E   8888  888R \n"
                            "  888E  888Iu8888cJ888. 8888 ,888BX.+;.X888xx888X.;888~  888E   8888  888R \n"
                            " x888N><888' \"*888*P\";.\"8888Y 8888\"x;.`%88%::\"*888Y\";.   888&  \"*88*\" 8888\"\n"
                            "  \"88\". 888    'Y\"xxx;.:`Y\"   'YP;+;:.  `~.xx;.`\"x+;.    R888\" .:\"\"   'Y\"  \n"
                            "  .;+;;;88F:..   .x++;..:+:.    .;;::.    .;;..++;;:    .:.. .;Xx::.       \n"
                            "  .;XXx98\"+x+:   .x+;;;.:;;.   .;;:::..  .:;:.:+;;;:    ;x+;..xXx+;.       \n"
                            "     ../\"..:+X+. :++;;;;;;;:.  ;;;::.:.  :;::;;;;;;; ..:;:..:xXx;.         \n"
                            "     ~`:;x;..+Xx;x+;;;::;;;;::;+;:::..:.;;;;+;::;;;;;++;. .+XXx.           \n"
                            "       .:;+x;;xXX++;;;:::;+++++;;+XX+....::::::;;;;;+++;;+XxXX:            \n"
                            "        .;++xxxxxx++X$x;::;;++;;+$X++:.......:x$Xx;::;+++xxXX:             \n"
                            "         .++xxXXXx+x$$;+;:;;++;;x$X++;......:+$$+;+::::;++xXx.             \n"
                            "         ;X$xxXXXx+x&$XX+:;;++;;+$$$$+....:::+$$$$;;:::;+xX$+.             \n"
                            "         x+$$xxXXxx+X&$X;;;++++;;xX$x::::::;;+X$$x;;::;++x$Xx.             \n"
                            "         ;$$$xxXXXx++++;;;;++++++;;;;;;;;;;++++++;;;;;++xx$$x.             \n"
                            "         .+Xx+xXXXxxx+++++++++++++++++;;;;;;;++++++;;+++xxx+.              \n"
                            "           .;+++++xxxxxxxxXXXXXXXXXXXXXXXXXXxxxx++xx+++++x;.               \n"
                            "          .;+xX$$$$XXXXXXXXXXXXXXXXXXXXXXXXX$$$$$$$$$$$$Xx+.               \n"
                            "           :+XXXXXXXXXXxxxxxx+++++xxxxxxXXXXXXXX$$$$$$$$Xx;.    'Q' to quit\n";

    std::string colorField ="  rr                                   rrr              rrrt               \n"
                            " rr                rr    rr    r      rrrr   rrr   rrr   rtt     rr    rr  \n"
                            "rrrrr   rrr  rrrrrrrrr rrrrr  rrrr   rrrrrrttrrrrttrrr    r    rrrrr rrrrrr\n"
                            "rrrrrrttrrrr rrrr rrrrrrrrrr rrrrr    rrrrr  rrrr trrrr rrrrr rrrrrrrrrrrrr\n"
                            "  rrrr  rrrr rrrr rrrrt rrrr  rrrr     rrrr  rrrr  rrrrrrrrrrr  rrrr  rrrr \n"
                            "  rrrr  rrrr rrrr rrrrt rrrr  rrrr     rrrr  rrrr  rrrr  rrrr   rrrr  rrrt \n"
                            "  rrrr  rrrr rrrr rrrrt rrrr  rrrtwwwwwrrrr  rrrt  rrrt  rrrr   rrrr  rrrt \n"
                            "  rrrr  rrrtrrrrrrrrrtw rrrr rrrrtwtwwrrrrrrrrrrttwrrrt  rrrr   rrrt  rrrt \n"
                            " rrrrrrrrrrt rrrrrtttwwrrrrrt rrrttwwwrrrrrwwtrrrrttww   rrrt  rrrrtt rrrtt\n"
                            "  rtttw rrt    tttwwwwwwttt   tttwwwww  ttwwwwwttwwww    rrrrt wwtt   ttt  \n"
                            "  wwwwwwrrtwww   wwwwwwwwwww    wwwwww    wwwwwwwwww    wwww wwwwwww       \n"
                            "  wwwwwrrtwwww   wwwwwwwwwww   wwwwwwww  wwwwwwwwwww    wwwwwwwwwwww       \n"
                            "     wrttwwwwwww wwwwwwwwwwww  wwwwwwww wwwwwwwwwwww wwwwwwwwwwwww         \n"
                            "     ttwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww wwwww           \n"
                            "       wwwwwwwwwwwwwwwwwwwwwwwwwweeeewwwwwwwwwwwwwwwwwwwwwwwwww            \n"
                            "        wwwwwwwwwwwweeeewwwwwwwweeeeeewwwwwwwweeeewwwwwwwwwwww             \n"
                            "         wwwwwwwwwweeeeewwwwwwwweeeeeewwwwwwwweeeeewwwwwwwwwww             \n"
                            "         weewwwwwwweeeeeewwwwwwweeeeeewwwwwwwweeeeewwwwwwweeww             \n"
                            "         eeeewwwwwwweeeewwwwwwwwweeeewwwwwwwwweeeeewwwwwweeeew             \n"
                            "         weeewwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweeeew             \n"
                            "         weeewwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwweew              \n"
                            "           wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww               \n"
                            "          wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww               \n"
                            "           wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww    yuy yu yuuy\n";

    move(0, 0);
    for (unsigned int i = 0; i < youWin.length(); i++) {
        if (supportsColor) {
            if (colorField[i] == 'w') {
                attron(COLOR_PAIR(Color::Yellow));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::Yellow));
            }
            else if (colorField[i] == 'e') {
                attron(COLOR_PAIR(Color::Red));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::Red));
            }
            else if (colorField[i] == 'r') {
                attron(COLOR_PAIR(Color::Green));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::Green));
            }
            else if (colorField[i] == 't') {
                attron(COLOR_PAIR(Color::Blue));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::Blue));
            }
            else if (colorField[i] == 'y') {
                attron(COLOR_PAIR(Color::Cyan));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::Cyan));
            }
            else if (colorField[i] == 'u') {
                attron(COLOR_PAIR(Color::White));
                addch(youWin[i]);
                attroff(COLOR_PAIR(Color::White));
            }
            else {
                attron(COLOR_PAIR(Color::Magenta));
                addch(colorField[i]);
                attroff(COLOR_PAIR(Color::Magenta));
            }
        }
        else {
            addch(youWin[i]);
        }
    }
  
    while (getch() != 'Q')
        ;
}
