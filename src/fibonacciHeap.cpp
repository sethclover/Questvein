#include <climits>
#include <cmath>

#include "dungeon.hpp"
#include "fibonacciHeap.hpp"

FibNode* FibHeap::insertNew(int key, Pos pos) {
    nodes.emplace_back(std::make_unique<FibNode>(key, pos));
    FibNode *node = nodes.back().get();

    if (min == nullptr) {
        min = node;
    }
    else {
        node->setRight(min->getRight());
        node->setLeft(min);
        min->getRight()->setLeft(node);
        min->setRight(node);

        if (node->getKey() < min->getKey()) {
            min = node;
        }
    }

    numNodes++;
    return node;
}

FibNode* FibHeap::insertNode(FibNode *node) {
    node->setParent(nullptr);
    node->setChild(nullptr);
    node->setLeft(node);
    node->setRight(node);
    node->setDegree(0);
    node->setMarked(false);

    if (min == nullptr) {
        min = node;
    }
    else {
        node->setRight(min->getRight());
        node->setLeft(min);
        min->getRight()->setLeft(node);
        min->setRight(node);

        if (node->getKey() < min->getKey()) {
            min = node;
        }
    }

    numNodes++;
    return node;
}

int FibHeap::consolidate() {
    int maxDegree;
    if (numNodes <= 1) {
        maxDegree = 1;
    }
    else {
        maxDegree = (int)(log(numNodes - 1) / log(2)) + 1;
    }
    FibNode **A = new FibNode*[maxDegree]();
    FibNode **nodes = new FibNode*[numNodes]();

    int count = 0;
    if (min) {
        FibNode *curr = min;
        do {
            nodes[count++] = curr;
            curr = curr->getRight();
        } while (curr != min);
    }

    for (int i = 0; i < count; i++) {
        FibNode *node = nodes[i];
        int degree = node->getDegree();

        while (A[degree] != nullptr) {
            FibNode *compareNode = A[degree];

            if (node->getKey() > compareNode->getKey()) {
                FibNode *temp = node;
                node = compareNode;
                compareNode = temp;
            }

            compareNode->getLeft()->setRight(compareNode->getRight());
            compareNode->getRight()->setLeft(compareNode->getLeft());
            compareNode->setParent(node);
            compareNode->setMarked(false);

            if (node->getChild() == nullptr) {
                node->setChild(compareNode);
                compareNode->setLeft(compareNode);
                compareNode->setRight(compareNode);
            }
            else {
                compareNode->setRight(node->getChild()->getRight());
                compareNode->setLeft(node->getChild());
                node->getChild()->getRight()->setLeft(compareNode);
                node->getChild()->setRight(compareNode);
            }

            node->incrementDegree();
            A[degree] = nullptr;
            degree++;   
        }
        A[degree] = node;
    }

    min = nullptr;
    for (int i = 0; i < maxDegree; i++) {
        if (A[i] != nullptr) {
            if (min == nullptr) {
                min = A[i];
                min->setLeft(min);
                min->setRight(min);
            }
            else {
                A[i]->setRight(min->getRight());
                A[i]->setLeft(min);
                min->getRight()->setLeft(A[i]);
                min->setRight(A[i]);

                if (A[i]->getKey() < min->getKey()) {
                    min = A[i];
                }
            }
        }
    }

    delete[] A;
    delete[] nodes;
    return 0;
}

FibNode* FibHeap::getMin() {
    return min;
}

FibNode* FibHeap::extractMin() {
    FibNode *minNode = min;

    if (minNode != nullptr) {
        if (minNode->getChild() != nullptr) {
            FibNode *child = minNode->getChild();
            FibNode *startChild = child;

            do {
                FibNode *next = child->getRight();
                child->setParent(nullptr);
                if (min == nullptr) {
                    min = child;
                    child->setLeft(child);
                    child->setRight(child);
                }
                else {
                    child->setRight(min->getRight());
                    child->setLeft(min);
                    min->getRight()->setLeft(child);
                    min->setRight(child);
                }
                child = next;
            } while (child != startChild);

            minNode->setChild(nullptr);
        }

        if (minNode == minNode->getRight()) {
            min = nullptr;
        }
        else {
            minNode->getLeft()->setRight(minNode->getRight());
            minNode->getRight()->setLeft(minNode->getLeft());
            min = minNode->getRight();
            consolidate();
        }

        numNodes--;
    }

    return minNode;
}

void FibHeap::cut(FibNode *node, FibNode *parent) {
    if (node == node->getRight()) {
        parent->setChild(nullptr);
    }
    else {
        node->getLeft()->setRight(node->getRight());
        node->getRight()->setLeft(node->getLeft());

        if (parent->getChild() == node) {
            parent->setChild(node->getRight());
        }
    }
    parent->decrementDegree();

    node->setLeft(min);
    node->setRight(min->getRight());
    min->getRight()->setLeft(node);
    min->setRight(node);
    node->setParent(nullptr);
    node->setMarked(false);
    if (node->getKey() < min->getKey()) {
        min = node;
    }
}

void FibHeap::cascadingCut(FibNode *node) {
    FibNode *parent = node->getParent();
    if (parent != nullptr) {
        if (!node->isMarked()) {
            node->setMarked(true);
        }
        else {
            cut(node, parent);
            cascadingCut(parent);
        }
    }
}

void FibHeap::decreaseKey(FibNode *node, int newKey) {
    if (newKey > node->getKey()) {
        return;
    }

    node->setKey(newKey);
    FibNode *parent = node->getParent();
    if (parent != nullptr && node->getKey() < parent->getKey()) {
        cut(node, parent);
        cascadingCut(parent);
    }
    if (node->getKey() < min->getKey()) {
        min = node;
    }
}

void FibHeap::removeNode(FibNode *node) {
    if (node == nullptr) {
        return;
    }

    int key = node->getKey();
    decreaseKey(node, INT_MIN);
    extractMin();
    node->setKey(key);
}
