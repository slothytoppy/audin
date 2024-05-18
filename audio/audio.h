#ifndef AUDIO_H
#define AUDIO_H

#include "../deps/miniaudio.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/mman.h>
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
  struct audio_info {
    unsigned short seconds;
  } audio_info;
  struct audio_data {
    void* data;
    unsigned long len;
  } audio_data;
  thread thread;
} Audio;

TA_PUBLIC void InitAudio(void);
TA_PRIVATE void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
TA_PRIVATE void InitThread(thread* thread, callback callback, void* arg);
TA_PRIVATE void LockThread(thread);
TA_PRIVATE void UnlockThread(thread);
/* void* return for LoadSong, and PlaySong is for threading reasons*/
TA_PUBLIC void* PlaySong(void* filename);
TA_PUBLIC void AsyncPlaySong(char* filename);
TA_PUBLIC void AsyncUnloadSong(void);
TA_PUBLIC void TogglePause(void);
TA_PUBLIC bool AtSongEnd(void);
TA_PUBLIC void GoToSongEnd(void);
TA_PUBLIC void SetDeviceVolume(float); // sets the device volume without setting the internal volume
TA_PUBLIC void SetVolume(float);       // sets device volume and sets the internal volume
TA_PUBLIC float GetVolume(void);
TA_PUBLIC void ToggleMute(void);
TA_PUBLIC bool IsMuted(void);
TA_PUBLIC bool IsAudioReady(void);
TA_PUBLIC unsigned long int GetSongLength(void);
#endif // AUDIO_H
