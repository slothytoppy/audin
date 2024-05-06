#define MINIAUDIO_IMPLEMENTATION
#include "./deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#include "./deps/nom/nom.h"
#include <ncurses.h>
#include <sys/time.h>

typedef struct {
  ma_device device;
  ma_engine engine;
  ma_sound sound;
  ma_decoder decoder;
  ma_device_config device_config;
  ma_engine_config engine_config;
  ma_sound_config sound_config;
  ma_decoder_config decoder_config;
  ma_backend ma_backend;
  float volume;
  long int frames;
  ma_device_data_proc data_callback;
  ma_result result;
  void* music_data;
} Audio;

Audio audio;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

typedef struct {
  char* SongData;
  long int SongSize;
} SongInfo;

bool LoadSongFromMemory(char* file, SongInfo* song) {
  struct stat fi;
  int fd = open(file, O_RDONLY);
  if(fd < 0 || stat(file, &fi) < 0) {
    return false;
  }
  song->SongData = mmap(song->SongData, fi.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if(song->SongData == MAP_FAILED) {
    song->SongData = NULL;
    song->SongSize = 0;
    return false;
  }
  song->SongSize = fi.st_size;
  return true;
}

bool InitDevice() {
  SongInfo data = {0};
  if(LoadSongFromMemory("./stuff/Gorillaz - Baby Queen.mp3", &data) == false) {
    return false;
  }
  audio.result = ma_decoder_init_memory(data.SongData, data.SongSize, &audio.decoder_config, &audio.decoder);
  audio.device_config = ma_device_config_init(ma_device_type_playback);
  audio.device_config.playback.format = audio.decoder.outputFormat;
  audio.device_config.playback.channels = audio.decoder.outputChannels;
  audio.device_config.sampleRate = audio.decoder.outputSampleRate;
  audio.device_config.dataCallback = data_callback;
  audio.device_config.pUserData = &audio.decoder;
  if(audio.result != MA_SUCCESS) {
    return false;
  }
  return true;
}

bool InitAudio() {
  InitDevice();
  if(ma_device_init(NULL, &audio.device_config, &audio.device) != MA_SUCCESS) {
    goto failure;
  }
  if(ma_device_start(&audio.device) != MA_SUCCESS) {
    ma_device_uninit(&audio.device);
    goto failure;
  }

failure:
  ma_decoder_uninit(&audio.decoder);
  return true;
}

int main(int argc, char** argv) {
  initscr();
  noecho();
  raw();
  bool should_close = false;
  InitAudio();
  while(!should_close) {
    char ch = getch();
    switch(ch) {
    case 'r': {
      exit(1);
      break;
    }
    case 'q':
      should_close = true;
      exit(0);
      break;
    }
  }
  return 0;
}
