#include "./common.h"

struct tag {
  char* title;
  char* album;
  char* artist;
  char* year;
} tag;

struct tag get_tag(char* filename);

int main(void) {
  char* filename = "../stuff/down_with_the_sickness.mp3";
  struct tag tag = get_tag(filename);
}

struct tag get_tag(char* filename) {
  struct tag tag = {0};
  int fd = open(filename, O_RDONLY);
  long unsigned int size = nom_get_fsize(filename);
  int length = 128;
  char* file = map_file_into_memory(filename);
  for(int i = 0; file[i] != '/'; i++) {
    file++;
    lseek(fd, i, SEEK_SET);
  }
  for(int i = 0; i < 128; i++) {
    read(fd, file, 1);
    printf("%s", file);
  }
  return tag;
}
