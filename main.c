#define MINIAUDIO_IMPLEMENTATION
#include "./deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#include "./deps/nom/nom.h"
#include <ncurses.h>
#include <sys/time.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

bool load_file_from_fd(int fd, char* data) {
  if(fd <= 0) {
    return false;
  }
  struct stat fi;
  int result = fstat(fd, &fi);
  if(result != 0) {
    return false;
  }
  return mmap(data, fi.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

char* load_file_from_name(char* file, char* data) {
  if(file == NULL) {
    return NULL;
  }
  struct stat fi;
  int result = stat(file, &fi);
  int fd = open(file, O_RDWR);
  if(result != 0 || fd < 0) {
    printw("%s\n", strerror(errno));
    return NULL;
  }
  data = mmap(data, fi.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(data == MAP_FAILED) {
    printw("%s\n", strerror(errno));
    return NULL;
  }
  return data;
}

bool init_ncurses(void) {
  initscr();
  raw();
  noecho();
  return true;
}

ma_result ta_decoder_init_memory(ma_decoder* decoder, void* data, long int len, ...) {

  if(len == 0) {
    va_list arg;
    va_start(arg, len);
    char* file = va_arg(arg, char*);
    int fd = open(file, O_RDWR);
    struct stat fi;
    int success = fstat(fd, &fi);
    len = fi.st_size;
    data = mmap(data, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return ma_decoder_init_memory(data, len, NULL, decoder);
  }
  ma_decoder_init_memory(data, len, NULL, decoder);
  return MA_SUCCESS;
}

int main(int argc, char** argv) {
  rebuild(argc, argv, __FILE__, "gcc");
  init_ncurses();

  argc -= 1;
  argv += 1;

  char* musicdata = load_file_from_name("./stuff/Nonpoint - Bullet With a Name.mp3", musicdata);
  if(musicdata == NULL) {
    endwin();
    return 1;
  }
  struct stat fi;
  if(stat("./stuff/Nonpoint - Bullet With a Name.mp3", &fi) < 0) {
    printw("%s\n", strerror(errno));
    return 1;
  }
  // result = ma_decoder_init_file("./stuff/Nonpoint - Bullet With a Name.mp3", NULL, &decoder);

  if(ta_decoder_init_memory(&decoder, musicdata, fi.st_size) != MA_TRUE) {
    return 1;
  }
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
  ma_sound_init_from_data_source(&engine, musicdata, MA_SOUND_FLAG_DECODE, NULL, &sound);
  result = ma_engine_init(&engine_conf, &engine);
  ma_sound_config sound_config;
  if(result != MA_SUCCESS) {
    printf("%d\n", result);
    return result;
  }
  bool muted = false;
  ma_device_set_master_volume(&device, volume);
  bool playing = true;
  while(!not_close) {
    // curtime = clock() - length;
    ma_float length = 0.0f;
    ma_sound_get_length_in_seconds(&sound, &length);
    printw("frames played: %f\n", length);
    refresh();

    int ch = getchar();
    switch(ch) {
    case 'v':
      if(volume >= 1.0f) {
        continue;
      }
      volume += 0.1f;
      ma_device_set_master_volume(&device, volume);
      break;
    case 'c':
      if(volume <= 0.0f) {
        continue;
      }
      volume -= 0.1f;
      ma_device_set_master_volume(&device, volume);
      break;
    case 'r':
      ma_decoder_seek_to_pcm_frame(&decoder, 0);
      break;
    case 'q':
      not_close += 1;
      endwin();
      break;
    case 'd': {
      float length = 0;
      /*
      ma_sound_get_length_in_pcm_frames(&sound, &length);
      length = length / decoder.outputSampleRate;
      printw("length: %llu\n", clock() - length);
      */
      float volume_copy = volume;
      ma_device_get_master_volume(&device, &volume_copy);
      ma_sound_get_length_in_seconds(&sound, &length);
      while(length > 59) {
        length /= 60;
      }
      printw("song len: %f\n", length);
      printw("volume: %f\n", volume_copy);
      break;
    }
    case 'p': {
      playing = !playing;
      if(playing) {
        ma_device_stop(&device);
      } else {
        ma_device_start(&device);
      }
      printw("bool %d\n", playing);
      refresh();
      break;
    }
    case 'm':
      muted = !muted;
      if(muted) {
        ma_device_set_master_volume(&device, volume);
      } else {
        ma_device_set_master_volume(&device, 0);
      }
      break;
    }
    ma_sound_set_volume(&sound, volume);
    refresh();
  }

  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);

  return 0;
}
