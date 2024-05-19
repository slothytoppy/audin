#include "./audio/audio.h"
#include "./queue/queue.h"
#include "./ui/ui.h"
#include <assert.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

void init_queue(Queue* queue, char* dirpath) {
  struct dirent* dirent;
  struct stat fi;
  DIR* dir = opendir(dirpath);
  assert(dir != NULL);
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    char* buffer = calloc(1, 4096);
    strcat(buffer, dirpath);
    if(buffer[strlen(buffer) - 1] != '/') {
      strcat(buffer, "/");
    }
    strcat(buffer, dname);
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    int result = stat(buffer, &fi);
    assert(result == 0);
    if(fi.st_mode & S_IFREG) {
      queue_append(queue, buffer);
    }
  }
}

void DecQueueCursor(Queue* queue) {
  if(queue->cursor - 1 < 0) {
    queue->cursor += 1;
  } else {
    queue->cursor -= 1;
  }
}

void IncQueueCursor(Queue* queue) {
  if(queue->cursor >= queue->count - 1) {
    queue->cursor = 0;
  } else {
    queue->cursor += 1;
  }
}

void PlayPreviousSong(Queue* queue) {
  DecQueueCursor(queue);
  AsyncPlaySong(queue->items[queue->cursor]);
}

void PlayNextSong(Queue* queue) {
  IncQueueCursor(queue);
  AsyncPlaySong(queue->items[queue->cursor]);
}

void queue_is_not_null(Queue q) {
  assert(q.count > 0 && q.capacity > 0);
  for(int i = 0; i < q.count; i++) {
    assert(q.items[i] != NULL);
  }
}

int main(void) {
  InitAudio();
  init_ui();
  SetVolume(0.3);
  Queue queue;
  queue_init(&queue);
  init_queue(&queue, "./stuff/");
  char* current_song = queue.items[queue.cursor];
  AsyncPlaySong(queue.items[queue.cursor]);
  key_append('q');
  float volume = GetVolume();
  while(should_close() != true) {
    int ch = getch();
    if(ch != ERR) {
      handle_exit_keys(ch);
      switch(ch) {
      case 'a':
        AsyncUnloadSong();
        PlayPreviousSong(&queue);
        break;
      case 'd':
        AsyncUnloadSong();
        PlayNextSong(&queue);
        break;
      case 'c':
        volume -= 0.1f;
        SetVolume(volume);
        break;
      case 'v':
        volume += 0.1f;
        SetVolume(volume);
        break;
      case 'm':
        ToggleMute();
        break;
      case ' ':
      case 'p':
        TogglePause();
        break;
      }
    }
    if(AtSongEnd()) {
      AsyncUnloadSong();
      PlayNextSong(&queue);
    }
  }
  deinit_ui();
}
