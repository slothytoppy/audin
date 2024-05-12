#include "common.h"
#include "ui.h"
#include <sys/stat.h>

// TODO: add audio threading
// make v1 of the UI

typedef struct Song {
  void* data_memory;
  ma_decoder decoder;
  ma_decoder_config decoder_config;
  ma_mutex mutex;
  ma_bool8 is_ready;
  float volume;     // 0..1
  ma_bool8 playing; // no paused field because !playing means it is paused
  ma_bool8 at_end;
  ma_bool8 muted;
  ma_bool8 looping;
  ma_uint64 frame_count;
} Song;

typedef struct Music {
  Song song;
  long int frame_count;
  void* data;
} Music;

// to stream audio, you can map the entire file into memory and only send chunks of that to be decoded, what takes the most time is decoding it
// maybe use ma_resource_manager for streaming sound?

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
    ma_bool8 at_end;
    ma_bool8 muted;
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
  if(volume <= 1.1f && volume > 0.0f) {
    SetInternalVolume(volume);
    SetDeviceVolume(volume);
    return true;
  } else {
    return false;
  }
}

bool IsAudioReady(void) {
  return audio.base.is_ready;
}

bool GoToSongEnd() {
  // ma_uint64 frames;
  // ma_decoder_get_available_frames(&audio.base.decoder, &frames);
  // printw("frames %llu\n", frames);
  ma_mutex_lock(&audio.base.mutex);
  ma_result result = ma_decoder_seek_to_pcm_frame(&audio.base.decoder, audio.base.frames_count);
  ma_mutex_unlock(&audio.base.mutex);
  if(result != MA_SUCCESS) {
    return false;
  }
  audio.internal.at_end = true;
  return true;
}

bool AtSongEnd() {
  if(audio.internal.at_end == true || audio.base.cursor == audio.base.frames_count) {
    return true;
  }
  return false;
}

void data_source_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_data_source* pdatasource = (ma_data_source*)pDevice->pUserData;
  if(AtSongEnd()) {
    audio.internal.playing = MA_FALSE;
    return;
  }
  if(audio.internal.playing == MA_FALSE) {
    return;
  }
  if(pdatasource == NULL) {
    return;
  }
  ma_mutex_lock(&audio.base.mutex);
  ma_uint64 framesRead;
  ma_data_source_read_pcm_frames(pdatasource, pOutput, frameCount, &framesRead);
  if(frameCount != framesRead) {
    audio.internal.at_end = true;
  }
  ma_decoder_get_cursor_in_pcm_frames(&audio.base.decoder, &audio.base.cursor);

  (void)pInput;
  ma_mutex_unlock(&audio.base.mutex);
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
  ma_mutex_init(&audio.base.mutex);
  if(result != MA_SUCCESS) {
    printf("%s\n", ma_result_description(result));
    return;
  }
  result = ma_device_start(&audio.base.device);
  if(result != MA_SUCCESS) {
    printf("%s\n", ma_result_description(result));
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
  ma_result result = MA_SUCCESS;
  assert(length > 0);
  ma_decoder_config config = ma_decoder_config_init_default();
  audio.base.decoder_config = ma_decoder_config_init_default();
  result = ma_decoder_init_memory(data, length, &audio.base.decoder_config, &audio.base.decoder);
  audio.base.device.pUserData = &audio.base.decoder;
  ma_uint64 song_length = 0;
  result = ma_decoder_get_length_in_pcm_frames(&audio.base.decoder, &song_length);
  audio.base.frames_count = song_length;
  audio.internal.at_end = false;
  assert(result == MA_SUCCESS);
  assert(song_length > 0);
  return music;
  // ma_data_converter_process_pcm_frames()
  // ma_data_converter_init()
}

void PlaySound(void) {
  audio.internal.playing = MA_TRUE;
  ma_device_start(&audio.base.device);
}

void UnloadSong(Music song) {
  audio.base.decoder.pUserData = NULL;
  audio.base.device.pUserData = NULL;
  audio.base.cursor = 0;
  audio.base.frames_count = 0;
  audio.base.is_ready = false;
  audio.internal.playing = false;
  audio.time.seconds = 0;
  audio.time.minutes = 0;
  audio.time.frame_count = 0;
  audio.time.total_seconds = 0;
}

