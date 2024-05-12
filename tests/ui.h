#include <assert.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Position {
  int x, y;
} Position;

void ui_init(void);
void ui_deinit(void);
void ncurses_init(void); // calls __ncurses_init()
void* __ncurses_init(void* a);
void ncurses_deinit(void);
WINDOW* open_window(void);
void close_window(WINDOW* win);
void renderat(int x, int y, char* fmt);
void clearat(int x, int y);
void rendervolume(Position pos, float);
void SetPos(Position* pos, int, int);
int maxx(void);
int maxy(void);
void getcenter(int*, int*);
