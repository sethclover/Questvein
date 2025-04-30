#pragma once

#include <memory>

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

    FibNode() = delete;
    FibNode(int key, Pos pos) : pos(pos),
                                key(key),
                                degree(0),
                                parent(nullptr),
                                child(nullptr),
                                left(this), 
                                right(this),
                                marked(0) {}
    ~FibNode() = default;
};

class FibHeap {
public:
    int numNodes;
    FibNode *min;
    std::vector<std::unique_ptr<FibNode>> nodes;

    FibHeap() : numNodes(0), min(nullptr) {}
    ~FibHeap() = default;

    FibNode *getMin() { return min; }
};

FibNode *insertNew(FibHeap *heap, int key, Pos pos);
FibNode *insertNode(FibHeap *heap, FibNode *node);
FibNode *extractMin(FibHeap *heap);
void decreaseKey(FibHeap *heap, FibNode *node, int newKey);
void removeNode(FibHeap *heap, FibNode *node);
