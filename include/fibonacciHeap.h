#pragma once

#include "dungeon.h"

typedef struct FibNode {
    Pos pos;
    int key;
    int degree;
    struct FibNode *parent;
    struct FibNode *child;
    struct FibNode *left;
    struct FibNode *right;
    int marked;
} FibNode;

typedef struct FibHeap {
    int numNodes;
    FibNode *min;
} FibHeap;

FibHeap *createFibHeap();
FibNode *insert(FibHeap *heap, int key, Pos pos);
FibNode *getMin(FibHeap *heap);
FibNode *extractMin(FibHeap *heap);
void decreaseKey(FibHeap *heap, FibNode *node, int newKey);
void destroyFibHeap(FibHeap *heap);
