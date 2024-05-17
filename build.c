#include "./deps/nom/nom.h"

char* ma_source_path = "./deps/miniaudio.c";

void build_miniaudio(void) {
  Nom_cmd cmd = {0};
  if(needs_rebuild(ma_source_path, "./bin/miniaudio.o")) {
    nom_cmd_append_many(&cmd, 7, "gcc", "-g", "-lm", ma_source_path, "-c", "-o", "./bin/miniaudio.o");
    nom_run_sync(cmd);
    nom_cmd_reset(&cmd);
  }
}

void build_audio(void) {
  if(!IS_PATH_EXIST("./audio/audio.h")) {
    nom_log(NOM_PANIC, "./audio/audio.h does not exist");
    return;
  }
  Nom_cmd cmd = {0};
  build_miniaudio();
  nom_cmd_append_many(&cmd, 6, "gcc", "-g", "./audio/audio.c", "-c", "-o", "./bin/audio.o");
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
}

void build_ui(void) {
  if(needs_rebuild("./ui/ui.c", "./bin/ui.o")) {
    Nom_cmd cmd = {0};
    nom_cmd_append_many(&cmd, 7, "gcc", "-g", "-lncurses", "./ui/ui.c", "-c", "-o", "./bin/ui.o");
    nom_run_sync(cmd);
    nom_cmd_reset(&cmd);
  }
}

void build_with_audio_and_ui(char* filename) {
  char* base_file = base(filename);
  Nom_cmd cmd = {0};
  build_ui();
  build_audio();
  nom_cmd_append_many(&cmd, 10, "gcc", "-g", "-lm", "-lncurses", "./bin/audio.o", "./bin/miniaudio.o", "./bin/ui.o", filename, "-o", base_file);
  nom_run_sync(cmd);
  nom_cmd_reset(&cmd);
}

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, " gcc");
  Nom_cmd cmd = {0};

  if(!IS_PATH_EXIST("./bin")) {
    mkdir_if_not_exist("./bin");
  }
  build_with_audio_and_ui("./test.c");

  return 0;
}
