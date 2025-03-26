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
    clrtoeol();
    printw("%s\n", buffer);
    mvprintw(1, 0, "Press 'Q' to exit");
    while (getch() != 'Q')
        ;
    refresh();

    endwin();
}