#define MINIAUDIO_IMPLEMENTATION
#include "./deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#include "./deps/nom/nom.h"
#include <ncurses.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, "gcc");

  initscr();
  raw();
  noecho();

  ma_result result;
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;

  DIR* dir;
  struct dirent* dirent;
  Nom_cmd dir_files = {0};

  argc -= 1;
  argv += 1;

  if(argc == 0) {
    dir = opendir(".");
  } else if(argc > 0) {
    if(IS_PATH_DIR(argv[argc - 1])) {
      dir = opendir(argv[argc - 1]);
      while((dirent = readdir(dir))) {
        char* dname = dirent->d_name;
        if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
          continue;
        }
        nom_cmd_append(&dir_files, dname);
      }
    } else {
      for(int i = 0; i < argc; i++) {
        if(IS_PATH_FILE(argv[i])) {
          nom_cmd_append(&dir_files, argv[i]);
        }
      }
    }
  }
  refresh();
  printw("argc=%d\n", argc);

  for(int i = 0; i < dir_files.count; i++) {
    printw("%s\n", dir_files.items[i]);
  }

  result = ma_decoder_init_file("./stuff/down_with_the_sickness.mp3", NULL, &decoder);
  if(result != MA_SUCCESS) {
    return -2;
  }

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = decoder.outputFormat;
  deviceConfig.playback.channels = decoder.outputChannels;
  deviceConfig.sampleRate = decoder.outputSampleRate;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = &decoder;

  if(ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    ma_decoder_uninit(&decoder);
    return -3;
  }

  if(ma_device_start(&device) != MA_SUCCESS) {
    printf("Failed to start playback device.\n");
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return -4;
  }

  int not_close = 0;
  float volume = 0.5f;
  ma_engine_config engine_conf = ma_engine_config_init();
  ma_engine engine;
  ma_sound sound;
  ma_engine_init(&engine_conf, &engine);
  ma_sound_init_from_file(&engine, "./stuff/down_with_the_sickness.mp3", MA_SOUND_FLAG_DECODE, NULL, NULL, &sound);
  // result = ma_engine_init(&engine_conf, &engine);
  ma_uint64 length = 0;
  if(result != MA_SUCCESS) {
    return result;
  }
  if(result != MA_SUCCESS) {
    return result;
  }
  bool playing = true;
  bool muted = false;
  ma_device_set_master_volume(&device, volume);
  while(!not_close) {
    length = ma_sound_get_length_in_pcm_frames(&sound, &length) / decoder.outputSampleRate;
    int ch = getchar();
    if(ch == 'v') {
      if(volume >= 1.0f) {
        continue;
      }
      volume += 0.1f;
      ma_device_set_master_volume(&device, volume);
    }
    if(ch == 'c') {
      if(volume <= 0.0f) {
        continue;
      }
      volume -= 0.1f;
      ma_device_set_master_volume(&device, volume);
    }
    if(ch == 'q') {
      not_close += 1;
      endwin();
    }
    if(ch == 'p') {
      playing = !playing;
      if(!playing) {
        ma_device_stop(&device);
      } else {
        ma_device_start(&device);
      }
    }
    if(ch == 'd') {
      ma_device_get_master_volume(&device, &volume);
      printw("volume: %f\n", volume);
      printw("length: %llu\n", length);
      refresh();
    }
    if(ch == 'r') {
      ma_decoder_seek_to_pcm_frame(&decoder, 0);
    }
    if(ch == 'm') {
      muted = !muted;
      if(muted) {
        ma_device_set_master_volume(&device, volume);
      } else {
        ma_device_set_master_volume(&device, 0);
      }
    }
    ma_sound_set_volume(&sound, volume);
  }

  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);

  return 0;
}
