#include "./deps/nom/nom.h"

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, " gcc");
  Nom_cmd cmd = {0};

  if(!IS_PATH_EXIST("./bin")) {
    mkdir_if_not_exist("./bin");
  }

  if(!IS_PATH_EXIST("./bin/miniaudio.o")) {
    nom_cmd_append_many(&cmd, 5, "gcc", "./deps/miniaudio/extras/miniaudio_split/miniaudio.c", "-c", "-o", "./bin/miniaudio.o");
    nom_run_sync(cmd);
    nom_cmd_reset(&cmd);
  }
  nom_cmd_append_many(&cmd, 9, "gcc", "-g", "-lncurses", "-lm", "./bin/miniaudio.o", "./plug.c", "main.c", "-o", "./bin/main");
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
  nom_cmd_append_many(&cmd, 2, "./bin/main", "./stuff");
  nom_run_path(cmd);
  return 0;
}
