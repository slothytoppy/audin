#include "../deps/nom/nom.h"
#include <limits.h>

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, "gcc");
  Nom_cmd cmd = {0};
  char* filepath;
  if(argc < 2) {
    nom_cmd_append_many(&cmd, 7, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", "complexaudio.c", "-o", "complexaudio");
    nom_run_sync(cmd);
    nom_cmd_reset(&cmd);
    nom_cmd_append_many(&cmd, 2, "./complexaudio", "../stuff");
    return nom_run_path(cmd);
  } else {
    char* endptr;
    unsigned ind = strtol(argv[1], &endptr, 10);
    switch(ind) {
    case 0:
      filepath = "simpleaudio.c";
      break;
    case 1:
      filepath = "complexaudio.c";
      break;
    case 2:
      filepath = "managed_audio.c";
      break;
    };
  }
  nom_cmd_append_many(&cmd, 7, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", filepath, "-o", base(filepath));
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
}
