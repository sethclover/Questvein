#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dungeon.h"
#include "perlin.h"
#include "pathFinding.h"
#include "fibonacciHeap.h"

Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
int roomCount;
Room *rooms;
Pos player;
Pos *upStairs;
int upStairsCount;
Pos *downStairs;
int downStairsCount;

Mon *monsterAt[MAX_HEIGHT][MAX_WIDTH] = {NULL};
Mon **monsters;

void initDungeon() {
    generateHardness();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            dungeon[i][j].type = ROCK;
        }
    }

    for (int i = 1; i < MAX_WIDTH - 1; i++) {
        dungeon[0][i].type = '-';
        dungeon[0][i].hardness = MAX_HARDNESS;

        dungeon[MAX_HEIGHT - 1][i].type = '-';
        dungeon[MAX_HEIGHT - 1][i].hardness = MAX_HARDNESS;
    }
    for (int i = 1; i < MAX_HEIGHT - 1; i++) {
        dungeon[i][0].type = '|';
        dungeon[i][0].hardness = MAX_HARDNESS;

        dungeon[i][MAX_WIDTH - 1].type = '|';
        dungeon[i][MAX_WIDTH - 1].hardness = MAX_HARDNESS;
    }

    dungeon[0][0].type = CORNER;
    dungeon[0][0].hardness = MAX_HARDNESS;

    dungeon[0][MAX_WIDTH - 1].type = '+';
    dungeon[0][MAX_WIDTH - 1].hardness = MAX_HARDNESS;

    dungeon[MAX_HEIGHT - 1][0].type = '+';
    dungeon[MAX_HEIGHT - 1][0].hardness = MAX_HARDNESS;

    dungeon[MAX_HEIGHT - 1][MAX_WIDTH - 1].type = '+';
    dungeon[MAX_HEIGHT - 1][MAX_WIDTH - 1].hardness = MAX_HARDNESS;
}

void initRoom(Room *roomsLoaded) {
    rooms = malloc(roomCount * sizeof(Room));
    if (!rooms) {
        fprintf(stderr, "Error: Failed to allocate memory for rooms\n");
        free(rooms);
        return;
    }
    for (int i = 0; i < roomCount; i++) {
        rooms[i].width = roomsLoaded[i].width;
        rooms[i].height = roomsLoaded[i].height;
        rooms[i].x = roomsLoaded[i].x;
        rooms[i].y = roomsLoaded[i].y;
    }
}

int placeRoom(Room room) {
    for (int i = room.y - 1; i < room.y + room.height + 1; i++) {
        for (int j = room.x - 1; j < room.x + room.width + 1; j++) {
            if (dungeon[i][j].type == FLOOR) {
                return 0;
            }
        }
    }

    for (int i = room.y; i < room.y + room.height; i++) {
        for (int j = room.x; j < room.x + room.width; j++) {
            dungeon[i][j].type = FLOOR;
            dungeon[i][j].hardness = 0;
        }
    }

    return 1;
}

Room* buildRooms(int roomCount) {
    int roomsBuilt = 0;

    Room *rooms = malloc(roomCount * sizeof(Room));
    if (!rooms) {
        fprintf(stderr, "Error: Failed to allocate memory for rooms\n");
        free(rooms);
        return NULL;
    }

    for (int i = 0; i < ATTEMPTS; i++) {
        rooms[roomsBuilt].width = rand() % 9 + 4;
        rooms[roomsBuilt].height = rand() % 10 + 3;
        rooms[roomsBuilt].x = rand() % (MAX_WIDTH - rooms[roomsBuilt].width - 1) + 1;
        rooms[roomsBuilt].y = rand() % (MAX_HEIGHT - rooms[roomsBuilt].height - 1) + 1;

        if (placeRoom(rooms[roomsBuilt])) {
            roomsBuilt++;
        }

        if (roomsBuilt >= roomCount) {
            return rooms;
        }
    }
    
    fprintf(stderr, "Error: Failed to build all rooms\n");
    free(rooms);
    return NULL;
}

