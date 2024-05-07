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
  ma_mutex mutex;
  ma_uint64 song_len;
} Audio;

Audio audio = {0};
ma_result result;

typedef struct SongLength {
  ma_uint8 minutes;
  ma_uint8 seconds;
} SongLength;

SongLength GetReadableSongLength(void) {
  ma_float total_seconds = (float)audio.song_len / (float)audio.decoder.outputSampleRate;
  ma_uint8 minutes = total_seconds / 60;
  ma_uint8 seconds = (int)total_seconds % 60;

  printw("song length %d:%d\n", minutes, seconds);
  SongLength song_length = {.minutes = minutes, .seconds = seconds};
  refresh();
  return song_length;
}

void MutexInit(void) {
  result = ma_mutex_init(&audio.mutex);
  assert(result == MA_SUCCESS);
}

bool LoadSong(char* song) {
  result = ma_decoder_init_file(song, NULL, &audio.decoder);
  if(result != MA_SUCCESS) {
    printw("decoder init %s\n", ma_result_description(result));
    refresh();
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
    printw("device init %s\n", ma_result_description(result));
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }
  result = ma_device_start(&audio.device);
  if(result != MA_SUCCESS) {
    printw("device start %s\n", ma_result_description(result));
    ma_device_uninit(&audio.device);
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }
  audio.engine_config = ma_engine_config_init();
  result = ma_engine_init(&audio.engine_config, &audio.engine);
  if(result != MA_SUCCESS) {
    printw("engine init %s\n", ma_result_description(result));
    refresh();
    return false;
  }
  result = ma_engine_start(&audio.engine);
  if(result != MA_SUCCESS) {
    printw("engine start %s\n", ma_result_description(result));
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }
  // audio.sound.pDataSource = &audio.decoder;
  audio.sound_config = ma_sound_config_init();
  result = ma_sound_init_ex(&audio.engine, &audio.sound_config, &audio.sound);
  if(!ma_sound_is_spatialization_enabled(&audio.sound)) {
    printw("not enabled\n");
    refresh();
  }

  if(result != MA_SUCCESS) {
    printw("sound init %s\n", ma_result_description(result));
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }
  result = ma_sound_start(&audio.sound);
  if(result != MA_SUCCESS) {
    printw("sound %s\n", ma_result_description(result));
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }
  audio.sound.pDataSource = &audio.decoder;
  result = ma_sound_get_length_in_pcm_frames(&audio.sound, &audio.song_len);
  if(result != MA_SUCCESS) {
    printw("error getting sound length %s\n", ma_result_description(result));
    ma_decoder_uninit(&audio.decoder);
    refresh();
    return false;
  }

refscr:
  refresh();

  return true;
}

bool UnloadSong() {
  ma_mutex_lock(&audio.mutex);
  ma_decoder_uninit(&audio.decoder);
  ma_sound_uninit(&audio.sound);
  ma_device_uninit(&audio.device);
  ma_mutex_unlock(&audio.mutex);
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

ma_uint64 GetSongLength() {
  ma_uint64 length = audio.song_len;
  if(length == 0) {
    printw("error getting sound length %s\n", ma_result_description(result));
    refresh();
  }
  return length;
}

bool GoToEnd(void) {
  if(ma_sound_at_end(&audio.sound)) {
    return true;
  }
  ma_uint64 total_length = audio.song_len;
  if(total_length == 0) {
    printw("length is too low\n");
    refresh();
    return false;
  }
  result = ma_sound_seek_to_pcm_frame(&audio.sound, total_length);
  if(result != MA_SUCCESS) {
    printw("error seeking %s\n", ma_result_description(result));
  }
  SongLength song_len = GetReadableSongLength();
  printw("total len %d:%d\n", song_len.minutes, song_len.seconds);
  refresh();
  return true;
}

int main(int argc, char** argv) {
  if(argc < 2) {
    printf("not enough args\n");
    return 1;
  }
  initscr();
  noecho();
  raw();
  MutexInit();
  struct dirent* dirent;
  DIR* dir = opendir(argv[1]);
  chdir(argv[1]);
  Nom_cmd cmd = {0};
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.')
      continue;
    if(IS_PATH_FILE(dname)) {
      nom_cmd_append(&cmd, dname);
    }
  }
  assert(cmd.count > 0);
  printw("cmd.count %d\n", cmd.count);
  char ch = getch();
  bool close = false;
  long int index = 0;
  float volume = 0.5f;
  if(LoadSong(cmd.items[index]) != true) {
    endwin();
  }
  nom_log_cmd(NOM_INFO, NULL, cmd);
  printw("current song %s\n", (char*)cmd.items[index]);
  ma_uint64 length = 0;
  ma_sound_get_length_in_pcm_frames(&audio.sound, &length);
  ChangeVolume(volume);
  bool playing = true;
  while(close != true) {
    ch = getch();
    switch(ch) {
    case 'q':
      close = true;
      break;
    case 'p':
      if(playing) {
        ma_device_stop(&audio.device);
      } else {
        ma_device_start(&audio.device);
      }
      playing = !playing;
      break;
    case 's':
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      printw("index %lu\n", index);
      printw("current song %s\n", (char*)cmd.items[index]);
      UnloadSong();
      LoadSong(cmd.items[index]);
      GetReadableSongLength();
      ma_sound_get_length_in_pcm_frames(&audio.sound, &length);
      ChangeVolume(volume);
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
      break;
    case 'e':
      GoToEnd();
      break;
    default:

      printw("song %s\nvolume %f\n", (char*)cmd.items[index], GetVolume());
      printw("pcm frames length %llu\n", length);
      refresh();
    }
    if(ma_sound_at_end(&audio.sound)) {
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      LoadSong(cmd.items[index]);
      printw("at end of song\n");
    }
  }
  endwin();
  echo();
}
