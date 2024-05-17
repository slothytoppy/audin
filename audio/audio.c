#include "./audio.h"
#include <stdio.h>

Audio audio = {0};

TA_PUBLIC void GoToSongEnd(void) {
  ma_uint64 length = GetSongLength();
  ma_decoder_seek_to_pcm_frame(&audio.decoder, length);
}

TA_PUBLIC bool AtSongEnd() {
  if(audio.cursor > 0 && audio.length > 0 && audio.cursor == audio.length) {
    audio.at_end = true;
    return true;
  }
  return false;
}

TA_PRIVATE void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_data_source* pdatasource = (ma_data_source*)pDevice->pUserData;
  if(AtSongEnd()) {
    return;
  }
  if(audio.playing == MA_FALSE) {
    return;
  }
  ma_uint64 framesRead;
  ma_result result = ma_data_source_read_pcm_frames(pdatasource, pOutput, frameCount, &framesRead);
  assert(result == MA_SUCCESS);
  result = ma_decoder_get_cursor_in_pcm_frames(&audio.decoder, &audio.cursor);
  assert(result == MA_SUCCESS);
  if(frameCount != framesRead) {
    audio.at_end = true;
  }

  (void)pInput;
}

TA_PUBLIC void InitAudio(void) {
  ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
  device_config.playback.format = ma_format_f32;
  device_config.playback.channels = 2;
  device_config.sampleRate = 0;
  device_config.dataCallback = data_callback;
  device_config.pUserData = NULL;
  ma_result result = MA_ERROR;
  result = ma_device_init(NULL, &device_config, &audio.device);
  assert(result == MA_SUCCESS);
  result = ma_device_start(&audio.device);
  assert(result == MA_SUCCESS);
  audio.is_initialized = true;
}

TA_PRIVATE void InitThread(thread* thread, callback callback, void* arg) {
  pthread_attr_init(&thread->attr);
  pthread_create(&thread->thread, &thread->attr, callback, arg);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&thread->mutex, &attr);
}

TA_PRIVATE void LockThread(thread thread) {
  pthread_mutex_lock(&thread.mutex);
}

TA_PRIVATE void UnlockThread(thread thread) {
  pthread_mutex_unlock(&thread.mutex);
}

TA_PUBLIC bool IsAudioReady(void) {
  return audio.is_initialized;
}

TA_PUBLIC unsigned long int GetSongLength(void) {
  ma_uint64 length;
  ma_result result = ma_decoder_get_length_in_pcm_frames(&audio.decoder, &length);
  return length;
}

TA_PUBLIC void* PlaySong(void* filename) {
  assert(IsAudioReady() == true);
  int fd = open(filename, O_RDONLY);
  struct stat fi;
  if(stat(filename, &fi) < 0) {
    return false;
  }
  audio.audio_data.data = mmap(audio.audio_data.data, fi.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  audio.audio_data.len = fi.st_size;
  assert(audio.audio_data.data != MAP_FAILED);
  ma_decoder_config config = ma_decoder_config_init_default();
  ma_result result;
  result = ma_decoder_init_memory(audio.audio_data.data, audio.audio_data.len, &config, &audio.decoder);
  assert(result == MA_SUCCESS);
  audio.device.pUserData = &audio.decoder;
  audio.playing = true;
  return NULL;
}

void AsyncPlaySong(char* filename) {
  thread thread;
  InitThread(&thread, PlaySong, filename);
  pthread_join(thread.thread, NULL);
}

TA_PUBLIC void SetVolume(float volume) {
  assert(IsAudioReady() == true);
  ma_result result = ma_device_set_master_volume(&audio.device, volume);
  assert(result == MA_SUCCESS);
  audio.volume = volume;
}

TA_PUBLIC float GetVolume(void) {
  assert(IsAudioReady() == true);
  float volume;
  ma_result result = ma_device_get_master_volume(&audio.device, &volume);
  assert(result == MA_SUCCESS);
  return volume;
}
