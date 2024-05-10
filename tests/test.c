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
  char* file = calloc(1, 128 + 1);
  // lseek(fd, 9, SEEK_SET);
  for(int i = 0; i < 1000; i++) {
    lseek(fd, i, SEEK_SET);
    read(fd, file, 1);
    printf("%s", file);
  }
  fflush(stdout);
  return tag;
}
