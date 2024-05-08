#include "common.h"

typedef struct Music {
  bool looping;
  void* data_memory;
  ma_decoder decoder;
} Music;

typedef struct Audio {
  struct base {
    ma_device device;
    ma_device_config device_config;
    ma_decoder decoder;
    ma_decoder_config decoder_config;
    ma_mutex mutex;
    ma_result result;
    ma_uint64 frames_count;
    ma_uint64 cursor;
    ma_bool8 is_ready;
  } base;
  struct internal {
    float volume;     // 0..1
    ma_uint64 length; // length of the song in seconds
    ma_bool8 playing; // no paused field because !playing means it is paused
    ma_bool8 looping;
  } internal;
  struct time {
    ma_uint64 frame_count;
    ma_uint16 total_seconds;
    ma_uint8 seconds;
    ma_uint8 minutes;
  } time;
  struct callbacks {
    ma_log_callback_proc logcallback;
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

void Log(ma_uint32 level, char* msg, ...) {
  va_list arg;
  va_start(arg, msg);
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
  vfprintf(stdout, msg, arg);
}

void OnLog(void* userdata, ma_uint32 level, const char* msg) {
  Log(WARNING, "miniaudio: %s", msg);
}

void InitLogCallback(ma_log_callback_proc callback) {
  audio.callbacks.logcallback = callback;
}

void SetInternalVolume(float volume) {
  audio.internal.volume = volume;
}

bool SetDeviceVolume(float volume) {
  if(ma_device_set_master_volume(&audio.base.device, volume) != MA_SUCCESS) {
    return false;
  }
  return true;
}

bool SetVolume(float volume) {
  if(volume <= 1.0f && volume > 0.0f) {
    SetInternalVolume(volume);
    SetDeviceVolume(volume);
    printw("volume %f \n", volume);
    return true;
  } else {
    printw("error tried to change volume from %f to %f is below 0.0f\n", audio.internal.volume, volume);
    return false;
  }
}

bool IsAudioReady(void) {
  return audio.base.is_ready;
}

bool GoToSongEnd(Music song) {
  ma_uint64 frames;
  ma_decoder_get_available_frames(&audio.base.decoder, &frames);
  printw("frames %llu\n", frames);
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.base.decoder, audio.base.frames_count);
  if(result != MA_SUCCESS) {
    return false;
  }
  return true;
}

void data_source_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_data_source* pdatasource = (ma_data_source*)pDevice->pUserData;
  if(audio.internal.playing == MA_FALSE) {
    return;
  }
  if(pdatasource == NULL) {
    return;
  }

  ma_data_source_read_pcm_frames(pdatasource, pOutput, frameCount, NULL);
  ma_decoder_get_cursor_in_pcm_frames(&audio.base.decoder, &audio.base.cursor);

  (void)pInput;
}

void InitAudio(void) {
  ma_result result;
  audio.base.device_config = ma_device_config_init(ma_device_type_playback);
  InitLogCallback(OnLog);
  audio.base.device_config.playback.format = ma_format_f32;
  audio.base.device_config.playback.channels = 2;
  audio.base.device_config.sampleRate = 0;
  audio.base.device_config.dataCallback = data_source_data_callback;
  audio.base.device_config.pUserData = NULL;
  result = ma_device_init(NULL, &audio.base.device_config, &audio.base.device);
  if(result != MA_SUCCESS) {
    printw("%s\n", ma_result_description(result));
    return;
  }
  result = ma_device_start(&audio.base.device);
  if(result != MA_SUCCESS) {
    printw("%s\n", ma_result_description(result));
    return;
  }
  audio.base.is_ready = true;
  audio.internal.volume = 0.5f;
}

char* LoadFile(char* filepath, long int* length) {
  if(!IS_PATH_EXIST(filepath)) {
    return NULL;
  }
  struct stat fi;
  stat(filepath, &fi);
  long int data_len = fi.st_size;
  int fd = open(filepath, O_RDONLY);
  assert(fd > 0);
  char* data = mmap(data, data_len, PROT_READ, MAP_SHARED, fd, 0);
  if(data == MAP_FAILED) {
    printf("FAILED\n");
    return NULL;
  }
  ma_decoder_get_length_in_pcm_frames(&audio.base.decoder, &audio.base.frames_count);

  *length = data_len;
  return data;
}

