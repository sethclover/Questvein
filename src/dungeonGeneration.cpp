#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dungeon.hpp"
#include "pathFinding.hpp"
#include "perlin.hpp"

Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
int roomCount;
std::vector<Room> rooms;
std::vector<Pos> upStairs;
int upStairsCount;
std::vector<Pos> downStairs;
int downStairsCount;

Player player((Pos){-1, -1});
std::unique_ptr<Monster> monsterAt[MAX_HEIGHT][MAX_WIDTH];
std::vector<std::unique_ptr<Object>> objectsAt[MAX_HEIGHT][MAX_WIDTH];

void initDungeon() {
    generateHardness();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (i == 0 || i == MAX_HEIGHT - 1 || j == 0 || j == MAX_WIDTH - 1) {
                dungeon[i][j].hardness = MAX_HARDNESS;
            }
            dungeon[i][j].type = ROCK;
        }
    }
}

int placeRoom(Room& room) {
    for (int i = room.getPos().y - 1; i < room.getPos().y + room.getHeight() + 1; i++) {
        for (int j = room.getPos().x - 1; j < room.getPos().x + room.getWidth() + 1; j++) {
            if (dungeon[i][j].type == FLOOR) {
                return 0;
            }
        }
    }

    for (int i = room.getPos().y; i < room.getPos().y + room.getHeight(); i++) {
        for (int j = room.getPos().x; j < room.getPos().x + room.getWidth(); j++) {
            dungeon[i][j].type = FLOOR;
            dungeon[i][j].hardness = 0;
        }
    }

    return 1;
}

int buildRooms() {
    rooms.reserve(roomCount);
    for (int i = 0; i < roomCount; i++) {
        for (int j = 0; j < ATTEMPTS; j++) {
            int width = rand() % 9 + 4;
            int height = rand() % 10 + 3;
            int x = rand() % (MAX_WIDTH - width - 1) + 1;
            int y = rand() % (MAX_HEIGHT - height - 1) + 1;
           
            Room room = Room((Pos){x, y}, width, height);
            if (placeRoom(room)) {
                rooms.emplace_back(room);
                break;
            }
        }
    }
    return 0;
}

void buildCorridors() {
    for (int i = 0 ; i < roomCount - 1; i++) {
        int x = rand() % (rooms[i].getWidth() - 2) + rooms[i].getPos().x + 1;
        int y = rand() % (rooms[i].getHeight() - 2) + rooms[i].getPos().y + 1;
        int x2 = rand() % (rooms[i + 1].getWidth() - 2) + rooms[i + 1].getPos().x + 1;
        int y2 = rand() % (rooms[i + 1].getHeight() - 2) + rooms[i + 1].getPos().y + 1;

        int xDir = (x2 - x > 0) ? 1 : -1;
        int yDir = (y2 - y > 0) ? 1 : -1;

        while (x != x2 && y != y2) {
            int dir = rand() % 5;

            if (dir == 0) {
                if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
                y += yDir;
            } 
            else {
                if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
                x += xDir;
            }
        }
        while (x != x2) {
            if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
            x += xDir;
        }
        while (y != y2) {
            if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
            y += yDir;
        }
    }
}

int buildStairs() {
    int xUp = rand() % rooms.front().getWidth() + rooms.front().getPos().x;
    int yUp = rand() % rooms.front().getHeight() + rooms.front().getPos().y;
    upStairsCount = 1;
    upStairs.emplace_back((Pos){xUp, yUp});
    dungeon[yUp][xUp].type = STAIR_UP;

    int xDown = rand() % rooms.back().getWidth() + rooms.back().getPos().x;
    int yDown = rand() % rooms.back().getHeight() + rooms.back().getPos().y;
    downStairsCount = 1;
    downStairs.emplace_back((Pos){xDown, yDown});
    dungeon[yDown][xDown].type = STAIR_DOWN;

    return 0;
}

