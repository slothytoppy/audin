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
  printf("%s %s\n", string_copy, ext);
  return true;
}

void build_file(char* file) {
  Nom_cmd cmd = {0};
  nom_cmd_append_many(&cmd, 7, "gcc", "-lm", "-lncurses", "../bin/miniaudio.o", file, "-o", base(file));
  nom_run_async(cmd);
}

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, "gcc");
  Nom_cmd cmd = {0};
  struct dirent* dirent;
  DIR* dir;
  dir = opendir(".");
  bool remove_all = false;
  if(argc > 2 && strcmp(argv[1], "clean")) {
    remove_all = true;
  }
  while((dirent = readdir(dir))) {
    if(strlen(dirent->d_name) <= 2 && dirent->d_name[0] == '.' || dirent->d_name[1] == '.') {
      continue;
    }
    if(ends_with(dirent->d_name, ".c")) {
      if(remove_all == true) {
        remove(base(dirent->d_name));
        if(IS_PATH_EXIST("build.old")) {
          remove("build.old");
        }
      } else {
        build_file(dirent->d_name);
      }
    }
  }
}
