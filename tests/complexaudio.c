#include "common.h"

typedef struct {
  char** queue;
  long int count;
} Track;

typedef struct {
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;
  ma_sound sound;
  ma_sound_config sound_config;
  ma_float volume;
  ma_engine engine;
  ma_engine_config engine_config;
  Track track;
} Audio;

Audio audio = {0};
ma_result result;

bool LoadSong(char* song) {
  result = ma_decoder_init_file(song, NULL, &audio.decoder);
  if(result != MA_SUCCESS) {
    printw("could not load %s for %d\n", song, result);
    goto refscr;
    return false;
  }
  audio.deviceConfig = ma_device_config_init(ma_device_type_playback);
  audio.deviceConfig.playback.format = audio.decoder.outputFormat;
  audio.deviceConfig.playback.channels = audio.decoder.outputChannels;
  audio.deviceConfig.sampleRate = audio.decoder.outputSampleRate;
  audio.deviceConfig.dataCallback = data_callback;
  audio.deviceConfig.pUserData = &audio.decoder;

  result = ma_device_init(NULL, &audio.deviceConfig, &audio.device);
  if(result != MA_SUCCESS) {
    printw("Failed to open playback device.\n");
    ma_decoder_uninit(&audio.decoder);
    goto refscr;
    return false;
  }
  result = ma_device_start(&audio.device);
  if(result != MA_SUCCESS) {
    printw("Failed to start playback device.\n");
    ma_device_uninit(&audio.device);
    ma_decoder_uninit(&audio.decoder);
    goto refscr;
    return false;
  }
  audio.engine_config = ma_engine_config_init();
  result = ma_engine_init(&audio.engine_config, &audio.engine);
  if(result != MA_SUCCESS) {
    printw("could initialize engine for %d\n", result);
    goto refscr;
    return false;
  }
  result = ma_engine_start(&audio.engine);
  if(result != MA_SUCCESS) {
    printw("could not start engine\n");
    ma_decoder_uninit(&audio.decoder);
    goto refscr;
    return false;
  }

  audio.sound_config = ma_sound_config_init();
  result = ma_sound_init_ex(&audio.engine, &audio.sound_config, &audio.sound);
  if(result != MA_SUCCESS) {
    printw("could not initialize sound\n");
    ma_decoder_uninit(&audio.decoder);
    goto refscr;
    return false;
  }
  if(result != MA_SUCCESS) {
    printw("could not start sound\n");
    ma_decoder_uninit(&audio.decoder);
    goto refscr;
    return false;
  }
  result = ma_sound_start(&audio.sound);

refscr:
  refresh();

  return true;
}

bool UnloadSong() {
  ma_decoder_uninit(&audio.decoder);
  ma_sound_uninit(&audio.sound);
  ma_device_uninit(&audio.device);
  return true;
}

void ChangeVolume(float volume) {
  ma_device_set_master_volume(&audio.device, volume);
}

float GetVolume(void) {
  float volume;
  ma_device_get_master_volume(&audio.device, &volume);
  return volume;
}

bool GetSongLength(float* length) {
  return (false);
}

int main(int argc, char** argv) {
  if(argc < 2) {
    printf("not enough args\n");
    return 1;
  }
  initscr();
  noecho();
  raw();
  struct dirent* dirent;
  DIR* dir = opendir(argv[1]);
  chdir(argv[1]);
  Nom_cmd cmd = {0};
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.')
      continue;
    nom_cmd_append(&cmd, dname);
  }
  assert(cmd.count > 0);
  printw("cmd.count %d\n", cmd.count);
  char ch = getch();
  bool close = false;
  long int index = 0;
  float volume = 0.0f;
  if(LoadSong("../stuff/down_with_the_sickness.mp3") != true) {
    endwin();
    exit(1);
  }
  nom_log_cmd(NOM_INFO, NULL, cmd);
  printw("current song %s\n", cmd.items[index]);
  while(close != true) {
    ch = getch();
    switch(ch) {
    case 'q':
      close = true;
      break;
    case 's':
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      printw("index %d\n", index);
      printw("current song %s\n", cmd.items[index]);
      UnloadSong();
      LoadSong(cmd.items[index]);
      break;
    case 'c':
      if(volume > 0) {
        volume -= 0.1f;
        ChangeVolume(volume);
      }
      break;
    case 'v':
      if(volume < 1) {
        volume += 0.1f;
        ChangeVolume(volume);
      }
      break;
    case 'r':
      ma_decoder_seek_to_pcm_frame(&audio.decoder, 0);
    default:
      printw("song %s\nvolume %f\n", cmd.items[index], GetVolume());
      refresh();
    }
  }
  assert(cmd.count > 0);
}
