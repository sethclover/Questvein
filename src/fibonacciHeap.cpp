#include <climits>
#include <cmath>

#include "dungeon.hpp"
#include "fibonacciHeap.hpp"

FibNode *insertNew(FibHeap *heap, int key, Pos pos) {
    heap->nodes.emplace_back(std::make_unique<FibNode>(key, pos));
    FibNode *node = heap->nodes.back().get();

    if (heap->min == nullptr) {
        heap->min = node;
    }
    else {
        node->right = heap->min->right;
        node->left = heap->min;
        heap->min->right->left = node;
        heap->min->right = node;

        if (node->key < heap->min->key) {
            heap->min = node;
        }
    }

    heap->numNodes++;
    return node;
}

FibNode *insertNode(FibHeap *heap, FibNode *node) {
    node->parent = nullptr;
    node->child = nullptr;
    node->left = node;
    node->right = node;
    node->degree = 0;
    node->marked = 0;

    if (heap->min == nullptr) {
        heap->min = node;
    }
    else {
        node->right = heap->min->right;
        node->left = heap->min;
        heap->min->right->left = node;
        heap->min->right = node;

        if (node->key < heap->min->key) {
            heap->min = node;
        }
    }

    heap->numNodes++;
    return node;
}

int consolidate(FibHeap *heap) {
    int maxDegree;
    if (heap->numNodes <= 1) {
        maxDegree = 1;
    }
    else {
        maxDegree = (int)(log(heap->numNodes - 1) / log(2)) + 1;
    }
    FibNode **A = new FibNode*[maxDegree]();
    FibNode **nodes = new FibNode*[heap->numNodes]();

    int count = 0;
    if (heap->min) {
        FibNode *curr = heap->min;
        do {
            nodes[count++] = curr;
            curr = curr->right;
        } while (curr != heap->min && count < heap->numNodes);
    }

    for (int i = 0; i < count; i++) {
        FibNode *node = nodes[i];
        int degree = node->degree;

        while (A[degree] != nullptr) {
            FibNode *compareNode = A[degree];

            if (node->key > compareNode->key) {
                FibNode *temp = node;
                node = compareNode;
                compareNode = temp;
            }

            compareNode->left->right = compareNode->right;
            compareNode->right->left = compareNode->left;
            compareNode->parent = node;
            compareNode->marked = 0;

            if (node->child == nullptr) {
                node->child = compareNode;
                compareNode->left = compareNode;
                compareNode->right = compareNode;
            }
            else {
                compareNode->right = node->child->right;
                compareNode->left = node->child;
                node->child->right->left = compareNode;
                node->child->right = compareNode;
            }

            node->degree++;
            A[degree] = nullptr;
            degree++;   
        }
        A[degree] = node;
    }

    heap->min = nullptr;
    for (int i = 0; i < maxDegree; i++) {
        if (A[i] != nullptr) {
            if (heap->min == nullptr) {
                heap->min = A[i];
                heap->min->left = heap->min;
                heap->min->right = heap->min;
            }
            else {
                A[i]->right = heap->min->right;
                A[i]->left = heap->min;
                heap->min->right->left = A[i];
                heap->min->right = A[i];

                if (A[i]->key < heap->min->key) {
                    heap->min = A[i];
                }
            }
        }
    }

    delete[] A;
    delete[] nodes;
    return 0;
}

FibNode *extractMin(FibHeap *heap) {
    FibNode *minNode = heap->min;

    if (minNode != nullptr) {
        if (minNode->child != nullptr) {
            FibNode *child = minNode->child;
            FibNode *startChild = child;

            do {
                FibNode *next = child->right;
                child->parent = nullptr;
                child->right = heap->min->right;
                child->left = heap->min;
                heap->min->right->left = child;
                heap->min->right = child;
                child = next;
            } while (child != startChild);

            minNode->child = nullptr;
        }

        if (minNode == minNode->right) {
            heap->min = nullptr;
        }
        else {
            minNode->left->right = minNode->right;
            minNode->right->left = minNode->left;

            heap->min = minNode->right;
            consolidate(heap);
        }

        heap->numNodes--;
    }

    return minNode;
}

void cut(FibHeap *heap, FibNode *node, FibNode *parent) {
    if (node == node->right) {
        parent->child = nullptr;
    }
    else {
        node->left->right = node->right;
        node->right->left = node->left;

        if (parent->child == node) {
            parent->child = node->right;
        }
    }
    parent->degree--;

    node->left = heap->min;
    node->right = heap->min->right;
    heap->min->right->left = node;
    heap->min->right = node;
    node->parent = nullptr;
    node->marked = 0;
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}

void cascadingCut(FibHeap *heap, FibNode *node) {
    FibNode *parent = node->parent;
    if (parent != nullptr) {
        if (!node->marked) {
            node->marked = 1;
        }
        else {
            cut(heap, node, parent);
            cascadingCut(heap, parent);
        }
    }
}

void decreaseKey(FibHeap *heap, FibNode *node, int newKey) {
    if (newKey > node->key) {
        return;
    }

    node->key = newKey;
    FibNode *parent = node->parent;
    if (parent != nullptr && node->key < parent->key) {
        cut(heap, node, parent);
        cascadingCut(heap, parent);
    }
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}

void removeNode(FibHeap *heap, FibNode *node) {
    if (node == nullptr) {
        return;
    }

    int key = node->key;
    decreaseKey(heap, node, INT_MIN);
    extractMin(heap);
    node->key = key;
}
