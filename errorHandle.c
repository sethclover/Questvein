#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>

#include "errorHandle.h"

void errorHandle(char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    if (!ncursesFlag) {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
    }
    move(0, 0);
    clear();
    printw("%s\n"
           "Press 'Q' to exit", buffer);
    while (getch() != 'Q')
        ;
    refresh();

    endwin();
}