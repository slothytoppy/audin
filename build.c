#include "./deps/nom/nom.h"

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, " gcc");
  Nom_cmd cmd = {0};

  if(!IS_PATH_EXIST("./bin")) {
    mkdir_if_not_exist("./bin");
  }

  char* ma_source_path = "./deps/miniaudio/extras/miniaudio_split/miniaudio.c";
  enum buildtype {
    so = 0,
    exe = 1,
  };
  enum buildtype build_type = 1;
  switch(build_type) {
  case 0:
    nom_cmd_append_many(&cmd, 8, "gcc", "-g", "-fPIC", "-shared", ma_source_path, "plug.c", "-o", "./bin/plug.so");
    nom_run_sync(cmd);
    nom_cmd_reset(&cmd);
    nom_cmd_append_many(&cmd, 7, "gcc", "-g", "-lncurses", "-lm", "main.c", "-o", "./bin/main");
    break;
  case 1:
    nom_cmd_append_many(&cmd, 8, "gcc", "-g", "-lncurses", ma_source_path, "-lm", "./main.c", "-o", "./bin/main");
    break;
  }

  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
  return 0;
}
