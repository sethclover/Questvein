#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_LENGTH 80
#define MAX_WIDTH 21
#define ATTEMPTS 1000
#define FLOOR '.'
#define CORRIDOR '#'
#define STAIR_UP '<'
#define STAIR_DOWN '>'
#define ROCK ' '

typedef struct {
    int length;
    int width;
    int x;
    int y;
} Room;

char dungeon[MAX_WIDTH][MAX_LENGTH];

void buildPerimeter() {
    memset(dungeon, ROCK, sizeof(dungeon));

    dungeon[0][0] = '+';
    dungeon[0][MAX_LENGTH - 1] = '+';
    dungeon[MAX_WIDTH - 1][0] = '+';
    dungeon[MAX_WIDTH - 1][MAX_LENGTH - 1] = '+';

    for (int i = 1; i < MAX_LENGTH - 1; i++) {
        dungeon[0][i] = '-';
        dungeon[MAX_WIDTH - 1][i] = '-';
    }
    for (int i = 1; i < MAX_WIDTH - 1; i++) {
        dungeon[i][0] = '|';
        dungeon[i][MAX_LENGTH - 1] = '|';
    }
}

int placeRoom(Room room) {
    for (int i = room.y - 1; i < room.y + room.width + 1; i++) {
        for (int j = room.x - 1; j < room.x + room.length + 1; j++) {
            if (dungeon[i][j] == FLOOR) {
                return 0;
            }
        }
    }

    for (int i = room.y; i < room.y + room.width; i++) {
        for (int j = room.x; j < room.x + room.length; j++) {
            dungeon[i][j] = FLOOR;
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
        rooms[roomsBuilt].length = rand() % 9 + 4;
        rooms[roomsBuilt].width = rand() % 10 + 3;
        rooms[roomsBuilt].x = rand() % (MAX_LENGTH - rooms[roomsBuilt].length - 1) + 1;
        rooms[roomsBuilt].y = rand() % (MAX_WIDTH - rooms[roomsBuilt].width - 1) + 1;

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
        int x = rooms[i].x + rooms[i].length / 2;
        int y = rooms[i].y + rooms[i].width / 2;
        int x2 = rooms[i + 1].x + rooms[i + 1].length / 2;
        int y2 = rooms[i + 1].y + rooms[i + 1].width / 2;
        int xDir = (x2 - x > 0) ? 1 : -1;
        int yDir = (y2 - y > 0) ? 1 : -1;

        while (x != x2 && y != y2) {
            int dir = rand() % 5;

            if (dir == 0) {
                if (dungeon[y][x] != FLOOR) {
                    dungeon[y][x] = CORRIDOR;
                }
                y += yDir;
            } 
            else {
                if (dungeon[y][x] != FLOOR) {
                    dungeon[y][x] = CORRIDOR;
                }
                x += xDir;
            }
        }
        while (x != x2) {
            if (dungeon[y][x] != FLOOR) {
                    dungeon[y][x] = CORRIDOR;
                }
            x += xDir;
        }
        while (y != y2) {
            if (dungeon[y][x] != FLOOR) {
                    dungeon[y][x] = CORRIDOR;
                }
            y += yDir;
        }
    }
}

void buildStairs(Room *rooms, int roomCount) {
    int xUp = rand() % rooms[0].length + rooms[0].x;
    int yUp = rand() % rooms[0].width + rooms[0].y;
    dungeon[yUp][xUp] = STAIR_UP;

    int xDown = rand() % rooms[roomCount - 1].length + rooms[roomCount - 1].x;
    int yDown = rand() % rooms[roomCount - 1].width + rooms[roomCount - 1].y;
    dungeon[yDown][xDown] = STAIR_DOWN;
}

void printDungeon() {
    for (int i = 0; i < MAX_WIDTH; i++) {
        for (int j = 0; j < MAX_LENGTH; j++) {
            printf("%c", dungeon[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    buildPerimeter();

    int roomCount = rand() % 3 + 6;
    Room *rooms = buildRooms(roomCount);
    if (!rooms) {
        return 1;
    }

    buildCorridors(rooms, roomCount);

    buildStairs(rooms, roomCount);

    printDungeon();

    free(rooms);
    rooms = NULL;
    return 0;
}
