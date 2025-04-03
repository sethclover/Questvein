#pragma once

#include "dungeon.hpp"

class FibNode {
public:
    Pos pos;
    int key;
    int degree;
    FibNode *parent;
    FibNode *child;
    FibNode *left;
    FibNode *right;
    int marked;
};

class FibHeap {
public:
    int numNodes;
    FibNode *min;
};

FibHeap *createFibHeap();
FibNode *insert(FibHeap *heap, int key, Pos pos);
FibNode *getMin(FibHeap *heap);
FibNode *extractMin(FibHeap *heap);
void decreaseKey(FibHeap *heap, FibNode *node, int newKey);
void destroyFibHeap(FibHeap *heap);
