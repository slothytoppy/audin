#include "./deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>

#define func_unimplemented __attribute__((unavailable("unimplemented")))

typedef struct {
  ma_decoder decoder;
  ma_decoder_config decoder_config;
  ma_device_config device_config;
  ma_device device;
  float volume;
  ma_engine_config engine_conf;
  ma_engine engine;
  ma_sound sound;
  ma_device_data_proc data_callback;
  bool should_close;
  bool is_muted;
  bool is_playing;
  char* cur_song;
  char** queue;
  ma_float length;
  long int queue_count;
  ma_result result;
} ta_state;

static ta_state* p = NULL;
static struct stat fi = {0};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

bool should_close(void) {
  p->should_close = true;
  return true;
}

bool set_volume(float volume) {
  return ma_device_set_master_volume(&p->device, volume);
}

bool get_volume(float volume) {
  return ma_device_get_master_volume(&p->device, &volume);
}

bool load_song_from_file(char* file) {
  if(stat(file, &fi) < 0) {
    return false;
  }
  p->cur_song = file;
  p->result = ma_decoder_init_file(file, NULL, &p->decoder);
  if(p->result != MA_SUCCESS) {
    return false;
  }
  ma_sound_init_from_file(&p->engine, file, MA_SOUND_FLAG_DECODE, NULL, NULL, &p->sound);
  if(p->result != MA_SUCCESS) {
    return false;
  }
  return true;
}

func_unimplemented bool load_song_from_dir(char** dir) {
  return false;
}

bool restart_song(void) {
  return ma_decoder_seek_to_pcm_frame(&p->decoder, 0);
}

void plug_init(void) {
  initscr();
  noecho();
  raw();
  p = malloc(sizeof(*p));
  p->should_close = false;
  p->volume = 0.5f;
  p->cur_song = "./stuff/Nonpoint - Bullet With a Name.mp3";
  // engine_init
#if 0
  p->engine_conf = ma_engine_config_init();
  p->result = ma_engine_init(&p->engine_conf, &p->engine);
  p->result = ma_sound_init_from_file(&p->engine, p->cur_song, MA_SOUND_FLAG_DECODE, NULL, NULL, &p->sound);
  p->result = ma_sound_get_length_in_seconds(&p->sound, &p->length);
  printw("%f", p->length);
#endif
  // device_init
  p->result = ma_decoder_init_file(p->cur_song, NULL, &p->decoder);
  if(p->result != MA_SUCCESS) {
    printw("failed with %d\n", p->result);
    return;
  }
  p->device_config = ma_device_config_init(ma_device_type_playback);
  p->device_config.playback.format = p->decoder.outputFormat;
  p->device_config.playback.channels = p->decoder.outputChannels;
  p->device_config.sampleRate = p->decoder.outputSampleRate;
  p->data_callback = data_callback;
  p->device_config.dataCallback = p->data_callback;
  p->device_config.pUserData = &p->decoder;

  if(p->result != MA_SUCCESS) {
    printw("failed with %d\n", p->result);
    return;
  }
  p->result = ma_device_init(NULL, &p->device_config, &p->device);
  if(p->result != MA_SUCCESS) {
    printw("failed with %d\n", p->result);
    return;
  }
  if(p->result != MA_SUCCESS) {
    printw("failed with %d\n", p->result);
    return;
  }

  // decoder_init
  if(p->result != MA_SUCCESS) {
    printw("failed with %d\n", p->result);
    return;
  }
  // sound_init
  // device_start
  p->result = ma_device_start(&p->device);
  p->should_close = false;
  p->result = ma_device_set_master_volume(&p->device, p->volume);
  if(p->result != MA_SUCCESS) {
    printw("here failed with %d\n", p->result);
    return;
  }
  float vol_copy = p->volume;
  ma_device_get_master_volume(&p->device, &vol_copy);
  printw("volume: %f\n", vol_copy);
}
