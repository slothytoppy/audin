#include "../deps/nom/nom.h"

int main(void) {
  rebuild1(__FILE__, "gcc");
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 7, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", "complexaudio.c", "-o", "complexaudio");
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
  nom_cmd_append_many(&cmd, 2, "./complexaudio", "../stuff");
  nom_run_path(cmd);
}