void buildCorridors() {
    for (int i = 0 ; i < roomCount - 1; i++) {
        int x = rand() % (rooms[i].width - 2) + rooms[i].x + 1;
        int y = rand() % (rooms[i].height - 2) + rooms[i].y + 1;
        int x2 = rand() % (rooms[i + 1].width - 2) + rooms[i + 1].x + 1;
        int y2 = rand() % (rooms[i + 1].height - 2) + rooms[i + 1].y + 1;

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
    int xUp = rand() % rooms[0].width + rooms[0].x;
    int yUp = rand() % rooms[0].height + rooms[0].y;

    upStairsCount = 1;
    upStairs = malloc(upStairsCount * sizeof(Pos));
    if (!upStairs) {
        fprintf(stderr, "Error: Failed to allocate memory for upStairs\n");
        free(upStairs);
        return 1;
    }
    upStairs[0].x = xUp;
    upStairs[0].y = yUp;
    dungeon[yUp][xUp].type = STAIR_UP;

    int xDown = rand() % rooms[roomCount - 1].width + rooms[roomCount - 1].x;
    int yDown = rand() % rooms[roomCount - 1].height + rooms[roomCount - 1].y;

    downStairsCount = 1;
    downStairs = malloc(downStairsCount * sizeof(Pos));
    if (!downStairs) {
        fprintf(stderr, "Error: Failed to allocate memory for downStairs\n");
        free(upStairs);
        free(downStairs);
        return 1;
    }
    downStairs[0].x = xDown;
    downStairs[0].y = yDown;
    dungeon[yDown][xDown].type = STAIR_DOWN;

    return 0;
}

void spawnPlayer() {
    int roomNum = rand() % roomCount;
    int x, y;
    do {
        x = rand() % (rooms[roomNum].width - 2) + rooms[roomNum].x + 1;
        y = rand() % (rooms[roomNum].height - 2) + rooms[roomNum].y + 1;
    } while (dungeon[y][x].type != FLOOR);
    
    player.x = x;
    player.y = y;
}

Mon *createMonsterWithMonType(char c, Pos pos) {
    Mon *monster = malloc(sizeof(Mon));
    if (!monster) {
        fprintf(stderr, "Error: Failed to allocate memory for monster\n");
        free(monster);
        return NULL;
    }
    monster->pos.x = pos.x;
    monster->pos.y = pos.y;
    monster->speed = rand() % 16 + 5;

    int num;
    c = tolower(c);
    if (c >= '0' && c <= '9') {
        num = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        num = c - 'a' + 10;
    } else {
        printf("Error: Invalid hex character '%c'\n", c);
        free(monster);
        return NULL;
    }

    monster->intelligent = num & 1;         
    monster->telepathic = (num >> 1) & 1;   
    monster->tunneling = (num >> 2) & 1;
    monster->erratic = (num >> 3) & 1;

    return monster;
}

Mon *createMonster(Pos pos) {
    Mon *monster = malloc(sizeof(Mon));
    if (!monster) {
        fprintf(stderr, "Error: Failed to allocate memory for monster\n");
        free(monster);
        return NULL;
    }

    monster->intelligent = rand() % 2;
    monster->tunneling = rand() % 2;
    monster->telepathic = rand() % 2;
    monster->erratic = rand() % 2;
    monster->speed = rand() % 16 + 5;
    monster->pos = pos;

    return monster;
}

int spawnMonsterWithMonType(char monType) {
    monsters = malloc(sizeof(Mon*));
    if (!monsters) {
        fprintf(stderr, "Error: Failed to allocate memory for monsters\n");
        free(monsters);
        return 1;
    }

    for (int j = 0; j < ATTEMPTS; j++) {
        int placed = 0;
        int x = rand() % MAX_WIDTH;
        int y = rand() % MAX_HEIGHT;
        if (dungeon[y][x].type != FLOOR) {
            continue;
        }
        for (int k = 0; k < roomCount; k++) {
            if ((player.x >= rooms[k].x && player.x <= rooms[k].x + rooms[k].width - 1 &&
                player.y >= rooms[k].y && player.y <= rooms[k].y + rooms[k].height - 1)) {
                continue;
            }
            else if (x >= rooms[k].x && x <= rooms[k].x + rooms[k].width - 1 &&
                        y >= rooms[k].y && y <= rooms[k].y + rooms[k].height - 1) {
                monsters[0] = createMonsterWithMonType(monType, (Pos){x, y});
                if (!monsters[0]) {
                    free(monsters[0]);
                    free(monsters);
                    return 1;
                }
                monsterAt[y][x] = monsters[0];
                placed = 1;
                break;
            }
        }

        if (placed) {
            break;
        }
    }
    if (monsters[0] == NULL) {
        fprintf(stderr, "Error: Failed to spawn monster\n");
        free(monsters[0]);
        free(monsters);
        return 1;
    }

    return 0;
}

int spawnMonsters(int numMonsters) {
    monsters = malloc(numMonsters * sizeof(Mon*));
    if (!monsters) {
        fprintf(stderr, "Error: Failed to allocate memory for monsters\n");
        free(monsters);
        return 1;
    }

    for (int i = 0; i < numMonsters; i++) {  
        for (int j = 0; j < ATTEMPTS; j++) {
            int placed = 0;
            int x = rand() % MAX_WIDTH;
            int y = rand() % MAX_HEIGHT;
            if (dungeon[y][x].type != FLOOR) {
                continue;
            }
            for (int k = 0; k < roomCount; k++) {
                if ((player.x >= rooms[k].x && player.x <= rooms[k].x + rooms[k].width - 1 &&
                    player.y >= rooms[k].y && player.y <= rooms[k].y + rooms[k].height - 1)) {
                    continue;
                }
                else if (x >= rooms[k].x && x <= rooms[k].x + rooms[k].width - 1 &&
                         y >= rooms[k].y && y <= rooms[k].y + rooms[k].height - 1) {
                    if (monsterAt[y][x]) {
                        continue;
                    }
                    else {
                        monsters[i] = createMonster((Pos){x, y});
                        if (!monsters[i]) {
                            for (int l = 0; l < i; l++) {
                                free(monsters[l]);
                            }
                            free(monsters);
                            fprintf(stderr, "Error: Failed to create monster\n");
                            return 1;
                        }
                        monsterAt[y][x] = monsters[i];
                        placed = 1;
                        break;
                    }
                }
            }

            if (placed) {
                break;
            }
        }
        if (monsters[i] == NULL) {
            fprintf(stderr, "Error: Failed to spawn monster\n");
            for (int k = 0; k < i; k++) {
                free(monsters[k]);
            }
            free(monsters);
            return 1;
        }
    }

    return 0;
}

void printDungeon() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (monsterAt[i][j]) {
                int personality = 1 * monsterAt[i][j]->intelligent +
                                  2 * monsterAt[i][j]->telepathic +
                                  4 * monsterAt[i][j]->tunneling +
                                  8 * monsterAt[i][j]->erratic;
                printf("%c", personality < 10 ? '0' + personality : 'A' + (personality - 10));
            }
            else if (player.x == j && player.y == i) {
                printf("@");
            }
            else {
                printf("%c", dungeon[i][j].type);
            }
        }
        printf("\n");
    }
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