void spawnPlayer() {
    int x, y;
    bool valid = false;
    while (!valid) {
        x = rand() % (rooms[rand() % roomCount].getWidth() - 2) + rooms[rand() % roomCount].getPos().x + 1;
        y = rand() % (rooms[rand() % roomCount].getHeight() - 2) + rooms[rand() % roomCount].getPos().y + 1;
        if (dungeon[y][x].type == FLOOR) {
            valid = true;
        }
        else {
            continue;
        }

        for (int i = 0; i < upStairsCount; i++) {
            for (int j = 0; j < roomCount; j++) {
                if (upStairs[i].x >= rooms[j].getPos().x && upStairs[i].x < rooms[j].getPos().x + rooms[j].getWidth() &&
                    upStairs[i].y >= rooms[j].getPos().y && upStairs[i].y < rooms[j].getPos().y + rooms[j].getHeight() &&
                    x >= rooms[j].getPos().x && x < rooms[j].getPos().x + rooms[j].getWidth() &&
                    y >= rooms[j].getPos().y && y < rooms[j].getPos().y + rooms[j].getHeight()) {
                        valid = false;
                    }
            }
        }
        for (int i = 0; i < downStairsCount; i++) {
            for (int j = 0; j < roomCount; j++) {
                if (downStairs[i].x >= rooms[j].getPos().x && downStairs[i].x < rooms[j].getPos().x + rooms[j].getWidth() &&
                    downStairs[i].y >= rooms[j].getPos().y && downStairs[i].y < rooms[j].getPos().y + rooms[j].getHeight() &&
                    x >= rooms[j].getPos().x && x < rooms[j].getPos().x + rooms[j].getWidth() &&
                    y >= rooms[j].getPos().y && y < rooms[j].getPos().y + rooms[j].getHeight()) {
                        valid = false;
                    }
            }
        }
    }

    player.setPos((Pos){x, y});
}

int spawnMonsters(int numMonsters, int playerX, int playerY) {
    int monsterTypeListSize = monsterTypeList.size();

    for (int i = 0; i < numMonsters; i++) {
        for (int j = 0; j < ATTEMPTS; j++) {
            int placed = 0;
            int x = rand() % (MAX_WIDTH - 2) + 1;
            int y = rand() % (MAX_HEIGHT - 2) + 1;
            if (dungeon[y][x].type != FLOOR) {
                continue;
            }
            for (int k = 0; k < roomCount; k++) {
                if ((playerX >= rooms[k].getPos().x && playerX <= rooms[k].getPos().x + rooms[k].getWidth() - 1 &&
                    playerY >= rooms[k].getPos().y && playerY <= rooms[k].getPos().y + rooms[k].getHeight() - 1)) {
                    continue;
                }
                else if (x >= rooms[k].getPos().x && x <= rooms[k].getPos().x + rooms[k].getWidth() - 1 &&
                         y >= rooms[k].getPos().y && y <= rooms[k].getPos().y + rooms[k].getHeight() - 1) {
                    if (monsterAt[y][x]) {
                        continue;
                    }

                    int monTypeIndex = rand() % monsterTypeListSize;
                    MonsterType *monType = &monsterTypeList[monTypeIndex];
                    int rarityCheck = rand() % 100 + 1;
                    if (rarityCheck >= monType->rarity || !monType->eligible || !monType->valid) {
                        continue;
                    }
                    
                    monsterAt[y][x] = std::make_unique<Monster>(monType, monTypeIndex, (Pos){x, y});
                    placed = 1;
                    if (monsterAt[y][x].get()->isUnique() || monsterAt[y][x].get()->isBoss()) {
                        monsterTypeList[monTypeIndex].eligible = false;
                    }

                    break;
                }
            }

            if (placed) {
                break;
            }
        }
    }

    return 0;
}

int spawnObjects(int numObjects) {
    int objectTypeListSize = objectTypeList.size();

    for (int i = 0; i < numObjects; i++) {
        for (int j = 0; j < ATTEMPTS; j++) {
            int x = rand() % MAX_WIDTH;
            int y = rand() % MAX_HEIGHT;
            if (dungeon[y][x].type != FLOOR || (player.getPos().x == x && player.getPos().y == y)) {
                continue;
            }
            
            int objTypeIndex = rand() % objectTypeListSize;
            ObjectType *objType = &objectTypeList[objTypeIndex];
            int rarityCheck = rand() % 100;
            if (rarityCheck >= objType->rarity || !objType->eligible || !objType->valid) {
                continue;
            }


            objectsAt[y][x].emplace_back(std::make_unique<Object>(objType, objTypeIndex, (Pos){x, y}));
            if (objectsAt[y][x].back().get()->isArtifact()) {
                objectTypeList[objTypeIndex].eligible = false;
            }

            break;
        }

    }

    return 0;
}

void printHardness() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            int h = dungeon[i][j].hardness;
            if (h < 1) {
                printf(" ");
            }
            else if ( h < 65) {
                printf(".");
            }
            else if (h < 130) {
                printf(";");
            } 
            else if (h < 195) {
                printf("*");
            }
            else if (h < 255) {
                printf("%%");
            } 
            else {
                printf("#");
            }
        }
        printf("\n");
    }
}

int generateStructures() {
    roomCount = rand() % 5 + 7;
    buildRooms();
    buildCorridors();
    buildStairs();

    return 0;
}

void clearAll() {
    rooms.clear();
    upStairs.clear();
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            monsterAt[i][j] = nullptr;
        }
    }

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            objectsAt[i][j].clear();
        }
    }
}
