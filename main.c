#include "./audio/audio.h"
#include "./ui/ui.h"
#include <assert.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

void send_current_song(char* song_name) {
  char* args[] = {"notify-send", "currently playing ", song_name, "-t", "2000", NULL};
  pid_t pid = fork();
  if(pid == 0) {
    execvp(args[0], args);
    return;
  }
}

void render_volume() {
  char* msg = calloc(1, 9);
  snprintf(msg, 9, "%f", get_volume());
  renderat(maxx() / 2, maxy() / 2, msg);
  free(msg);
}

void render_length(void) {
  char* msg = calloc(1, 256);
  char* played_minutes = calloc(1, 5);
  char* played_seconds = calloc(1, 5);
  char* total_minutes = calloc(1, 5);
  char* total_seconds = calloc(1, 5);
  unsigned long remaining_time = get_song_time_played_in_seconds();
  unsigned long song_length = get_song_length_in_seconds();
  if(remaining_time / 60 < 10) {
    snprintf(played_minutes, 5, "0%lu:", remaining_time / 60);
  } else {
    snprintf(played_minutes, 5, "%lu:", remaining_time / 60);
  }
  if(remaining_time % 60 < 10) {
    snprintf(played_seconds, 5, "0%lu/", remaining_time % 60);
  } else {
    snprintf(played_seconds, 5, "%lu/", remaining_time % 60);
  }
  if(song_length / 60 < 10) {
    snprintf(total_minutes, 5, "0%lu:", song_length / 60);
  } else {
    snprintf(total_minutes, 5, "%lu:", song_length / 60);
  }
  if(song_length % 60 < 10) {
    snprintf(total_seconds, 5, "0%lu", song_length % 60);
  } else {
    snprintf(total_seconds, 5, "%lu", song_length % 60);
  }
  snprintf(msg, 256, "%s%s%s%s\n", played_minutes, played_seconds, total_minutes, total_seconds);
  renderat(maxx() / 2, maxy() / 2 - 1, msg);

  free(msg);
  free(total_seconds);
  free(total_minutes);
  free(played_seconds);
  free(played_minutes);
}

void render_song_name(void) {
  renderat(maxx() / 2, maxy() / 2 - 2, get_song_name());
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
  Queue queue;
  queue_init(&queue);
  queue_read_dir(&queue, "./stuff/");
  assert(queue.count > 0);
  for(int i = 0; i < queue.count; i++) {
    assert(queue.items[i] != NULL);
  }
  async_play_song(queue.items[queue.cursor]);
  send_current_song(queue.items[queue.cursor]);
  key_append(' ');
  float volume = get_volume();
  while(should_close() != true) {
    int ch = getch();
    if(ch != ERR) {
      handle_exit_keys(ch);
      switch(ch) {
      case 'a':
        play_prev_song();
        send_current_song(get_song_name());
        break;
      case 'd':
        play_next_song();
        send_current_song(get_song_name());
        break;
      case 'q':
        if(get_song_time_played_in_seconds() > 5) {
          seek_to_second(get_song_time_played_in_seconds() - 5);
        }
        break;
      case 'e':
        if(get_song_time_played_in_seconds() + 5 < get_song_length_in_seconds()) {
          seek_to_second(get_song_time_played_in_seconds() + 5);
        }
        break;
      case 'c':
        if(volume > 0.0f) {
          volume -= 0.1f;
          set_volume(volume);
        }
        break;
      case 'v':
        if(volume < 1.0f) {
          volume += 0.1f;
          set_volume(volume);
        }
        break;
      case 'm':
        toggle_mute();
        break;
      case 'p':
        toggle_pause();
        break;
      case 's':
        seek_to_second(20);
        break;
      }
    }
    if(at_song_end()) {
      play_next_song();
      send_current_song(get_song_name());
    }
    clear();
    usleep(600);
    /*
    render_length();
    render_volume();
    render_song_name();
    */
  }
  deinit_ui();
}
