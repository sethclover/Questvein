#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "dungeon.h"

typedef enum {
    op_hb,
    op_ha
} Operation;

Tile dungeon[MAX_HEIGHT][MAX_WIDTH];

void initDungeon() {
    generateHardness(dungeon);

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

void buildCorridors(Room *rooms, int roomCount) {
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

void buildStairs(Room *rooms, int roomCount) {
    int xUp = rand() % rooms[0].width + rooms[0].x;
    int yUp = rand() % rooms[0].height + rooms[0].y;
    dungeon[yUp][xUp].type = STAIR_UP;

    int xDown = rand() % rooms[roomCount - 1].width + rooms[roomCount - 1].x;
    int yDown = rand() % rooms[roomCount - 1].height + rooms[roomCount - 1].y;
    dungeon[yDown][xDown].type = STAIR_DOWN;
}

void placePlayer(Room *rooms, int roomCount) {
    int roomNum = rand() % roomCount;
    int x, y;
    do {
        x = rand() % (rooms[roomNum].width - 2) + rooms[roomNum].x + 1;
        y = rand() % (rooms[roomNum].height - 2) + rooms[roomNum].y + 1;
    } while (dungeon[y][x].type != FLOOR);
    

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

void printHardness(Tile dungeonToPrint[MAX_HEIGHT][MAX_WIDTH]) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            int h = dungeonToPrint[i][j].hardness;
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

int main(int argc, char *argv[]) {
    Operation op;
    if (argc == 2 && argv[1][0] == '-') {
        if (argv[1][1] == 'h') {
            if (argv[1][2] == 'b') {
                op = op_hb;
            }
            else if (argv[1][2] == 'a') {
                op = op_ha;
            }
        }
    }

    srand(time(NULL));

    initDungeon();

    if (op == op_hb) {
        printHardness(dungeon);
    }

    int roomCount = rand() % 5 + 7;
    Room *rooms = buildRooms(roomCount);
    if (!rooms) {
        return 1;
    }
    buildCorridors(rooms, roomCount);
    buildStairs(rooms, roomCount);
    placePlayer(rooms, roomCount);

    if (op == op_ha) {
        printHardness(dungeon);
    }
    printDungeon();

    free(rooms);
    rooms = NULL;
    return 0;
}
