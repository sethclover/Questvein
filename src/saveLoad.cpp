#include <cstdint>
#include <cstdlib>
#include <endian.h>
#include <iostream>
#include <string>

#include "dungeon.hpp"

std::string dungeonFile;

int  setupDungeonFile(char *filename) {
    char *homeDir = getenv("HOME");
    if (!homeDir) return 1;

    dungeonFile = std::string(homeDir) + "/.rlg327/" + std::string(filename);
    return 0 ;
}

int loadDungeon(char *filename) {
    setupDungeonFile(filename);
    FILE *file = fopen(dungeonFile.c_str(), "r");

    char marker[12];
    fread(marker, 1, 12, file);

    uint32_t version;
    fread(&version, 4, 1, file);
    version = be32toh(version);

    uint32_t size;
    fread(&size, 4, 1, file);
    size = be32toh(size);

    uint8_t pos[2];
    fread(pos, 2, 1, file);
    player.setPos((Pos){(int)pos[0], (int)pos[1]});

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            fread(&dungeon[i][j].hardness, 1, 1, file);
            if (dungeon[i][j].hardness == 0) {
                dungeon[i][j].type = CORRIDOR;
            }
            else {
                dungeon[i][j].type = ROCK;
            }
        }
    }

    uint16_t r = 0;
    fread(&r, 2, 1, file);
    r = be16toh(r);
    roomCount = r;

    rooms.reserve(r);
    for (int i = 0; i < r; i++) {
        uint8_t x, y, width, height;
        fread(&x, 1, 1, file);
        fread(&y, 1, 1, file);
        fread(&width, 1, 1, file);
        fread(&height, 1, 1, file);

        rooms.emplace_back(Room((Pos){(int)x, (int)y}, (int)width, (int)height));
        for (int j = rooms.back().getPos().y; j < rooms.back().getPos().y + rooms.back().getHeight(); j++) {
            for (int k = rooms.back().getPos().x; k < rooms.back().getPos().x + rooms.back().getWidth(); k++) {
                dungeon[j][k].type = FLOOR;
            }
        }
    }

    uint16_t u;
    fread(&u, 2, 1, file);
    u = be16toh(u);
    upStairsCount = u;

    for (int i = 0; i < u; i++) {
        uint8_t x, y;
        fread(&x, 1, 1, file);
        fread(&y, 1, 1, file);
        upStairs.emplace_back((Pos){(int)x, (int)y});
        dungeon[(int)y][(int)x].type = STAIR_UP;
    }

    uint16_t d;
    fread(&d, 2, 1, file);
    d = be16toh(d);
    downStairsCount = d;

    for (int i = 0; i < d; i++) {
        uint8_t x, y;
        fread(&x, 1, 1, file);
        fread(&y, 1, 1, file);
        downStairs.emplace_back((Pos){(int)x, (int)y});
        dungeon[(int)y][(int)x].type = STAIR_DOWN;
    }

    std::cout << "Dungeon loaded from" << dungeonFile << std::endl;
    fclose(file);
    return 0;
}

int saveDungeon(char *filename) {
    setupDungeonFile(filename);
    FILE *file = fopen(dungeonFile.c_str(), "w");

    fwrite("RLG327-S2025", 1, 12, file);

    uint32_t version = htobe32(0);
    fwrite(&version, 4, 1, file);

    uint32_t size = htobe32(sizeof(1712 + roomCount * 4));
    fwrite(&size, 4, 1, file);

    uint8_t pos[2] = {(uint8_t) player.getPos().x, (uint8_t) player.getPos().y};
    fwrite(pos, 2, 1, file);

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            fwrite(&dungeon[i][j].hardness, 1, 1, file);
        }
    }

    uint16_t r = htobe16(roomCount);
    fwrite(&r, 2, 1, file);

    for (int i = 0; i < roomCount; i++) {
        uint8_t room[4] = {
            static_cast<uint8_t>(rooms[i].getPos().x),
            static_cast<uint8_t>(rooms[i].getPos().y),
            static_cast<uint8_t>(rooms[i].getWidth()),
            static_cast<uint8_t>(rooms[i].getHeight())
        };
        fwrite(room, 4, 1, file);
    }

    uint16_t u = htobe16(upStairsCount);
    fwrite(&u, 2, 1, file);

    for (int i = 0; i < upStairsCount; i++) {
        uint8_t upStairsPos[2] = {static_cast<uint8_t>(upStairs[i].x), static_cast<uint8_t>(upStairs[i].y)};
        fwrite(upStairsPos, 2, 1, file);
    }

    uint16_t d = htobe16(downStairsCount);
    fwrite(&d, 2, 1, file);

    for (int i = 0; i < downStairsCount; i++) {
        uint8_t downStairsPos[2] = {static_cast<uint8_t>(downStairs[i].x), static_cast<uint8_t>(downStairs[i].y)};
        fwrite(downStairsPos, 2, 1, file);
    }

    std::cout << "Dungeon saved to" << dungeonFile << std::endl;
    fclose(file);
    return 0;
}
