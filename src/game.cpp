#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>
#include <memory>

#include "display.hpp"
#include "dungeon.hpp"
#include "fibonacciHeap.hpp"
#include "game.hpp"
#include "pathFinding.hpp"

static bool fogOfWarToggle = true;

int checkCorridor(int x, int y, bool visited[MAX_HEIGHT][MAX_WIDTH]) {
    if (x < 0 || x >= MAX_WIDTH || y < 0 || y >= MAX_HEIGHT) {
        return 0;
    }
    else if ((visited[y][x] || dungeon[y][x].type != CORRIDOR)) {
        return 0;
    }
    else if (player.getPos().x == x && player.getPos().y == y) {
        return 1;
    }
    visited[y][x] = true;

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

bool inLineOfSight(Pos pos) {
    int xDist = abs(player.getPos().x - pos.x);
    int yDist = abs(player.getPos().y - pos.y);  
    if (xDist <= 2 && yDist <= 2 && !(xDist == 2 && yDist == 2)) {
        return true;
    }
    return false;
}

void updateAroundPlayer() {
    for (int i = player.getPos().y - 2; i <= player.getPos().y + 2; i++) {
        for (int j = player.getPos().x - 2; j <= player.getPos().x + 2; j++) {
            if (((i == player.getPos().y - 2 || i == player.getPos().y + 2) && (j == player.getPos().x - 2 || j == player.getPos().x + 2)) ||
                (i < 0 || i >= MAX_HEIGHT || j < 0 || j >= MAX_WIDTH)) {
                continue;
                }
            dungeon[i][j].visible = dungeon[i][j].type;
        }
    }
}

int playGame(int numMonsters, int numObjects, bool autoFlag, bool godmodeFlag, bool supportsColor) {
    int time = 0;
    int monstersAlive = numMonsters;
    std::unique_ptr<FibHeap> heap = std::make_unique<FibHeap>();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                insert(heap.get(), 1000 / monsterAt[i][j].get()->getSpeed(), monsterAt[i][j].get()->getPos());
            }
            
            dungeon[i][j].visible = FOG;
        }
    }
    insert(heap.get(), 1, player.getPos());

    if (autoFlag) {
        fogOfWarToggle = false;
    }
     
    while (1) {
        FibNode *node = extractMin(heap.get());
        time = node->key;

        if (node->pos.x == player.getPos().x && node->pos.y == player.getPos().y) {
            updateAroundPlayer();
            printDungeon(supportsColor, fogOfWarToggle);
            bool turnEnd = false;
            while (!turnEnd) {
                int ch = 0;
                int xDir = 0;
                int yDir = 0;
                if (autoFlag) {
                    ch = getch();
                    if (ch == 'Q') {
                        printLine(MESSAGE_LINE, "Goodbye!");
                        napms(1000);
                        
                        clearAll();
                        return 0;
                    }

                    xDir = rand() % 3 - 1;
                    yDir = rand() % 3 - 1;
                    napms(500);
                    turnEnd = true;
                }
                else {
                    ch = getch();
                    switch (ch) {
                        case KEY_HOME:
                        case '7':
                        case 'y':
                            xDir = -1;
                            yDir = -1;
                            turnEnd = true;
                            break;

                        case KEY_UP:
                        case '8':
                        case 'k':
                            xDir = 0;
                            yDir = -1;
                            turnEnd = true;
                            break;
                        
                        case KEY_PPAGE:
                        case '9':
                        case 'u':
                            xDir = 1;
                            yDir = -1;
                            turnEnd = true;
                            break;

                        case KEY_RIGHT:
                        case '6':
                        case 'l':
                            xDir = 1;
                            yDir = 0;
                            turnEnd = true;
                            break;

                        case KEY_NPAGE:
                        case '3':
                        case 'n':
                            xDir = 1;
                            yDir = 1;
                            turnEnd = true;
                            break;

                        case KEY_DOWN:
                        case '2':
                        case 'j':
                            xDir = 0;
                            yDir = 1;
                            turnEnd = true;
                            break;

                        case KEY_END:
                        case '1':
                        case 'b':
                            xDir = -1;
                            yDir = 1;
                            turnEnd = true;
                            break;

                        case KEY_LEFT:
                        case '4':
                        case 'h':
                            xDir = -1;
                            yDir = 0;
                            turnEnd = true;
                            break;

                        case KEY_B2:
                        case ' ':
                        case '.':
                        case '5':
                            turnEnd = true;
                            break;

                        case '>':
                            if (dungeon[player.getPos().y][player.getPos().x].type == STAIR_DOWN) {
                                printLine(MESSAGE_LINE, "Going down stairs...");
                                napms(1000);

                                for (int i = 0; i < MAX_HEIGHT; i++) {
                                    for (int j = 0; j < MAX_WIDTH; j++) {
                                        if (monsterAt[i][j]) {
                                            if (monsterAt[i][j].get()->isBoss() || monsterAt[i][j].get()->isUnique()) {
                                                monsterTypeList[monsterAt[i][j].get()->getMonTypeIndex()].eligible = true;
                                            }
                                        }
                                        for (const auto& obj : objectsAt[i][j]) {
                                            if (obj->isArtifact()) {
                                                objectTypeList[obj->getObjTypeIndex()].eligible = true;
                                            }
                                        }
                                    }
                                }

                                clearAll();

                                clear();
                                initDungeon();
                                generateStructures();
                                player.setPos((Pos){upStairs.back().x, upStairs.back().y});
                                spawnMonsters(numMonsters, player.getPos().x, player.getPos().y);
                                spawnObjects(numObjects);
                                
                                return 1;
                            }
                            else {
                                printLine(MESSAGE_LINE, "There are no stairs down here.");
                            }
                            break;

                        case '<':
                            if (dungeon[player.getPos().y][player.getPos().x].type == STAIR_UP) {
                                printLine(MESSAGE_LINE, "Going up stairs...");
                                napms(1000);

                                for (int i = 0; i < MAX_HEIGHT; i++) {
                                    for (int j = 0; j < MAX_WIDTH; j++) {
                                        if (monsterAt[i][j]) {
                                            if (monsterAt[i][j].get()->isBoss() || monsterAt[i][j].get()->isUnique()) {
                                                monsterTypeList[monsterAt[i][j].get()->getMonTypeIndex()].eligible = true;
                                            }
                                        }
                                        for (const auto& obj : objectsAt[i][j]) {
                                            if (obj->isArtifact()) {
                                                objectTypeList[obj->getObjTypeIndex()].eligible = true;
                                            }
                                        }
                                    }
                                }

                                clearAll();

                                clear();
                                initDungeon();
                                generateStructures();
                                player.setPos((Pos){downStairs.back().x, downStairs.back().y});
                                spawnMonsters(numMonsters, player.getPos().x, player.getPos().y);
                                spawnObjects(numObjects);

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
                            printLine(MESSAGE_LINE, "Choose an inventory slot 0-9");
                            {
                                int ch = getch();
                                if (ch >= '0' && ch <= '9') {
                                    int index = ch - '0';
                                    if (player.getInventoryItem(index) == nullptr) {
                                        printLine(MESSAGE_LINE, "Nothing in that slot.");
                                        break;
                                    }
                                    std::string itemName = player.getInventoryItem(index)->getName();
                                    player.dropFromInventory(index);
                                    printLine(MESSAGE_LINE, "%s has been dropped.", itemName.c_str());
                                }
                                else if (ch == 'd' || ch == 27) {
                                    printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
                                }
                                else {
                                    printLine(MESSAGE_LINE, "Not a valid slot.");
                                }
                            }
                            break;

                        case 'e':
                            openEquipment(supportsColor, fogOfWarToggle);
                            break;
                        
                        case 'f':
                            fogOfWarToggle = !fogOfWarToggle;
                            printDungeon(supportsColor, fogOfWarToggle);
                            {
                                const char* fogStatus = fogOfWarToggle ? "on" : "off";
                                printLine(STATUS_LINE1, "Fog of war toggled %s", fogStatus);
                            }
                                
                            break;

                        case 'g':
                            {
                                int replaceFogOfWar = fogOfWarToggle;
                                fogOfWarToggle = false;
                                printDungeon(supportsColor, fogOfWarToggle);

                                bool drop = false;
                                int x = player.getPos().x;
                                int y = player.getPos().y;
                                while (!drop) {
                                    int oldX = x;
                                    int oldY = y;
                                    mvaddch(y + 1, x, '!');
                                    refresh();
                            
                                    int ch;
                                    ch = getch();
                                    printLine(MESSAGE_LINE, "Use movement keys to move and 'g' to finalize, or 'r' to be placed randomly.");
                                    switch (ch) {
                                        case 'r':
                                            x = rand() % (MAX_WIDTH - 2) + 1;
                                            y = rand() % (MAX_HEIGHT - 2) + 1;
                                            drop = true;
                                            break;
                            
                                        case 'g':
                                            drop = true;
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
                                            break;
                                    }
                                    if (player.getPos().x == oldX && player.getPos().y == oldY) {
                                        mvaddch(oldY + 1, oldX, '@');
                                    }
                                    else if (monsterAt[oldY][oldX]) {
                                        if (supportsColor) {
                                            Color c = monsterAt[oldY][oldX].get()->getColor();
                                            attron(COLOR_PAIR(c));
                                            mvaddch(oldY + 1, oldX, monsterAt[oldY][oldX].get()->getSymbol());
                                            attroff(COLOR_PAIR(c));
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, monsterAt[oldY][oldX].get()->getSymbol());
                                        }
                                    }
                                    else if (objectsAt[oldY][oldX].size() > 0) {
                                        if (supportsColor) {
                                            Color c = objectsAt[oldY][oldX].back()->getColor();
                                            attron(COLOR_PAIR(c));
                                            mvaddch(oldY + 1, oldX, objectsAt[oldY][oldX].back()->getSymbol());
                                            attroff(COLOR_PAIR(c));
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, objectsAt[oldY][oldX].back()->getSymbol());
                                        }
                                    }
                                    else {
                                        if (inLineOfSight((Pos){oldX, oldY})) {
                                            if (supportsColor) {
                                                attron(COLOR_PAIR(Color::Yellow));
                                                mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].visible);
                                                attroff(COLOR_PAIR(Color::Yellow));
                                            }
                                            else {
                                                mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].visible);
                                            }
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].type);
                                        }
                                    }
                                }
                            
                                if (dungeon[y][x].type == ROCK) {
                                    dungeon[y][x].hardness = 0;
                                    dungeon[y][x].type = CORRIDOR;
                                }

                                player.setPos((Pos){x, y});
                                mvaddch(y + 1, x, '@');

                                if (monsterAt[player.getPos().y][player.getPos().x]) {
                                    Monster *mon = monsterAt[player.getPos().y][player.getPos().x].get();
                                    if (mon->isBoss()) {
                                        printDungeon(supportsColor, fogOfWarToggle);
                                        printLineColor(STATUS_LINE1, Color::Green, supportsColor, "%s has been slain!\n", mon->getName().c_str());
                                        printLine(STATUS_LINE2, "You win! Press any key to continue...");
                                        getch();
                                        winScreen(supportsColor);
                                        
                                        clearAll();
                                        return 0;
                                    }
                                    monstersAlive--;

                                    std::unique_ptr<FibHeap> tempHeap = std::make_unique<FibHeap>();
                                    FibNode *tempNode = extractMin(heap.get());
                                    while (tempNode->pos.x != mon->getPos().x || tempNode->pos.y != mon->getPos().y) {
                                        insert(tempHeap.get(), tempNode->key, tempNode->pos);
                                        tempNode = extractMin(heap.get());
                                    }
                                    while (tempHeap.get()->min) {
                                        tempNode = extractMin(tempHeap.get());
                                        insert(heap.get(), tempNode->key, tempNode->pos);
                                    }

                                    printLine(STATUS_LINE1, "Player stomped %s", mon->getName().c_str());
                                    monsterAt[player.getPos().y][player.getPos().x] = nullptr;
                                }
                                fogOfWarToggle = replaceFogOfWar;
                                updateAroundPlayer();
                                printDungeon(supportsColor, fogOfWarToggle);
                            }
                            break;
                        
                        case 'i':
                            openInventory(supportsColor, fogOfWarToggle);
                            break;

                        case 'm':
                            monsterList(supportsColor, fogOfWarToggle);
                            break;

                        case 'o':
                            objectList(supportsColor, fogOfWarToggle);
                            break;

                        case 's':
                            // "display the default (terrain) map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 't':
                            printLine(MESSAGE_LINE, "Choose an equipment slot a-l");
                            {
                                int ch = getch();
                                if (ch >= 'a' && ch <= 'l') {
                                    int index = ch - 'a';
                                    if (player.getEquipmentItem((Equip)index) == nullptr) {
                                        printLine(MESSAGE_LINE, "Nothing in that slot.");
                                        break;
                                    }
                                    std::string itemName = player.getEquipmentItem((Equip)index)->getName();
                                    if (player.unequip((Equip)index)) {
                                        printLine(MESSAGE_LINE, "%s has been unequipped.", itemName.c_str());
                                    }
                                    else {
                                        printLine(MESSAGE_LINE, "Inventory is full. Cannot unequip %s.", itemName.c_str());
                                    }
                                }
                                else if (ch == 't' || ch == 27) {
                                    printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
                                }
                                else {
                                    printLine(MESSAGE_LINE, "Not a valid slot.");
                                }
                            }
                            break;

                        case 'w':
                            printLine(MESSAGE_LINE, "Choose an inventory slot 0-9");
                            {
                                int ch = getch();
                                if (ch >= '0' && ch <= '9') {
                                    int index = ch - '0';
                                    if (player.getInventoryItem(index) == nullptr) {
                                        printLine(MESSAGE_LINE, "Nothing in that slot.");
                                        break;
                                    }
                                    std::string itemName = player.getInventoryItem(index)->getName();
                                    if (player.getInventoryItem(index)->isTwoHanded()) {
                                        if (player.inventoryFull() && player.getEquipmentItem(Equip::Weapon) != nullptr &&
                                            player.getEquipmentItem(Equip::Offhand) != nullptr) {
                                            printLine(MESSAGE_LINE, "Inventory and weapon slots full. Cannot equip two-handed weapon.");
                                            break;
                                        }
                                        else if (player.getEquipmentItem(Equip::Offhand) != nullptr) {
                                            player.unequip(Equip::Offhand);
                                            player.swapEquipment(index);
                                            printLine(MESSAGE_LINE, "Equipped two-handed weapon and unequipped offhand weapon.");
                                            break;
                                        }
                                        else {
                                            player.swapEquipment(index);
                                        }
                                    }
                                    else if (player.getInventoryItem(index)->getEquipmentIndex() == Equip::Offhand) {
                                        if (player.getEquipmentItem(Equip::Weapon) != nullptr) {
                                            if (player.getEquipmentItem(Equip::Weapon)->isTwoHanded()) {
                                                player.unequip(Equip::Weapon);
                                                player.equip(index);
                                                printLine(MESSAGE_LINE, "Equipped offhand weapon and unequipped two-handed weapon.");
                                                break;
                                            }
                                            else {
                                                player.swapEquipment(index);
                                            }
                                        }
                                        else {
                                            player.swapEquipment(index);
                                        }
                                    }
                                    else if (player.getInventoryItem(index)->getEquipmentIndex() == Equip::Ring1 || 
                                            player.getInventoryItem(index)->getEquipmentIndex() == Equip::Ring2) {
                                        if (player.getEquipmentItem(Equip::Ring1) == nullptr) {
                                            player.getInventoryItem(index)->setEquipmentIndex(Equip::Ring1);
                                            player.equip(index);
                                        }
                                        else if (player.getEquipmentItem(Equip::Ring2) == nullptr) {
                                            player.getInventoryItem(index)->setEquipmentIndex(Equip::Ring2);
                                            player.equip(index);
                                        }
                                        else {
                                            printLine(MESSAGE_LINE, "Ring slots full. Replace ring (1) or (2)?");
                                            char ch = getch();
                                            if (ch == '1') {
                                                player.getInventoryItem(index)->setEquipmentIndex(Equip::Ring1);
                                                player.swapEquipment(index);
                                            }
                                            else if (ch == '2') {
                                                player.getInventoryItem(index)->setEquipmentIndex(Equip::Ring2);
                                                player.swapEquipment(index);
                                            }
                                        }
                                    }
                                    else if (player.getInventoryItem(index)->getEquipmentIndex() != Equip::None) {
                                        player.swapEquipment(index);
                                    }
                                    else {
                                        break;
                                    }
                                    printLine(MESSAGE_LINE, "%s is now equipped.", itemName.c_str());
                                }
                                else if (ch == 'w' || ch == 27) {
                                    printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
                                }
                                else {
                                    printLine(MESSAGE_LINE, "Not a valid slot.");
                                }
                            }
                            break;

                        case 'x':
                            printLine(MESSAGE_LINE, "Choose an inventory slot 0-9");
                            {
                                int ch = getch();
                                if (ch >= '0' && ch <= '9') {
                                    int index = ch - '0';
                                    if (player.getInventoryItem(index) == nullptr) {
                                        printLine(MESSAGE_LINE, "Nothing in that slot.");
                                        break;
                                    }
                                    std::string itemName = player.getInventoryItem(index)->getName();
                                    player.expungeFromInventory(index);
                                    printLine(MESSAGE_LINE, "%s has been obliterated.", itemName.c_str());
                                }
                                else if (ch == 'd' || ch == 27) {
                                    printLine(MESSAGE_LINE, "Press a key to continue... or press '?' for help."); 
                                }
                                else {
                                    printLine(MESSAGE_LINE, "Not a valid slot.");
                                }
                            }
                            break;

                        case 'D':
                            tunnelingDistMap(supportsColor, fogOfWarToggle);
                            break;
                        
                        case 'E':
                            showEquipmentObjectDescription(supportsColor, fogOfWarToggle);
                            break;

                        case 'H':
                            // "display the hardness map"
                            printLine(MESSAGE_LINE, "Action for %c Not implemented yet!", (char) ch);
                            break;

                        case 'I':
                            showInventoryObjectDescription(supportsColor, fogOfWarToggle);
                            break;

                        case 'L':
                            {
                                bool view = false;
                                bool escape = false;
                                int x = player.getPos().x;
                                int y = player.getPos().y;
                                while (!view) {
                                    int oldX = x;
                                    int oldY = y;
                                    mvaddch(y + 1, x, '!');
                                    refresh();

                                    int ch;
                                    ch = getch();
                                    printLine(MESSAGE_LINE, "Use movement keys to move and 't' to finalize, or use 'Esc' or 'L'  exit.");
                                    switch (ch) {
                                        case 't':
                                            if (fogOfWarToggle && !inLineOfSight((Pos){x, y})) {
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (monsterAt[y][x]) {
                                                view = true;
                                            }
                                            else {
                                                printLine(MESSAGE_LINE, "There is nothing to view there.");
                                            }
                                            break;

                                        case KEY_HOME:
                                        case '7':
                                        case 'y':
                                            x -= 1;
                                            if (fogOfWarToggle && x < player.getPos().x - 2) {
                                                x++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y -= 1;
                                            if (fogOfWarToggle && y < player.getPos().y - 2) {
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x - 2 && y == player.getPos().y - 2) {
                                                x++;
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_UP:
                                        case '8':
                                        case 'k':
                                            y -= 1;
                                            if (fogOfWarToggle && y < player.getPos().y - 2) {
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && (x == player.getPos().x - 2 || x == player.getPos().x + 2) && y == player.getPos().y - 2) {
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                                        
                                        case KEY_PPAGE:
                                        case '9':
                                        case 'u':
                                            x += 1;
                                            if (fogOfWarToggle && x > player.getPos().x + 2) {
                                                x--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            } 
                                            else if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y -= 1;
                                            if (fogOfWarToggle && y < player.getPos().y - 2) {
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == 0) {
                                                y++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x + 2 && y == player.getPos().y - 2) {
                                                x--;
                                                y++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_RIGHT:
                                        case '6':
                                        case 'l':
                                            x += 1;
                                            if (fogOfWarToggle &&  x > player.getPos().x + 2) {
                                                x--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x + 2 && (y == player.getPos().y - 2 || y == player.getPos().y + 2)) {
                                                x--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_NPAGE:
                                        case '3':
                                        case 'n':
                                            x += 1;
                                            if (fogOfWarToggle && x > player.getPos().x + 2) {
                                                x--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (x == MAX_WIDTH - 1) {
                                                x--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y += 1;
                                            if (fogOfWarToggle && y > player.getPos().y + 2) {
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x + 2 && y == player.getPos().y + 2) {
                                                x--;
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_DOWN:
                                        case '2':
                                        case 'j':
                                            y += 1;
                                            if (fogOfWarToggle && y > player.getPos().y + 2) {
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && (x == player.getPos().x + 2 || x == player.getPos().x - 2) && y == player.getPos().y + 2) {
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_END:
                                        case '1':
                                        case 'b':
                                            x -= 1;
                                            if (fogOfWarToggle && x < player.getPos().x - 2) {
                                                x++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            y += 1;
                                            if (fogOfWarToggle && y > player.getPos().y + 2) {
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (y == MAX_HEIGHT - 1) {
                                                y--;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x - 2 && y == player.getPos().y + 2) {
                                                x++;
                                                y--;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;
                            
                                        case KEY_LEFT:
                                        case '4':
                                        case 'h':
                                            x -= 1;
                                            if (fogOfWarToggle && x < player.getPos().x - 2) {
                                                x++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            else if (x == 0) {
                                                x++;
                                                printLine(MESSAGE_LINE, "That's an impenetrable wall.");
                                            }
                                            if (fogOfWarToggle && x == player.getPos().x - 2 && (y == player.getPos().y + 2 || y == player.getPos().y - 2)) {
                                                x++;
                                                printLine(MESSAGE_LINE, "You cannot see that far.");
                                            }
                                            break;

                                        case 'L':
                                        case 27:
                                            escape = true;
                                            break;
                            
                                        default:
                                            break;
                                    }
                                    if (player.getPos().x == oldX && player.getPos().y == oldY) {
                                        mvaddch(oldY + 1, oldX, '@');
                                    }
                                    else if (monsterAt[oldY][oldX]) {
                                        if (supportsColor) {
                                            Color c = monsterAt[oldY][oldX].get()->getColor();
                                            attron(COLOR_PAIR(c));
                                            mvaddch(oldY + 1, oldX, monsterAt[oldY][oldX].get()->getSymbol());
                                            attroff(COLOR_PAIR(c));
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, monsterAt[oldY][oldX].get()->getSymbol());
                                        }
                                    }
                                    else if (objectsAt[oldY][oldX].size() > 0) {
                                        if (supportsColor) {
                                            Color c = objectsAt[oldY][oldX].back()->getColor();
                                            attron(COLOR_PAIR(c));
                                            mvaddch(oldY + 1, oldX, objectsAt[oldY][oldX].back()->getSymbol());
                                            attroff(COLOR_PAIR(c));
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, objectsAt[oldY][oldX].back()->getSymbol());
                                        }
                                    }
                                    else {
                                        if (inLineOfSight((Pos){oldX, oldY})) {
                                            if (supportsColor) {
                                                attron(COLOR_PAIR(Color::Yellow));
                                                mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].visible);
                                                attroff(COLOR_PAIR(Color::Yellow));
                                            }
                                            else {
                                                mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].visible);
                                            }
                                        }
                                        else {
                                            mvaddch(oldY + 1, oldX, dungeon[oldY][oldX].type);
                                        }
                                    }

                                    if (escape) {
                                        break;
                                    }
                                }
                                if (escape) {
                                    printDungeon(supportsColor, fogOfWarToggle);
                                    break;
                                }
                                showMonsterInfo((Pos){x, y}, supportsColor, fogOfWarToggle);
                            }
                            break;

                        case 'Q':
                            printLine(MESSAGE_LINE, "Goodbye!");
                            napms(1000);

                            clearAll();
                            return 0;

                        case 'T':
                            nonTunnelingDistMap(supportsColor, fogOfWarToggle);
                            break;

                        case ',':
                            if (!objectsAt[player.getPos().y][player.getPos().x].empty()) {
                                std::string itemName = objectsAt[player.getPos().y][player.getPos().x].back()->getName();
                                bool added = player.addToInventory(player.getPos());
                                if (added) {
                                    printLine(MESSAGE_LINE, "Picked up %s.", itemName.c_str()); 
                                }
                                else {
                                    printLine(MESSAGE_LINE, "Inventory full. Cannot pick up %s.", itemName.c_str());
                                }                                
                            }
                            else {
                                printLine(MESSAGE_LINE, "Nothing to pick up here.");
                            }
                            break;

                        case '?':
                            commandList(supportsColor, fogOfWarToggle);
                            break;

                        default:
                            printLine(MESSAGE_LINE, "Invalid key... press '?' for help.");
                            break;
                            
                    }
                }
                
                if (xDir == 0 && yDir == 0 && !(ch == KEY_B2 || ch == ' ' || ch == '.' || ch == '5')) {
                    turnEnd = false;
                }
                else if (dungeon[player.getPos().y + yDir][player.getPos().x + xDir].hardness == 0) {
                    if (monsterAt[player.getPos().y + yDir][player.getPos().x + xDir]) {
                        Monster *mon = monsterAt[player.getPos().y + yDir][player.getPos().x + xDir].get();

                        int dam = player.doDamage();
                        int hpLeft = mon->takeDamage(dam);

                        if (hpLeft <= 0) {
                            if (mon->isBoss()) {
                                printDungeon(supportsColor, fogOfWarToggle);
                                printLine(STATUS_LINE1, "%s has been slain!\n", mon->getName().c_str());
                                printLine(STATUS_LINE2, "You win! Press any key to continue...");
                                getch();
                                winScreen(supportsColor);
                                
                                clearAll();
                                return 0;
                            }
                            monstersAlive--;

                            std::unique_ptr<FibHeap> tempHeap = std::make_unique<FibHeap>();
                            FibNode *tempNode = extractMin(heap.get());
                            while (tempNode->pos.x != mon->getPos().x || tempNode->pos.y != mon->getPos().y) {
                                insert(tempHeap.get(), tempNode->key, tempNode->pos);
                                tempNode = extractMin(heap.get());
                            }
                            while (tempHeap.get()->min) {
                                tempNode = extractMin(tempHeap.get());
                                insert(heap.get(), tempNode->key, tempNode->pos);
                            }

                            printLine(STATUS_LINE1, "%s has been slain.\n", mon->getName().c_str());
                            monsterAt[player.getPos().y + yDir][player.getPos().x + xDir] = nullptr;
                        }
                        else {
                            printLineColor(STATUS_LINE1, Color::Green, supportsColor, "You dealt %d damage to %s.\n", dam, mon->getName().c_str());
                            napms(500);
                            flushinp();
                        }
                    }
                    else {
                        printLine(STATUS_LINE1, "");
                        player.setPos((Pos){player.getPos().x + xDir, player.getPos().y + yDir});
                    }
                    insert(heap.get(), time + 1000 / player.getSpeed(), player.getPos());
                }
                else {
                    printLine(MESSAGE_LINE, "There's a wall there, adventurer!");
                    turnEnd = false;
                }
            }
        }
        else {
            Monster *mon = monsterAt[node->pos.y][node->pos.x].get();
            int x = mon->getPos().x;
            int y = mon->getPos().y;

            int directions[8][2] = {
                {-1, 1},  {0, 1},  {1, 1},
                {-1, 0},           {1, 0},
                {-1, -1}, {0, -1}, {1, -1}};
            bool sameRoom = false;
            for (int i = 0; i < roomCount; i++) {
                if (x >= rooms[i].getPos().x && x <= rooms[i].getPos().x + rooms[i].getWidth() - 1 &&
                    y >= rooms[i].getPos().y && y <= rooms[i].getPos().y + rooms[i].getHeight() - 1 &&
                    player.getPos().x >= rooms[i].getPos().x && player.getPos().x <= rooms[i].getPos().x + rooms[i].getWidth() - 1 &&
                    player.getPos().y >= rooms[i].getPos().y && player.getPos().y <= rooms[i].getPos().y + rooms[i].getHeight() - 1) {
                    sameRoom = true;
                    break;
                }
            }

            bool visited[MAX_HEIGHT][MAX_WIDTH] = {false};
            int sameCorridor = checkCorridor(x, y, visited);

            bool hasLastSeen = (mon->getLastSeen().x != -1 && mon->getLastSeen().y != -1);
            bool canSee = (mon->isTelepathic() || sameRoom || sameCorridor);
            if (sameRoom || sameCorridor) {
                mon->setLastSeen(player.getPos());
            }

            int newX = x;
            int newY = y;
            if (mon->isErratic() && rand() % 2) {
                for (int i = 0; i < ATTEMPTS; i++) {
                    int dir = rand() % 8;
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
                    generateDistances(player.getPos());
                }
                else {
                    generateDistances(mon->getLastSeen());
                }
                
                if (mon->isIntelligent()) {
                    int minDist = UNREACHABLE;
                    int possibleDir[8] = {0};
                    int numPossible = 0;
                    for (int i = 0; i < 8; i++) {
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
                        targetX = player.getPos().x;
                        targetY = player.getPos().y;
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
                        insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                        continue;
                    }
                }
            } 
            else {
                insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                continue;
            }

            if (dungeon[newY][newX].type == ROCK) {
                if (dungeon[newY][newX].hardness > 85) {
                    dungeon[newY][newX].hardness -= 85;
                    insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                }
                else {
                    dungeon[newY][newX].hardness = 0;
                    dungeon[newY][newX].type = CORRIDOR;

                    mon->setPos((Pos){newX, newY});
                    monsterAt[newY][newX] = std::move(monsterAt[y][x]);

                    insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                }
            }
            else {
                if (monsterAt[newY][newX]) {
                    Monster* monDisplace = monsterAt[newY][newX].get();
                    int possibleDir[8] = {0};
                    int numPossible = 0;
                    for (int i = 0; i < 8; i++) {
                        int displaceX = newX + directions[i][0];
                        int displaceY = newY + directions[i][1];
                        if (((dungeon[displaceY][displaceX].type == CORRIDOR || dungeon[displaceY][displaceX].type == FLOOR ||
                            dungeon[displaceY][displaceX].type == STAIR_UP || dungeon[displaceY][displaceX].type == STAIR_DOWN) &&
                            !monsterAt[displaceY][displaceX] && (displaceX != player.getPos().x || displaceY != player.getPos().y))) {
                            numPossible++;
                            possibleDir[numPossible - 1] = i;
                        }
                    }

                    std::unique_ptr<FibHeap> tempHeap = std::make_unique<FibHeap>();
                    FibNode *tempNode = extractMin(heap.get());
                    while (tempNode->pos.x != monDisplace->getPos().x || tempNode->pos.y != monDisplace->getPos().y) {
                        insert(tempHeap.get(), tempNode->key, tempNode->pos);
                        tempNode = extractMin(heap.get());
                    }
                    while (tempHeap.get()->min) {
                        tempNode = extractMin(tempHeap.get());
                        insert(heap.get(), tempNode->key, tempNode->pos);
                    }

                    if (numPossible > 0) {
                        int dir = possibleDir[rand() % numPossible];
                        int displaceX = newX + directions[dir][0];
                        int displaceY = newY + directions[dir][1];

                        monDisplace->setPos((Pos){displaceX, displaceY});
                        monsterAt[displaceY][displaceX] = std::move(monsterAt[newY][newX]);
                        insert(heap.get(), tempNode->key, monDisplace->getPos());

                        mon->setPos((Pos){newX, newY});
                        monsterAt[newY][newX] = std::move(monsterAt[y][x]);
                        insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                    }
                    else {
                        monDisplace->setPos((Pos){x, y});
                        mon->setPos((Pos){newX, newY});

                        std::unique_ptr<Monster> tmp = std::move(monsterAt[y][x]);
                        monsterAt[y][x] = std::move(monsterAt[newY][newX]);
                        monsterAt[newY][newX] = std::move(tmp);

                        insert(heap.get(), tempNode->key, monDisplace->getPos());
                        insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                    }
                }
                else if (newX == player.getPos().x && newY == player.getPos().y) {
                    if (!godmodeFlag) {
                        int dam = mon->doDamage();
                        int hpLeft = player.takeDamage(dam);

                        if (dam > 0) {
                            printDungeon(supportsColor, fogOfWarToggle);
                            if (supportsColor) {
                                attron(COLOR_PAIR(Color::Red));
                                mvaddch(player.getPos().y + 1, player.getPos().x, '@');
                                attroff(COLOR_PAIR(Color::Red));
                            }
                            printLineColor(STATUS_LINE1, Color::Red, supportsColor, "%s dealt %d damage to you.\n", mon->getName().c_str(), dam);
                            napms(500);
                            flushinp();
                        }
                        if (hpLeft <= 0) {
                            updateAroundPlayer();
                            printDungeon(supportsColor, fogOfWarToggle);
                            printLine(MESSAGE_LINE, "");
                            printLine(STATUS_LINE1, "Player killed by %s, gg", mon->getName().c_str());
                            printLine(STATUS_LINE2, "Press any key to continue...");
                            getch();
                            lossScreen(supportsColor);

                            clearAll();
                            return 0;
                        }
                        
                    }

                    insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());                    
                }
                else {
                    mon->setPos((Pos){newX, newY});
                    monsterAt[newY][newX] = std::move(monsterAt[y][x]);
                    insert(heap.get(), time + 1000 / mon->getSpeed(), mon->getPos());
                }
            }
        }
    }
}
