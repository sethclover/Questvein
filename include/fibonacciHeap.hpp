#pragma once

#include <memory>

#include "dungeon.hpp"

class FibNode {
private:
    Pos pos;
    int key;
    int degree;
    FibNode *parent;
    FibNode *child;
    FibNode *left;
    FibNode *right;
    bool marked;

public:
    Pos getPos() { return pos; }
    void setPos(Pos p) { pos = p; }

    int getKey() { return key; }
    void setKey(int k) { key = k; }

    int getDegree() { return degree; }
    void setDegree(int d) { degree = d; }
    void incrementDegree() { degree++; }
    void decrementDegree() { degree--; }

    FibNode *getParent() { return parent; }
    void setParent(FibNode *p) { parent = p; }

    FibNode *getChild() { return child; }
    void setChild(FibNode *c) { child = c; }

    FibNode *getLeft() { return left; }
    void setLeft(FibNode *l) { left = l; }

    FibNode *getRight() { return right; }
    void setRight(FibNode *r) { right = r; }

    bool isMarked() { return marked; }
    void setMarked(bool m) { marked = m; }

    FibNode(int key, Pos pos) : pos(pos),
                                key(key),
                                degree(0),
                                parent(nullptr),
                                child(nullptr),
                                left(this), 
                                right(this),
                                marked(0) {}
    FibNode() = delete;
    ~FibNode() = default;
};

class FibHeap {
private:
    int numNodes;
    FibNode *min;
    std::vector<std::unique_ptr<FibNode>> nodes;

    int consolidate();
    void cut(FibNode *node, FibNode *parent);
    void cascadingCut(FibNode *node);

public:
    FibNode *getMin();
    FibNode *insertNew(int key, Pos pos);
    FibNode *insertNode(FibNode *node);
    FibNode *extractMin();
    void decreaseKey(FibNode *node, int newKey);
    void removeNode(FibNode *node);

    FibHeap() : numNodes(0), min(nullptr) {}
    ~FibHeap() = default;
};

FibNode *insertNew(int key, Pos pos);
FibNode *insertNode(FibNode *node);
FibNode *extractMin();
void decreaseKey(FibNode *node, int newKey);
void removeNode(FibNode *node);
