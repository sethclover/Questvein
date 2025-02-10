#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dungeon.h"

#define SCALE 0.12

typedef struct {
    float x;
    float y;
} Gradient;

Gradient grads[MAX_HEIGHT][MAX_WIDTH];

void initGrads() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            float angle = ((float)rand() / RAND_MAX) * 2.0 * M_PI;
            grads[i][j].x = cos(angle);
            grads[i][j].y = sin(angle);
        }
    }
}

float dotGridGrad(int ix, int iy, float x, float y) {
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    return (dx * grads[iy][ix].x + dy * grads[iy][ix].y);
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float perlin(float x, float y) {
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float n0 = dotGridGrad(x0, y0, x, y);
    float n1 = dotGridGrad(x1, y0, x, y);
    float ix0 = lerp(n0, n1, fade(x - (float)x0));

    n0 = dotGridGrad(x0, y1, x, y);
    n1 = dotGridGrad(x1, y1, x, y);
    float ix1 = lerp(n0, n1, fade(x - (float)x0));

    return lerp(ix0, ix1, fade(y - (float)y0));
}

void generateHardness(Tile dungeon[MAX_HEIGHT][MAX_WIDTH]) {
    initGrads();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            float value = perlin(j * SCALE, i * SCALE);

            int sign = (value < 0) ? -1 : 1;
            value = (1 - (1 - fabs(value)) * (1 - fabs(value))) * sign;
            value = (int)(((value + 1.0) / 2.0) * 253) + 1;
            dungeon[i][j].hardness = value;
        }
    }
}
