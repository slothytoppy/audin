#include "./audio/audio.h"
#include "./deps/nom/nom.h"
#include "./ui/ui.h"
#include <stdio.h>

int main(void) {
  open_ncurses();
  InitAudio();
  SetVolume(0.3);
  Nom_cmd cmd = {0};
  struct dirent* dirent;
  DIR* dir = opendir("./stuff/");
  struct stat fi;
  assert(dir != NULL);
  while((dirent = readdir(dir))) {
    char* buff = calloc(1, 4096);
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    strcat(buff, "./stuff/");
    strcat(buff, dname);
    if(stat(buff, &fi) < 0) {
      printf("file %s %s\n", buff, strerror(errno));
      return 1;
    }
    if(fi.st_mode & S_IFREG) {
      nom_cmd_append(&cmd, buff);
    }
  }
  unsigned long cursor = 0;
  AsyncPlaySong("./stuff/Eminem - Parking Lot (Skit).mp3");
  key_append('q');
  while(!should_close()) {
    int ch = getch();
    handle_exit_keys(ch);
    if(AtSongEnd()) {
      cursor += 1;
      AsyncPlaySong(cmd.items[cursor]);
    }
  }
  close_ncurses();
}
