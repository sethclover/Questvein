#pragma once

#define MAX_LENGTH 80
#define MAX_WIDTH 21
#define ATTEMPTS 1000
#define FLOOR '.'
#define CORRIDOR '#'
#define STAIR_UP '<'
#define STAIR_DOWN '>'
#define ROCK ' ' 

typedef struct {
    int length;
    int width;
    int x;
    int y;
} Room;

typedef struct {
    char type;
    int hardness;
} Sum;