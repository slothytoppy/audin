#include "./audio.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

Audio audio = {0};

void go_to_song_end(void) {
  assert(is_audio_ready() == true);
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.decoder, audio.length);
  if(result != MA_SUCCESS) {
    Log("failed to seek to %llu with error %s\n", audio.length, ma_result_description(result));
  }
}

__attribute__((format(printf, 1, 2))) TA_PUBLIC void Log(char* msg, ...) {
  FILE* fp = fopen("log", "a+");
  va_list args;
  va_start(args, msg);
  vfprintf(fp, msg, args);
  va_end(args);
  fclose(fp);
}

bool at_song_end() {
  // dont do assert(IsAudioReady()==true) here since its called in the data_callback and may be called before the device has any frames to read, which would fail the assert
  if(audio.at_end == true || audio.cursor > 0 && audio.cursor == audio.length) {
    audio.at_end = true;
    return true;
  }
  return false;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pdatasource = (ma_data_source*)pDevice->pUserData;
  ma_result result = MA_ERROR;
  if(at_song_end()) {
    return;
  }
  if(audio.playing == MA_FALSE) {
    return;
  }
  ma_uint64 framesRead;
  result = ma_decoder_read_pcm_frames(pdatasource, pOutput, frameCount, &framesRead);
  if(result != MA_SUCCESS) {
    Log("failed to read pcm frames in data callback, framecount: %u frames read: %llu error %s\n", frameCount, framesRead, ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  result = ma_decoder_get_cursor_in_pcm_frames(&audio.decoder, &audio.cursor);
  if(result != MA_SUCCESS) {
    Log("failed to get cursor in pcm frames in data callback, cursor: %llu error %s\n", audio.cursor, ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  (void)pInput;
}

void init_audio(void) {
  ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
  device_config.playback.format = ma_format_f32;
  device_config.playback.channels = 2;
  device_config.sampleRate = 0;
  device_config.dataCallback = data_callback;
  device_config.pUserData = NULL;
  ma_result result = MA_ERROR;
  result = ma_device_init(NULL, &device_config, &audio.device);
  if(result != MA_SUCCESS) {
    Log("in InitAudio failed to initialize device with error %s\n", ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  result = ma_device_start(&audio.device);
  if(result != MA_SUCCESS) {
    Log("in InitAudio failed to start device with error %s\n", ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  audio.is_initialized = true;
  set_volume(0.5f);
}

void init_thread(thread* thread, callback callback, void* arg) {
  assert(is_audio_ready() == true);
  pthread_attr_init(&thread->attr);
  pthread_create(&thread->thread, &thread->attr, callback, arg);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&thread->mutex, &attr);
  audio.thread = *thread;
}

void lock_thread(thread thread) {
  assert(is_audio_ready() == true);
  pthread_mutex_lock(&thread.mutex);
}

void unlock_thread(thread thread) {
  assert(is_audio_ready() == true);
  pthread_mutex_unlock(&thread.mutex);
}

bool is_audio_ready(void) {
  return audio.is_initialized;
}

unsigned long int get_song_length(void) {
  assert(is_audio_ready() == true);
  ma_uint64 length;
  ma_result result = ma_decoder_get_length_in_pcm_frames(&audio.decoder, &length);
  if(result != MA_SUCCESS) {
    Log("failed to get length of the song with error %s\n", ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  return length;
}

void* play_song(void* filename) {
  assert(is_audio_ready() == true);
  struct stat fi;
  if(stat(filename, &fi) < 0) {
    return false;
  }
  ma_decoder_config config = ma_decoder_config_init_default();
  ma_result result;
  result = ma_decoder_init_file(filename, &config, &audio.decoder);
  if(result != MA_SUCCESS) {
    Log("failed to decode %s with error %s\n", (char*)filename, ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  audio.length = get_song_length();
}

TA_PUBLIC unsigned long int get_song_length_in_seconds(void) {
  ma_uint64 length = get_song_length() / audio.decoder.outputSampleRate;
  return length;
}

TA_PUBLIC unsigned long int get_sample_rate(void) {
  return audio.decoder.outputSampleRate;
}

TA_PUBLIC unsigned long int get_song_time_played(void) {
  assert(is_audio_ready() == true);
  return audio.cursor;
}

TA_PUBLIC unsigned long int get_song_time_played_in_seconds(void) {
  ma_uint64 length = get_song_time_played() / audio.decoder.outputSampleRate;
  return length % 60;
}

TA_PUBLIC unsigned long int get_cursor(void) {
  return get_song_time_played();
}

TA_PUBLIC bool seek_to_frame(unsigned long int frame) {
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.decoder, frame);
  audio.cursor = frame;
  return result;
}

TA_PUBLIC void seek_to_second(unsigned long seconds) {
  unsigned long frame = audio.decoder.outputSampleRate * (ma_uint64)seconds;
  assert(frame <= audio.length);
  Log("second %lu frames %lu\n", (unsigned long)seconds, frame);
  // ma_mix_pcm_frames_f32()
  seek_to_frame(frame);
  return;
}

/*
TA_PUBLIC void SeekToSecond(unsigned long seconds) {
  thread thread;
  InitThread(&thread, _SeekToSecond, (void*)seconds);
  pthread_join(thread.thread, NULL);
}
*/

void async_play_song(char* filename) {
  assert(is_audio_ready() == true);
  thread thread;
  init_thread(&thread, play_song, filename);
  pthread_join(thread.thread, NULL);
  audio.thread = thread;
}

void async_unload_song(void) {
  assert(is_audio_ready() == true);
  ma_decoder_uninit(&audio.decoder);
}

void toggle_pause(void) {
  assert(is_audio_ready() == true);
  audio.playing = !audio.playing;
}

void toggle_mute(void) {
  assert(is_audio_ready() == true);
  audio.muted = !audio.muted;
  if(!audio.muted) {
    set_volume(audio.volume);
  } else {
    set_device_volume(0);
  }
}

bool is_muted(void) {
  assert(is_audio_ready() == true);
  if(audio.volume == 0 || audio.muted == true) {
    return true;
  }
  return false;
}

bool audio_within_range(float volume) {
  assert(is_audio_ready() == true);
  if(volume <= 1.0 && volume >= 0.0) {
    return true;
  }
  return false;
}

void set_device_volume(float volume) {
  assert(is_audio_ready() == true);
  if(audio_within_range(volume)) {
    ma_result result = ma_device_set_master_volume(&audio.device, volume);
    if(result != MA_SUCCESS) {
      Log("failed to set master volume with error %s\n", ma_result_description(result));
    }
    assert(result == MA_SUCCESS);
  }
}

void set_volume(float volume) {
  assert(is_audio_ready() == true);
  if(audio_within_range(volume)) {
    ma_result result = ma_device_set_master_volume(&audio.device, volume);
    if(result != MA_SUCCESS) {
      Log("failed to set master volume with error %s\n", ma_result_description(result));
    }
    audio.volume = volume;
  }
}

float get_volume(void) {
  assert(is_audio_ready() == true);
  float volume;
  ma_result result = ma_device_get_master_volume(&audio.device, &volume);
  if(result != MA_SUCCESS) {
    Log("failed to set master volume with error %s\n", ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  return volume;
}

/*
************** QUEUE START **************
*/

bool queue_read_dir(Queue* queue, char* dirpath) {
  struct stat fi;
  struct dirent* dirent;
  if(stat(dirpath, &fi) < 0) {
    Log("error in queue_read_dir %s\n", strerror(errno));
  }
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
  audio.queue = *queue;
  return true;
}

char* get_song_name(void) {
  return audio.queue.items[audio.queue.cursor];
}

void play_prev_song(void) {
  if(audio.queue.cursor - 1 < 0) {
    return;
  } else {
    audio.queue.cursor -= 1;
  }
  async_play_song(audio.queue.items[audio.queue.cursor]);
}

void play_next_song(void) {
  if(audio.queue.cursor + 1 > audio.queue.count) {
    return;
  } else {
    audio.queue.cursor += 1;
  }
  async_play_song(audio.queue.items[audio.queue.cursor]);
}

unsigned long long get_queue_cursor(void) {
  return audio.queue.cursor;
}

bool set_queue_cursor(unsigned long long cursor) {
  if(cursor > audio.queue.count || cursor < 0) {
    return false;
  }
  audio.queue.cursor = cursor;
  return true;
}

bool reset_queue(Queue* queue) {
  queue->items = memset(queue->items, '\0', queue->count * sizeof(void*));
  queue->count = 0;
  queue->cursor = 0;
  queue->capacity = 0;
  return true;
}