float GetSongLengthInSeconds(void) {
  ma_float seconds;
  seconds = (float)audio.base.frames_count / (float)audio.base.decoder.outputSampleRate;
  return (float)seconds;
}

float GetRemainingSongLength(void) {
  float seconds = (float)audio.base.frames_count;
  return seconds;
}

float GetSongLengthInMinutes(void) {
  float secondsplayed = 0.0f;
  secondsplayed = GetSongLengthInSeconds();
  audio.time.total_seconds = secondsplayed;
  audio.time.seconds = (int)secondsplayed % 60;
  audio.time.minutes = (int)secondsplayed / 60;
  return secondsplayed;
}

float SongTimePlayedInSeconds() {
  float secondsplayed = 0.0f;
  ma_uint64 cursor = 0;
  ma_decoder_get_cursor_in_pcm_frames(&audio.base.decoder, &cursor);
  secondsplayed = (float)cursor / (float)audio.base.decoder.outputSampleRate;
  secondsplayed = (int)secondsplayed % 60;
  char* buff = calloc(1, 9);
  snprintf(buff, 9, "%d", (int)secondsplayed);
  int cx;
  int cy;
  getcenter(&cx, &cy);
  renderat(cx, cy, buff);
  return secondsplayed;
}

bool IS_PATH_FILE_AT(const int fd, const char* file) {
  struct stat fi;
  if(fstatat(fd, file, &fi, 0) == 0) {
    if(fi.st_mode & S_IFREG) {
      return true;
    }
  } else {
    printf("errno %d\n", errno);
    return false;
  }
  return false;
}

Nom_cmd LoadSongFromDir(char* dir) {
  Nom_cmd cmd = {0};
  struct dirent* dirent;
  int fd = open(dir, O_DIRECTORY | O_RDONLY);
  DIR* fdir = fdopendir(fd);
  struct stat fi;
  while((dirent = readdir(fdir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    char* buff = calloc(1, strlen(dname) + strlen(dir) + 2);
    if(IS_PATH_FILE_AT(fd, dname)) {
      strcat(buff, dir);
      if(dname[strlen(dname) - 1] != '/') {
        strcat(buff, "/");
      }
      strcat(buff, dname);
      nom_cmd_append(&cmd, buff);
    }
    nom_log(NOM_INFO, "%s does not exist", buff);
  }
  assert(cmd.count > 0);
  return cmd;
}

int main(void) {
  InitAudio();
  Nom_cmd cmd = {0};
  cmd = LoadSongFromDir("../stuff");
  ncurses_init();
  Music song = {0};
  long int index = 0;
  song = LoadSong(cmd.items[index]);
  PlaySound();
  bool should_close = false;
  while(!should_close) {
    int ch = getch();
    switch(ch) {
    case 'q':
      ncurses_deinit();
      should_close = !should_close;
      break;
    case ' ':
    case 'p':
      audio.internal.playing = !audio.internal.playing;
      break;
    case KEY_DOWN:
    case 'c':
      SetVolume(audio.internal.volume - 0.1f);
      break;
    case KEY_UP:
    case 'v':
      SetVolume(audio.internal.volume + 0.1f);
      break;
    case 's':
      ma_mutex_lock(&audio.base.mutex);
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      UnloadSong(song);
      song = LoadSong(cmd.items[index]);
      PlaySound();
      GetSongLengthInMinutes();
      ma_mutex_unlock(&audio.base.mutex);
      break;
    case 'g':
      GoToSongEnd();
      break;
    case 'm':
      audio.internal.muted = !audio.internal.muted;
      if(audio.internal.muted) {
        SetVolume(audio.internal.volume);
      } else {
        ma_device_set_master_volume(&audio.base.device, 0);
      }
    }
    if(AtSongEnd()) {
      ma_mutex_lock(&audio.base.mutex);
      index += 1;
      if(index > cmd.count - 1) {
        index = 0;
      }
      song = LoadSong(cmd.items[index]);
      PlaySound();
      GetSongLengthInMinutes();
      ma_mutex_unlock(&audio.base.mutex);
    }
    SongTimePlayedInSeconds();
  }
  ncurses_deinit();
}
