#include <memory>

#include "dungeon.hpp"
#include "fibonacciHeap.hpp"

int tunnelingDistances(const int x, const int y) {
    std::unique_ptr<FibHeap> heap = std::make_unique<FibHeap>();
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};

    dungeon[y][x].tunnelingDist = 0;
    nodes[y][x] = insert(heap.get(), 0, (Pos){x, y});

    while (heap->min != NULL) {
        FibNode *minNode = extractMin(heap.get());

        Pos pos = minNode->pos;
        int dist = minNode->key;

        nodes[pos.y][pos.x] = NULL;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int newX = pos.x + j;
                int newY = pos.y + i;

                if ((i == 0 && j == 0) || dungeon[newY][newX].hardness == 255) {
                    continue;
                }
    
                int newDist = dist + dungeon[newY][newX].hardness / 85 + 1;
                if (newDist < dungeon[newY][newX].tunnelingDist) {
                    dungeon[newY][newX].tunnelingDist = newDist;

                    if (nodes[newY][newX] == NULL) {
                        nodes[newY][newX] = insert(heap.get(), newDist, (Pos){newX, newY});
                    }
                    else {
                        decreaseKey(heap.get(), nodes[newY][newX], newDist);
                    }
                }
            }
        }

        //delete minNode;     
    }

    return 0;
}

int nonTunnelingDistances(const int x, const int y) {
    std::unique_ptr<FibHeap> heap = std::make_unique<FibHeap>();
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};

    dungeon[y][x].nonTunnelingDist = 0;
    nodes[y][x] = insert(heap.get(), 0, (Pos){x, y});

    while (heap.get()->min != NULL) {
        FibNode *minNode = extractMin(heap.get());
        Pos pos = minNode->pos;
        int dist = minNode->key;

        nodes[pos.y][pos.x] = NULL;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int newX = pos.x + j;
                int newY = pos.y + i;
                
                if ((i == 0 && j == 0) || dungeon[newY][newX].hardness > 0) {
                    continue;
                }
    
                int newDist = dist + 1;
                if (newDist < dungeon[newY][newX].nonTunnelingDist) {
                    dungeon[newY][newX].nonTunnelingDist = newDist;

                    if (nodes[newY][newX] == NULL) {
                        nodes[newY][newX] = insert(heap.get(), newDist, (Pos){newX, newY});
                    }
                    else {
                        decreaseKey(heap.get(), nodes[newY][newX], newDist);
                    }
                }
            }
        }

        //delete minNode;       
    }

    return 0;
}

int generateDistances(const int x, const int y) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            dungeon[i][j].tunnelingDist = UNREACHABLE;
            dungeon[i][j].nonTunnelingDist = UNREACHABLE;
        }
    }
    tunnelingDistances(x, y);
    nonTunnelingDistances(x, y);

    return 0;
}