Music LoadSong(char* filepath) {
  Music music = {0};
  if(!IS_PATH_EXIST(filepath) || filepath == NULL) {
    return music;
  }
  long int length = 0;
  char* data = LoadFile(filepath, &length);
  assert(length > 0);
  ma_decoder_config config = ma_decoder_config_init_default();
  audio.base.decoder_config = ma_decoder_config_init_default();
  ma_decoder_init_memory(data, length, &audio.base.decoder_config, &audio.base.decoder);
  audio.base.device.pUserData = &audio.base.decoder;
  ma_uint64 song_length;
  ma_decoder_get_length_in_pcm_frames(&audio.base.decoder, &song_length);
  audio.base.frames_count = song_length;
  return music;
  // ma_data_converter_process_pcm_frames()
  // ma_data_converter_init()
}

void PlaySound(Music music) {
  audio.internal.playing = MA_TRUE;
  ma_device_start(&audio.base.device);
}

void UnloadSong(Music song) {
  audio.base.is_ready = false;
  ma_decoder_uninit(&audio.base.decoder);
  ma_device_uninit(&audio.base.device);
}

float GetSongLengthInSeconds(void) {
  ma_float seconds;
  seconds = (float)audio.base.frames_count / (float)audio.base.decoder.outputSampleRate;
  return (float)seconds;
}

float GetSongLengthInMinutes(void) {
  float secondsplayed = 0.0f;
  ma_uint64 cursor;
  ma_decoder_get_cursor_in_pcm_frames(&audio.base.decoder, &cursor);
  // secondsplayed = cursor % audio.base.frames_count;
  secondsplayed = GetSongLengthInSeconds();
  audio.time.total_seconds = secondsplayed;
  audio.time.seconds = (int)secondsplayed % 60;
  audio.time.minutes = (int)secondsplayed / 60;
  printw("length %d:%d\n", audio.time.minutes, audio.time.seconds);
  return secondsplayed;
}

float SongTimePlayedInSeconds() {
  float secondsplayed = 0.0f;
  ma_uint64 cursor;
  ma_decoder_get_cursor_in_pcm_frames(&audio.base.decoder, &cursor);
  secondsplayed = (float)cursor / (float)audio.base.decoder.outputSampleRate;
  secondsplayed = (int)secondsplayed % 60;
  printw("played %f\n", secondsplayed);
  return secondsplayed;
}

bool AtSongEnd() {
  if(SongTimePlayedInSeconds() == GetSongLengthInSeconds()) {
    return true;
  }
  return false;
}

int main(void) {
  Nom_cmd cmd = {0};
  struct dirent* dirent;
  DIR* dir = opendir("../stuff");
  chdir("../stuff");
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    if(IS_PATH_FILE(dname)) {
      nom_cmd_append(&cmd, dname);
    }
  }
  Music song = {0};
  long int index = 0;
  nom_log_cmd(NOM_INFO, "", cmd);
  InitAudio();
  if(IsAudioReady()) {
    song = LoadSong(cmd.items[index]);
    PlaySound(song);
  }
  bool should_close = false;
  initscr();
  raw();
  noecho();
  while(!should_close) {
    char ch = getch();
    switch(ch) {
    case 'q':
      endwin();
      echo();
      should_close = !should_close;
      break;
    case 'p':
      audio.internal.playing = !audio.internal.playing;
      break;
    case 'c':
      SetVolume(audio.internal.volume - 0.1f);
      break;
    case 'v':
      SetVolume(audio.internal.volume + 0.1f);
      break;
    case 'l':
      audio.internal.looping = true;
    case 's':
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      song = LoadSong(cmd.items[index]);
      PlaySound(song);
      printw("playing %s\n", cmd.items[index]);
      break;
    case 'g':
      GoToSongEnd(song);
      break;
    default:
      GetSongLengthInMinutes();
      printw("volume %f\n", audio.internal.volume);
      printw("playing %s\n", cmd.items[index]);
      SongTimePlayedInSeconds();
    }
    if(AtSongEnd()) {
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      song = LoadSong(cmd.items[index]);
      PlaySound(song);
      printw("playing %s\n", cmd.items[index]);
    }
  }
}
