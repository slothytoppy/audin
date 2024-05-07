#include "common.h"

typedef struct Audio {
  struct base {
    ma_device device;
    ma_device_config device_config;
    ma_float volume;
    ma_decoder decoder;
    ma_decoder_config decoder_config;
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
    return;
  }
  result = ma_device_start(&audio.base.device);
}

void* LoadFile(char* filepath) {
  if(!IS_PATH_EXIST(filepath)) {
    return NULL;
  }
  char* data;
  long int data_len = nom_get_fsize(filepath);
  int fd = open(filepath, O_RDONLY);
  return mmap(data, data_len, PROT_READ, MAP_SHARED, fd, 0);
}

typedef struct Music {
  ma_uint64 frame_count;
  bool looping;
  void* data_memory;
  ma_decoder decoder;
} Music;

Music LoadSong(char* filepath) {
  Music music = {0};
  if(!IS_PATH_EXIST(filepath) || filepath == NULL) {
    return music;
  }
  music.data_memory = LoadFile(filepath);
  ma_decoder_config config = ma_decoder_config_init_default();
  audio.base.decoder_config = ma_decoder_config_init_default();
  ma_decoder_init_file(filepath, &audio.base.decoder_config, &audio.base.decoder);
  audio.base.device.pUserData = &audio.base.decoder;
  return music;
  // ma_data_converter_process_pcm_frames()
  // ma_data_converter_init()
}

void PlaySound(Music music) {
  ma_device_start(&audio.base.device);
}

int main(void) {
  initscr();
  raw();
  noecho();
  InitAudio();
  Music music = LoadSong("../stuff/Gorillaz - Baby Queen.mp3");
  PlaySound(music);
  bool should_close = false;
  while(!should_close) {
    char ch = getch();
    if(ch == 'q') {
      endwin();
      echo();
      should_close = !should_close;
    }
  }
}
