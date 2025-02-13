#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dungeon.h"

Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
int roomCount;
Room *rooms;
Pos player;
Pos *upStairs;
int upStairsCount;
Pos *downStairs;
int downStairsCount;

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
        printf("Failed to allocate memory for rooms\n");
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
    
    printf("Failed to build all rooms\n");
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

void buildStairs() {
    int xUp = rand() % rooms[0].width + rooms[0].x;
    int yUp = rand() % rooms[0].height + rooms[0].y;

    upStairsCount = 1;
    upStairs = malloc(upStairsCount * sizeof(Pos));
    if (!upStairs) {
        printf("Failed to allocate memory for upStairs\n");
        return;
    }
    upStairs[0].x = xUp;
    upStairs[0].y = yUp;
    dungeon[yUp][xUp].type = STAIR_UP;

    int xDown = rand() % rooms[roomCount - 1].width + rooms[roomCount - 1].x;
    int yDown = rand() % rooms[roomCount - 1].height + rooms[roomCount - 1].y;

    downStairsCount = 1;
    downStairs = malloc(downStairsCount * sizeof(Pos));
    if (!downStairs) {
        printf("Failed to allocate memory for downStairs\n");
        return;
    }
    downStairs[0].x = xDown;
    downStairs[0].y = yDown;
    dungeon[yDown][xDown].type = STAIR_DOWN;
}

void placePlayer() {
    int roomNum = rand() % roomCount;
    int x, y;
    do {
        x = rand() % (rooms[roomNum].width - 2) + rooms[roomNum].x + 1;
        y = rand() % (rooms[roomNum].height - 2) + rooms[roomNum].y + 1;
    } while (dungeon[y][x].type != FLOOR);
    
    player.x = x;
    player.y = y;
    dungeon[y][x].type = '@';
}

void printDungeon() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            printf("%c", dungeon[i][j].type);
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

void fillDungeon() {
    roomCount = rand() % 5 + 7;
    rooms = buildRooms(roomCount);
    buildCorridors();
    buildStairs();
    placePlayer();
}
