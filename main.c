#include "./audio/audio.h"
#include "./ui/ui.h"
#include <assert.h>
#include <ncurses.h>
#include <unistd.h>

void PlayPreviousSong(Queue* queue) {
  if(queue->cursor - 1 < 0) {
    queue->cursor += 1;
  } else {
    queue->cursor -= 1;
  }
  async_play_song(queue->items[queue->cursor]);
}

void PlayNextSong(Queue* queue) {
  if(queue->cursor + 1 >= queue->count) {
    queue->cursor = 0;
  } else {
    queue->cursor += 1;
  }
  async_play_song(queue->items[queue->cursor]);
}

void queue_is_not_null(Queue q) {
  assert(q.capacity > 0);
  for(int i = 0; i < q.count; i++) {
    assert(q.items[i] != NULL);
  }
}

void send_current_song(void) {
  char* args[] = {"notify-send", "currently playing ", (char*)get_song_name(), "-t", "1500", NULL};
  pid_t pid = fork();
  if(pid == 0) {
    execvp(args[0], args);
    return;
  }
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
      case 'c':
        volume -= 0.1f;
        set_volume(volume);
        break;
      case 'v':
        volume += 0.1f;
        set_volume(volume);
        break;
      case 'm':
        toggle_mute();
        break;
      case ' ':
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