void printTunnelingDistances() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (dungeon[i][j].tunnelingDist == UNREACHABLE) {
                printf(" ");
            }
            else if (dungeon[i][j].tunnelingDist == 0) {
                printf("@");
            }
            else {
                printf("%d", dungeon[i][j].tunnelingDist % 10);
            }
        }
        printf("\n");
    }
}

void printNonTunnelingDistances() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (dungeon[i][j].nonTunnelingDist == UNREACHABLE) {
                printf(" ");
            }
            else if (dungeon[i][j].nonTunnelingDist == 0) {
                printf("@");
            }
            else {
                printf("%d", dungeon[i][j].nonTunnelingDist % 10);
            }
        }
        printf("\n");
    }
}

int populateDungeonWithMonType(char monType) {
    if (generateDistances()) {
        return 1;
    }
    if (spawnMonsterWithMonType(monType)) {
        return 1;
    }

    return 0;
}

int populateDungeon(int numMonsters) {
    if (generateDistances()) {
        return 1;
    }
    if (spawnMonsters(numMonsters)) {
        return 1;
    }

    return 0;
}

int fillDungeonWithMonType(char monType) {
    roomCount = rand() % 5 + 7;
    rooms = buildRooms(roomCount);
    if (!rooms) {
        return 1;
    }

    buildCorridors();

    if (buildStairs()) {
        free(rooms);
        return 1;
    }

    spawnPlayer();

    if (populateDungeonWithMonType(monType)) {
        free(rooms);
        free(upStairs);
        free(downStairs);
        return 1;
    }

    return 0;
}

int fillDungeon(int numMonsters) {
    roomCount = rand() % 5 + 7;
    rooms = buildRooms(roomCount);
    if (!rooms) {
        return 1;
    }

    buildCorridors();

    if (buildStairs()) {
        free(rooms);
        return 1;
    }

    spawnPlayer();

    if (populateDungeon(numMonsters)) {
        free(rooms);
        free(upStairs);
        free(downStairs);
        return 1;
    }

    return 0;
}

void cleanup(int numMonsters) {
    free(rooms);
    free(upStairs);
    free(downStairs);
    for (int i = 0; i < numMonsters; i++) {
        if (monsters[i]) {
            free(monsters[i]);
        }
    }
    free(monsters);
}
