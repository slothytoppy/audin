#include "./audio.h"
#include <stdio.h>
#include <stdlib.h>

Audio audio = {0};

__attribute__((format(printf, 1, 2))) TA_PUBLIC void Log(char* msg, ...) {
  FILE* fp = fopen("log", "a+");
  va_list args;
  va_start(args, msg);
  vfprintf(fp, msg, args);
  va_end(args);
  fclose(fp);
}

TA_PUBLIC void GoToSongEnd(void) {
  assert(IsAudioReady() == true);
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.decoder, audio.length);
  assert(result == MA_SUCCESS);
  audio.at_end = true;
}

TA_PUBLIC bool AtSongEnd() {
  if(audio.at_end == true || audio.cursor > 0 && audio.cursor == audio.length) {
    audio.at_end = true;
    return true;
  }
  return false;
}

TA_PRIVATE void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_data_source* pdatasource = (ma_data_source*)pDevice->pUserData;
  ma_result result = MA_ERROR;
  if(AtSongEnd()) {
    return;
  }
  if(audio.playing == MA_FALSE) {
    return;
  }
  ma_uint64 framesRead = 0;
  result = ma_data_source_read_pcm_frames(pdatasource, pOutput, frameCount, &framesRead);
  assert(result == MA_SUCCESS);
  result = ma_decoder_get_cursor_in_pcm_frames(&audio.decoder, &audio.cursor);
  assert(result == MA_SUCCESS);
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
  SetVolume(0.5f);
}

TA_PRIVATE void InitThread(thread* thread, callback callback, void* arg) {
  assert(IsAudioReady() == true);
  pthread_attr_init(&thread->attr);
  pthread_create(&thread->thread, &thread->attr, callback, arg);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&thread->mutex, &attr);
  audio.thread = *thread;
}

TA_PRIVATE void LockThread(thread thread) {
  assert(IsAudioReady() == true);
  pthread_mutex_lock(&thread.mutex);
}

TA_PRIVATE void UnlockThread(thread thread) {
  assert(IsAudioReady() == true);
  pthread_mutex_unlock(&thread.mutex);
}

TA_PUBLIC bool IsAudioReady(void) {
  return audio.is_initialized;
}

TA_PUBLIC unsigned long int GetSongLength(void) {
  assert(IsAudioReady() == true);
  ma_uint64 length = 0;
  ma_result result = ma_decoder_get_length_in_pcm_frames(&audio.decoder, &length);
  if(result != MA_SUCCESS) {
    Log("song:%s error %d:%s\n", audio.song_name, result, ma_result_description(result));
    exit(1);
  }
  assert(result == MA_SUCCESS);
  return length;
}

TA_PUBLIC unsigned long int GetSongLengthInSeconds(void) {
  ma_uint64 length = GetSongLength() / audio.decoder.outputSampleRate;
  return length;
}

TA_PUBLIC unsigned long int GetSampleRate(void) {
  return audio.decoder.outputSampleRate;
}

TA_PUBLIC unsigned long int GetSongPlayedTime(void) {
  assert(IsAudioReady() == true);
  return audio.cursor;
}

TA_PUBLIC unsigned long int GetSongPlayedTimeInSeconds(void) {
  ma_uint64 length = GetSongPlayedTime() / audio.decoder.outputSampleRate;
  return length % 60;
}

TA_PUBLIC unsigned long int GetCursor(void) {
  return GetSongPlayedTime();
}

TA_PUBLIC bool SeekToFrame(unsigned long int frame) {
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.decoder, frame);
  audio.cursor = frame;
  return result;
}

TA_PUBLIC void SeekToSecond(unsigned long seconds) {
  unsigned long frame = audio.decoder.outputSampleRate * (ma_uint64)seconds;
  assert(frame <= audio.length);
  Log("second %lu frames %lu\n", (unsigned long)seconds, frame);
  // ma_mix_pcm_frames_f32()
  SeekToFrame(frame);
  return;
}

/*
TA_PUBLIC void SeekToSecond(unsigned long seconds) {
  thread thread;
  InitThread(&thread, _SeekToSecond, (void*)seconds);
  pthread_join(thread.thread, NULL);
}
*/

TA_PUBLIC void* PlaySong(void* filename) {
  assert(IsAudioReady() == true);
  ma_decoder_config config = ma_decoder_config_init_default();
  ma_result result = ma_decoder_init_file(filename, &config, &audio.decoder);
  if(result != MA_SUCCESS) {
    Log("PlaySong %d:%s\n", result, ma_result_description(result));
  }
  assert(result == MA_SUCCESS);
  audio.length = GetSongLength();
  audio.device.pUserData = &audio.decoder;
  audio.playing = true;
  audio.song_name = filename;
  return NULL;
}

void AsyncPlaySong(char* filename) {
  assert(IsAudioReady() == true);
  thread thread;
  InitThread(&thread, PlaySong, filename);
  audio.thread = thread;
}

void AsyncUnloadSong(void) {
  assert(IsAudioReady() == true);
  ma_result result = ma_decoder_uninit(&audio.decoder);
  assert(result == MA_SUCCESS);
}

void TogglePause(void) {
  assert(IsAudioReady() == true);
  audio.playing = !audio.playing;
}

void ToggleMute(void) {
  assert(IsAudioReady() == true);
  // assert(0 && "TODO: fix muting messing with volume");
  audio.muted = !audio.muted;
  if(!audio.muted) {
    SetVolume(audio.volume);
  } else {
    SetDeviceVolume(0);
  }
}

bool IsMuted(void) {
  if(audio.volume == 0 || audio.muted == true) {
    return true;
  }
  return false;
}

TA_PRIVATE bool AudioWithinRange(float volume) {
  assert(IsAudioReady() == true);
  if(volume <= 1.0 && volume >= 0.0) {
    return true;
  }
  return false;
}

TA_PUBLIC void SetDeviceVolume(float volume) {
  assert(IsAudioReady() == true);
  ma_result result = ma_device_set_master_volume(&audio.device, volume);
  assert(result == MA_SUCCESS);
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
