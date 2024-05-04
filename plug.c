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
  ma_engine_config engine_conf;
  ma_engine engine;
  ma_sound sound;
  ma_device_data_proc data_callback;
} Audio;

typedef struct {
  float volume;
  Audio audio;
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

void ta_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

func_unimplemented bool ta_load_song_from_dir(char** dir) {
  return false;
}

bool ta_restart_song(void) {
  return ma_decoder_seek_to_pcm_frame(&p->audio.decoder, 0);
}

bool ta_mute(void) {
  return ma_device_set_master_volume(&p->audio.device, 0.0f);
}

void check_error(void) {
  if(p->result != MA_SUCCESS) {
    printw("here failed with %d\n", p->result);
    exit(1);
  }
}

void init_device(void) {
  p->audio.device_config = ma_device_config_init(ma_device_type_playback);
  p->audio.device_config.playback.format = p->audio.decoder.outputFormat;
  p->audio.device_config.playback.channels = p->audio.decoder.outputChannels;
  p->audio.device_config.sampleRate = p->audio.decoder.outputSampleRate;
  p->audio.data_callback = ta_data_callback;
  p->audio.device_config.dataCallback = p->audio.data_callback;
  p->audio.device_config.pUserData = &p->audio.decoder;

  p->result = ma_device_init(NULL, &p->audio.device_config, &p->audio.device);
  check_error();
  p->result = ma_device_start(&p->audio.device);
  check_error();
  p->result = ma_device_set_master_volume(&p->audio.device, p->volume);
  check_error();
  p->result = ma_device_init(NULL, &p->audio.device_config, &p->audio.device);
  check_error();
  p->result = ma_device_start(&p->audio.device);
  check_error();
}

void init_decoder(void) {
  p->result = ma_decoder_init_file(p->cur_song, NULL, &p->audio.decoder);
  check_error();
}

void init_engine(void) {
  p->audio.engine_conf = ma_engine_config_init();
  p->result = ma_engine_init(&p->audio.engine_conf, &p->audio.engine);
  check_error();
  p->result = ma_engine_start(&p->audio.engine);
  check_error();
}

void init_sound(void) {
  p->result = ma_sound_init_from_file(&p->audio.engine, p->cur_song, MA_SOUND_FLAG_DECODE, NULL, NULL, &p->audio.sound);
  check_error();
  p->result = ma_sound_start(&p->audio.sound);
  check_error();
}

void load_sound(char* song) {
  p->cur_song = song;
  init_engine();
  init_device();
  init_decoder();
  init_sound();
}

void uninit_engine(void) {
  ma_engine_stop(&p->audio.engine);
  ma_engine_uninit(&p->audio.engine);
}

void uninit_device(void) {
  ma_device_stop(&p->audio.device);
  ma_device_uninit(&p->audio.device);
}

void uninit_decoder(void) {
  ma_decoder_uninit(&p->audio.decoder);
}

void uninit_sound(void) {
  ma_sound_stop(&p->audio.sound);
  ma_sound_uninit(&p->audio.sound);
}

void unload_sound(void) {
  uninit_engine();
  uninit_device();
  uninit_decoder();
  uninit_sound();
}

void plug_init(void) {
  initscr();
  noecho();
  raw();
  p = malloc(sizeof(*p));
  p->volume = 0.5f;
  load_sound("./stuff/down_with_the_sickness.mp3");
  float vol_copy = 0.0f;
  p->result = ma_device_get_master_volume(&p->audio.device, &vol_copy);
  check_error();
  printw("volume: %f\n", vol_copy);
}

ta_state* plug_pre_update(void) {
  p->result = ma_decoder_uninit(&p->audio.decoder);
  ma_device_uninit(&p->audio.device);
  ma_sound_uninit(&p->audio.sound);
  // ma_device_stop(&p->audio.device);
  //  ma_sound_stop(&p->audio.sound);
  return p;
}

void plug_post_update(ta_state* pp) {
  p = pp;
  load_sound(p->cur_song);
}
