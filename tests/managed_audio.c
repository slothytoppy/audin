#include "common.h"

typedef void (*LogCallBackType)(ma_uint8, char*);

typedef struct Audio {
  struct base {
    ma_device device;
    ma_device_config device_config;
    ma_decoder decoder;
    ma_sound sound;
    ma_sound_config sound_config;
    ma_float volume;
    ma_engine engine;
    ma_engine_config engine_config;
    ma_uint64 frame_count;
    ma_mutex mutex;
    ma_result result;
  } base;
  struct internal {
    float volume;     // 0..1
    ma_uint64 length; // length of the song in seconds
    ma_bool8 playing; // no paused field because !playing means it is paused
  } internal;
  struct time {
    ma_uint64 frame_count;
    ma_uint8 minutes;
    ma_uint8 seconds;
  } time;
  struct callbacks {
    LogCallBackType logcallback;
  } callbacks;
} Audio;

Audio audio = {0};

typedef enum {
  NONE = 0,
  INFO = 1,
  DEBUG = 2,
  WARNING = 3,
  ERROR = 4,
} log_level;

void Log(ma_uint8 level, char* msg) {
  char* lvl = "";
  switch(level) {
  case NONE:
    break;
  case INFO:
    lvl = "INFO ";
    break;
  case DEBUG:
    lvl = "DEBUG ";
    break;
  case WARNING:
    lvl = "WARNING ";
    break;
  case ERROR:
    lvl = "ERROR ";
    break;
  }
  fprintf(stdout, "%s\n", lvl);
}

void InitLogCallback(LogCallBackType callback) {
  audio.callbacks.logcallback = callback;
}

void InitAudio(void) {
  ma_result result;
  audio.base.device_config = ma_device_config_init(ma_device_type_playback);
  audio.base.device_config.playback.format = audio.base.decoder.outputFormat;
  audio.base.device_config.playback.channels = audio.base.decoder.outputChannels;
  audio.base.device_config.sampleRate = audio.base.decoder.outputSampleRate;
  audio.base.device_config.dataCallback = data_callback;
  audio.base.device_config.pUserData = &audio.base.decoder;
  result = ma_device_init(NULL, &audio.base.device_config, &audio.base.device);
  if(result != MA_SUCCESS) {
    audio.callbacks.logcallback(WARNING, "failed to initialize device\n");
  }
  result = ma_device_start(&audio.base.device);
  InitLogCallback(Log);
}

int main(void) {
}
