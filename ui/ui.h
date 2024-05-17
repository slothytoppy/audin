#ifndef UI_H
#define UI_H
#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>

typedef struct UI {
  int maxx;
  int maxy;
  bool should_close;
} UI;

typedef struct {
  char* exit_keybinds;
  unsigned char exit_keybinds_count;
} keybinds;

void init_ui(void);
void deinit_ui(void);
int maxx();
int maxy();
void renderat(int x, int y, char*);
void key_init(void);
void key_append(char);
bool handle_exit_keys(char);
bool should_close(void);
#endif // UI_H
