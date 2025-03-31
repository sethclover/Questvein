#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "dungeon.h"
#include "errorHandle.h"
#include "fibonacciHeap.h"

FibHeap *createFibHeap() {
    FibHeap *heap = (FibHeap*)malloc(sizeof(FibHeap));
    if (!heap) {
        errorHandle("Error: Failed to allocate memory for Fibonacci heap");
        return NULL;
    }
    heap->numNodes = 0;
    heap->min = NULL;

    return heap;
}

FibNode *createFibNode(int key, Pos pos) {
    FibNode *node = (FibNode*)malloc(sizeof(FibNode));
    if (!node) {
        errorHandle("Error: Failed to allocate memory for Fibonacci node");
        return NULL;
    }
    node->pos = pos;
    node->key = key;
    node->degree = 0;
    node->parent = NULL;
    node->child = NULL;
    node->left = node;
    node->right = node;
    node->marked = 0;

    return node;
}

FibNode *insert(FibHeap *heap, int key, Pos pos) {
    FibNode *node = createFibNode(key, pos);
    if (!node) {
        return NULL;
    }

    if (heap->min == NULL) {
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

FibNode *getMin(FibHeap *heap) {
    return heap->min;
}

int consolidate(FibHeap *heap) {
    int maxDegree;
    if (heap->numNodes <= 1) {
        maxDegree = 1;
    }
    else {
        maxDegree = (int)(log(heap->numNodes - 1) / log(2)) + 1;
    }
    FibNode **A = (FibNode**)calloc(maxDegree, sizeof(FibNode*));
    if (!A) {
        errorHandle("Error: Failed to allocate memory for consolidation array");
        return 1;
    }

    FibNode **nodes = (FibNode**)malloc(heap->numNodes * sizeof(FibNode*));
    if (!nodes) {
        errorHandle("Error: Failed to allocate memory for nodes array");
        free(A);
        free(nodes);
        return 1;
    }
    int count = 0;

    if (heap->min) {
        FibNode *curr = heap->min;
        int safetyCounter = 0;
        int maxCount = heap->numNodes;
        
        do {
            nodes[count++] = curr;
            curr = curr->right;
            safetyCounter++;
            
            if (safetyCounter > maxCount) {
                errorHandle("Error: Possible circular reference issue detected");
                free(A);
                free(nodes);
                return 1;
            }
        } while (curr != heap->min);
    }

    for (int i = 0; i < count; i++) {
        FibNode *node = nodes[i];
        int degree = node->degree;

        while (A[degree] != NULL) {
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

            if (node->child == NULL) {
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
            A[degree] = NULL;
            degree++;   
        }
        A[degree] = node;
    }

    heap->min = NULL;
    for (int i = 0; i < maxDegree; i++) {
        if (A[i] != NULL) {
            if (heap->min == NULL) {
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

    free(A);
    free(nodes);
    return 0;
}

FibNode *extractMin(FibHeap *heap) {
    FibNode *minNode = heap->min;

    if (minNode != NULL) {
        if (minNode->child != NULL) {
            FibNode *child = minNode->child;

            do {
                FibNode *next = child->right;
                child->parent = NULL;
                child->right = heap->min->right;
                child->left = heap->min;
                heap->min->right->left = child;
                heap->min->right = child;
                child = next;
            } while (child != minNode->child);
        }
        minNode->left->right = minNode->right;
        minNode->right->left = minNode->left;

        if (minNode == minNode->right) {
            heap->min = NULL;
        }
        else {
            heap->min = minNode->right;
            if (consolidate(heap)) {
                return NULL;
            }
        }

        heap->numNodes--;
    }

    return minNode;
}

void cut(FibHeap *heap, FibNode *node, FibNode *parent) {
    if (node == node->right) {
        parent->child = NULL;
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
    node->parent = NULL;
    node->marked = 0;
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}

void cascadingCut(FibHeap *heap, FibNode *node) {
    FibNode *parent = node->parent;
    if (parent != NULL) {
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
    if (parent != NULL && node->key < parent->key) {
        cut(heap, node, parent);
        cascadingCut(heap, parent);
    }
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}

void destroyFibNode(FibNode *node) {
    if (node == NULL) {
        return;
    }
    else if (node->child != NULL) {
        FibNode *childStart = node->child;
        FibNode *childCurr = childStart;
        FibNode *childNext;
        
        do {
            childNext = childCurr->right;
            childCurr->left = NULL;
            childCurr->right = NULL;
            destroyFibNode(childCurr);
            childCurr = childNext;
        } while (childCurr != NULL && childCurr != childStart);
    }

    if (node->left && node->right && (node->left != node || node->right != node)) {
        FibNode *curr = node->right;
        FibNode *next;
        
        node->left->right = NULL;
        node->right = NULL;
        
        while (curr != NULL) {
            next = curr->right;
            curr->left = NULL;
            curr->right = NULL;
            destroyFibNode(curr);
            curr = next;
        }
    }

    free(node);
}

void destroyFibHeap(FibHeap *heap) {
    if (heap == NULL) {
        return;
    }
    else if (heap->min != NULL) {
        destroyFibNode(heap->min);
        heap->min = NULL;
    }

    free(heap);
}
