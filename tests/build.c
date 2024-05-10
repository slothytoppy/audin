#include "../deps/nom/nom.h"
#include <limits.h>

bool ends_with(char* string, char* ext) {
  if(string == NULL || ext == NULL) {
    return false;
  }
  long int string_len = strlen(string);
  long int ext_len = strlen(ext);
  char* string_copy = string;
  string_copy = string_copy + string_len - ext_len;
  for(int i = 0; string_copy[i] && ext[i]; i++) {
    if(string_copy[i] != ext[i]) {
      return false;
    }
  }
  return true;
}

void build_ui(void) {
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 6, "gcc", "-lpthread", "ui.c", "-c", "-o", "ui.o");
  nom_run_async(cmd);
  nom_cmd_reset(&cmd);
  free(cmd.items);
}

void build_file(char* file) {
  build_ui();
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 7, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", file, "-o", base(file));
  nom_run_async(cmd);
}

void build_with_ui(char* file) {
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 8, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", "ui.o", file, "-o", base(file));
  nom_run_async(cmd);
  nom_cmd_reset(&cmd);
  free(cmd.items);
}

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, "gcc");
  Nom_cmd cmd = {0};
  struct dirent* dirent;
  DIR* dir = opendir(".");
  bool remove_all = false;
  if(argc >= 2 && strncmp(argv[1], "clean", 5) == 0) {
    remove_all = true;
  }
  while((dirent = readdir(dir))) {
    if(strlen(dirent->d_name) <= 2 && dirent->d_name[0] == '.' || dirent->d_name[1] == '.') {
      continue;
    }
    if(remove_all == false && ends_with(dirent->d_name, ".c")) {
      if(strcmp(dirent->d_name, "managed_audio.c") == 0) {
        build_ui();
        build_with_ui(dirent->d_name);
      } else if(strcmp(dirent->d_name, "ui.c") == 0) {
        build_ui();
      } else {
        build_file(dirent->d_name);
      }
    } else if(ends_with(dirent->d_name, ".c")) {
      remove(base(dirent->d_name));
      if(IS_PATH_EXIST("build.old")) {
        remove("build.old");
      }
      if(IS_PATH_EXIST("ui.o")) {
        remove("ui.o");
      }
    }
  }
}
