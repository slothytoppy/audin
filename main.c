#include "./audio/audio.h"
#include "./ui/ui.h"
#include <assert.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

void send_current_song(void) {
  char* args[] = {"notify-send", "currently playing ", (char*)get_song_name(), "-t", "1500", NULL};
  pid_t pid = fork();
  if(pid == 0) {
    execvp(args[0], args);
    return;
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
  snprintf(msg, 256, "%lu/%lu", get_song_time_played_in_seconds(), get_song_length_in_seconds());
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
  init_audio();
  init_ui();
  set_volume(0.3);
  Queue queue;
  queue_init(&queue);
  queue_read_dir(&queue, "./stuff/");
  async_play_song(queue.items[get_queue_cursor()]);
  send_current_song();
  key_append('q');
  float volume = get_volume();
  while(should_close() != true) {
    int ch = getch();
    if(ch != ERR) {
      handle_exit_keys(ch);
      switch(ch) {
      case 'a':
        play_prev_song();
        send_current_song();
        break;
      case 'd':
        play_next_song();
        send_current_song();
        break;
      case 'q':
        break;
        if(get_song_time_played_in_seconds() > 5) {
          seek_to_second(get_song_time_played_in_seconds() - 5);
        }
      case 'e':
        break;
        Log("song length: %llu\n", get_song_length_in_seconds());
        Log("old played time: %llu\n", get_song_time_played_in_seconds());
        // if(GetSongPlayedTimeInSeconds() + 5 < GetSongLengthInSeconds()) {
        seek_to_second(get_song_time_played_in_seconds() + 5);
        // }
        Log("new played time: %llu\n", get_song_time_played_in_seconds());
        break;
      case 'c':
        volume -= 0.1f;
        set_volume(volume);
        if(volume < 0.0f) {
          volume += 0.1f;
        }
        break;
      case 'v':
        if(volume > 1.0f) {
          volume -= 0.1f;
        }
        volume += 0.1f;
        set_volume(volume);
        break;
      case 'm':
        toggle_mute();
        break;
      case 'p':
        toggle_pause();
        break;
      }
    }
    if(at_song_end()) {
      play_next_song();
      send_current_song();
    }
  }
  deinit_ui();
}
