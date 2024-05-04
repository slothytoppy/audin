#define MINIAUDIO_IMPLEMENTATION
#include "./deps/nom/nom.h"
#include <ncurses.h>
#include <sys/time.h>

#include <dlfcn.h>

int main(int argc, char** argv) {
  bool should_close = false;
  void* libplug = dlopen("./bin/plug.so", RTLD_NOW);
  if(libplug == NULL) {
    nom_log(NOM_PANIC, "could not open %s: %s", "./bin/plug.so", dlerror());
    return 1;
  }
  void (*plug_init)(void) = dlsym(libplug, "plug_init");
  void* (*plug_pre_update)(void) = dlsym(libplug, "plug_pre_update");
  void (*plug_post_update)(void*) = dlsym(libplug, "plug_post_update");
  plug_init();
  while(!should_close) {
    char ch = getch();
    switch(ch) {
    case 'r': {
      void* state = plug_pre_update();
      plug_post_update(state);
      break;
    }
    case 'q':
      should_close = true;
      break;
    }
  }
  return 0;
}
