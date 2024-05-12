#include "ui.h"

WINDOW* win = NULL;

void ncurses_init(void) {
  win = initscr();
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

void renderat(int x, int y, char* fmt) {
  assert(x <= maxx() && y <= maxy());
  move(y, x);
  deleteln();
  printw("%s", fmt);
  refresh();
}

void clearat(int x, int y) {
  wmove(win, y, y);
  wdeleteln(win);
}

void clearscr(WINDOW* win) {
  if(win == NULL) {
    erase();
  }
  werase(win);
}

void rendervolume(Position pos, float volume) {
  char* buff = calloc(1, 9);
  snprintf(buff, 9, "%f", volume);
  renderat(pos.x, pos.y, buff);
  free(buff);
}

void renderseconds(int x, int y, int seconds) {
  char* buff = calloc(1, 4);
  snprintf(buff, 9, "%d", seconds);
  renderat(x, y, buff);
  free(buff);
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

void getcenter(int* x, int* y) {
  *x = maxx() / 2;
  *y = maxy() / 2;
}
