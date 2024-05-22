#ifndef AUDIO_H
#define AUDIO_H

#include "../deps/miniaudio.h"
#include "../queue/queue.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>

#define TA_PRIVATE static
#define TA_PUBLIC extern

typedef void* (*callback)(void*);

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;
  pthread_mutex_t mutex;
  callback callback;
  void* arg;
} thread;

typedef struct {
  ma_device device;
  ma_context ctx;
  ma_decoder decoder;
  bool is_initialized;
  bool at_end;
  bool playing;
  float volume;
  bool muted;
  struct {
    ma_uint64 cursor;
    ma_uint64 length;
  };
  char* song_name;
  thread thread;
  Queue queue;
} Audio;

TA_PUBLIC void init_audio(void);
TA_PRIVATE void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
TA_PRIVATE void init_thread(thread* thread, callback callback, void* arg);
TA_PRIVATE void lock_thread(thread);
TA_PRIVATE void unlock_thread(thread);
/* void* return for LoadSong, and PlaySong is for threading reasons*/
TA_PUBLIC void* play_song(void* filename);
TA_PUBLIC void async_play_song(char* filename);
TA_PUBLIC void async_unload_song(void);
TA_PUBLIC void toggle_pause(void);
TA_PUBLIC bool at_song_end(void);
TA_PUBLIC void go_to_song_end(void);
TA_PUBLIC void set_device_volume(float); // sets the device volume without setting the internal volume
TA_PUBLIC void set_volume(float);        // sets device volume and sets the internal volume
TA_PUBLIC float get_volume(void);
TA_PUBLIC void toggle_mute(void);
TA_PUBLIC bool is_muted(void);
TA_PUBLIC bool is_audio_ready(void);
TA_PUBLIC unsigned long int get_song_length(void);
TA_PUBLIC unsigned long int get_song_length_in_seconds(void);
TA_PUBLIC unsigned long int get_song_time_played(void);
TA_PUBLIC unsigned long int get_song_time_played_in_seconds(void);
TA_PUBLIC void seek_to_frame(unsigned long int);
TA_PUBLIC void seek_to_second(unsigned long int);
TA_PUBLIC void Log(char* fmt, ...);
// queue
TA_PUBLIC bool queue_read_dir(Queue* queue, char* dirpath);
TA_PUBLIC void play_prev_song(void);
TA_PUBLIC void play_next_song(void);
TA_PUBLIC unsigned long long get_queue_cursor(void);
TA_PUBLIC bool set_queue_cursor(unsigned long long);
TA_PUBLIC char* get_song_name(void);
TA_PUBLIC bool reset_queue(Queue* queue);
TA_PUBLIC void dump_audio_info(void);

#endif // AUDIO_H
