#include "./audio/audio.h"
#include "./queue/queue.h"
#include "./ui/ui.h"
#include <assert.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void init_queue(Queue* queue, char* dirpath) {
  struct dirent* dirent;
  struct stat fi;
  DIR* dir = opendir(dirpath);
  assert(dir != NULL);
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    char* buffer = calloc(1, 4096);
    strcat(buffer, dirpath);
    if(buffer[strlen(buffer) - 1] != '/') {
      strcat(buffer, "/");
    }
    strcat(buffer, dname);
    int result = stat(buffer, &fi);
    assert(result == 0);
    if(fi.st_mode & S_IFREG) {
      queue_append(queue, buffer);
    }
  }
}

void DecQueueCursor(Queue* queue) {
  if(queue->cursor - 1 <= 0) {
    queue->cursor += 1;
  } else {
    queue->cursor -= 1;
  }
}

void IncQueueCursor(Queue* queue) {
  if(queue->cursor + 1 >= queue->count - 1) {
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

void render_volume(float volume) {
  char* msg = calloc(1, 9);
  snprintf(msg, 9, "%f", volume);
  renderat(maxx() / 2, maxy() / 2, msg);
  free(msg);
}

void render_length(void) {
  char* msg = calloc(1, 16);
  snprintf(msg, 256, "%lu/%lu", GetSongPlayedTimeInSeconds(), GetSongLengthInSeconds());
  renderat(maxx() / 2, maxy() / 2 - 1, msg);
  free(msg);
}

void render_song_name(Queue queue) {
  renderat(maxx() / 2, maxy() / 2 - 2, queue.items[queue.cursor]);
}

void render_queue(Queue q) {
  char* msg = calloc(1, 4096);
  for(int i = 0; i < q.count; i++) {
    if(q.items[i] == NULL) {
      continue;
    }
    strcat(msg, q.items[i]);
    strcat(msg, "\n");
  }
  renderat(0, 0, msg);
}

int main(void) {
  InitAudio();
  init_ui();
  SetVolume(0.5);
  Queue queue;
  queue_init(&queue);
  init_queue(&queue, "./stuff/");
  for(int i = 0; i < queue.count; i++) {
    Log("queue item:%d %s\n", i, queue.items[i]);
  }
  char* current_song = queue.items[queue.cursor];
  AsyncPlaySong(current_song);
  key_append('q');
  float volume = GetVolume();
  while(should_close() != true) {
    int ch = getch();
    if(ch != ERR) {
      handle_exit_keys(ch);
      switch(ch) {
      case 'a':
        PlayPreviousSong(&queue);
        break;
      case 'd':
        PlayNextSong(&queue);
        break;
      case 'q':
        break;
        if(GetSongPlayedTimeInSeconds() > 5) {
          SeekToSecond(GetSongPlayedTimeInSeconds() - 5);
        }
      case 'e':
        break;
        Log("song length: %llu\n", GetSongLengthInSeconds());
        Log("old played time: %llu\n", GetSongPlayedTimeInSeconds());
        // if(GetSongPlayedTimeInSeconds() + 5 < GetSongLengthInSeconds()) {
        SeekToSecond(GetSongPlayedTimeInSeconds() + 5);
        // }
        Log("new played time: %llu\n", GetSongPlayedTimeInSeconds());
        break;
      case 'c':
        volume -= 0.1f;
        if(volume < 0.0f) {
          volume += 0.1f;
        }
        SetVolume(volume);
        break;
      case 'v':
        if(volume > 1.0f) {
          volume -= 0.1f;
        }
        volume += 0.1f;
        SetVolume(volume);
        break;
      case 'm':
        ToggleMute();
        break;
      case 'p':
        TogglePause();
        break;
      }
    }
    if(AtSongEnd()) {
      PlayNextSong(&queue);
    }
  }
  deinit_ui();
}
