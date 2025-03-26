#include <ncurses.h>
#include <stdio.h>

#include "errorHandle.h"

void errorHandle(char *msg) {
    if (ncursesFlag) {
        endwin();
    }
    fprintf(stderr, "%s\n", msg);
}