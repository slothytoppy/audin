#include "./deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  char* cur_song;
  char** queue;
  long int queue_count;
  ma_result result;
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;
  bool should_close;
  float volume;
  ma_engine_config engine_conf;
  ma_engine engine;
  ma_sound sound;
  ma_device_data_proc data_callback;
} ta_state;

static ta_state* p = NULL;

void plug_init(void) {
  p = malloc(sizeof(*p));
  p->volume = 0.5f;
  ma_decoder_init();
  p->deviceConfig = ma_device_config_init(ma_device_type_playback);
  p->deviceConfig.playback.format = p->decoder.outputFormat;
  p->deviceConfig.playback.channels = p->decoder.outputChannels;
  p->deviceConfig.sampleRate = p->decoder.outputSampleRate;
  p->deviceConfig.dataCallback = p->data_callback;
  p->deviceConfig.pUserData = &p->decoder;
  ma_device_init(NULL, &p->deviceConfig, &p->device);
  p->should_close = false;
  p->engine_conf = ma_engine_config_init();
}
