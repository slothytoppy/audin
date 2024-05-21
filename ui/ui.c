#include "./ui.h"

UI ui = {0};

keybinds keys = {0};

void init_ui(void) {
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, true);
  nodelay(stdscr, true);
  ui.maxy = getmaxy(stdscr);
  ui.maxx = getmaxx(stdscr);
}

void deinit_ui(void) {
  curs_set(1);
  noraw();
  echo();
  keypad(stdscr, false);
  nodelay(stdscr, false);
  endwin();
}

int maxx(void) {
  return ui.maxx;
}

int maxy(void) {
  return ui.maxy;
}

void renderat(int x, int y, char* fmt) {
  assert(x <= maxx() && y <= maxy() && fmt != NULL);
  mvprintw(y, x, "%s", fmt);
}

void key_init(void) {
  keys.exit_keybinds_count += 1;
  keys.exit_keybinds = calloc(1, sizeof(char));
}

void key_append(char key) {
  keys.exit_keybinds_count += 1;
  keys.exit_keybinds = realloc(keys.exit_keybinds, keys.exit_keybinds_count);
  keys.exit_keybinds[keys.exit_keybinds_count - 1] = key;
}

bool handle_exit_keys(char ch) {
  for(int i = 0; i < keys.exit_keybinds_count; i++) {
    if(ch == keys.exit_keybinds[i]) {
      ui.should_close = true;
      return true;
    }
  }
  return false;
}

bool should_close(void) {
  return ui.should_close;
}
