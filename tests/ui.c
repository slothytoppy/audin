#include "ui.h"

WINDOW* win = NULL;

void ncurses_init(void) {
  win = initscr();
  win = open_window();
  raw();
  noecho();
  keypad(win, true);
}

WINDOW* open_window(void) {
  return newwin(0, 0, 0, 0);
}

void close_window(WINDOW* win) {
  delwin(win);
}

void ncurses_deinit(void) {
  if(win != NULL) {
    delwin(win);
  }
  echo();
  noraw();
}

void renderat(int x, int y, char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  move(y, x);
  deleteln();
  printw("%s", fmt);
  va_end(ap);
  refresh();
}

void clearat(int x, int y) {
  wmove(win, y, y);
  wdeleteln(win);
}

void rendervolume(Position pos, float volume) {
  char* buff = calloc(1, 9);
  snprintf(buff, 9, "%f", volume);
}

void SetPos(Position* pos, int x, int y) {
  pos->x = x;
  pos->y = y;
  return;
}

int maxx(void) {
  return getmaxx(win);
}

int maxy(void) {
  return getmaxy(win);
}
