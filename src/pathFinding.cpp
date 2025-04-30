#include <memory>

#include "dungeon.hpp"
#include "fibonacciHeap.hpp"

int tunnelingDistances(Pos pos) {
    std::unique_ptr<FibHeap> heap = std::make_unique<FibHeap>();
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {nullptr};

    dungeon[pos.y][pos.x].tunnelingDist = 0;
    nodes[pos.y][pos.x] = insertNew(heap.get(), 0, pos);

    while (heap->min != nullptr) {
        FibNode *minNode = extractMin(heap.get());

        Pos minPos = minNode->pos;
        int dist = minNode->key;

        nodes[minPos.y][minPos.x] = nullptr;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int newX = minPos.x + j;
                int newY = minPos.y + i;

                if ((i == 0 && j == 0) || dungeon[newY][newX].hardness == 255) {
                    continue;
                }
    
                int newDist = dist + dungeon[newY][newX].hardness / 85 + 1;
                if (newDist < dungeon[newY][newX].tunnelingDist) {
                    dungeon[newY][newX].tunnelingDist = newDist;

                    if (nodes[newY][newX] == nullptr) {
                        nodes[newY][newX] = insertNew(heap.get(), newDist, (Pos){newX, newY});
                    }
                    else {
                        decreaseKey(heap.get(), nodes[newY][newX], newDist);
                    }
                }
            }
        }
    }

    return 0;
}

int nonTunnelingDistances(Pos pos) {
    std::unique_ptr<FibHeap> heap = std::make_unique<FibHeap>();
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {nullptr};

    dungeon[pos.y][pos.x].nonTunnelingDist = 0;
    nodes[pos.y][pos.x] = insertNew(heap.get(), 0, pos);

    while (heap.get()->min != nullptr) {
        FibNode *minNode = extractMin(heap.get());
        Pos minPos = minNode->pos;
        int dist = minNode->key;

        nodes[minPos.y][minPos.x] = nullptr;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int newX = minPos.x + j;
                int newY = minPos.y + i;
                
                if ((i == 0 && j == 0) || dungeon[newY][newX].hardness > 0) {
                    continue;
                }
    
                int newDist = dist + 1;
                if (newDist < dungeon[newY][newX].nonTunnelingDist) {
                    dungeon[newY][newX].nonTunnelingDist = newDist;

                    if (nodes[newY][newX] == nullptr) {
                        nodes[newY][newX] = insertNew(heap.get(), newDist, (Pos){newX, newY});
                    }
                    else {
                        decreaseKey(heap.get(), nodes[newY][newX], newDist);
                    }
                }
            }
        }
    }

    return 0;
}

int generateDistances(Pos pos) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            dungeon[i][j].tunnelingDist = UNREACHABLE;
            dungeon[i][j].nonTunnelingDist = UNREACHABLE;
        }
    }
    tunnelingDistances(pos);
    nonTunnelingDistances(pos);

    return 0;
}
