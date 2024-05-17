#define MINIAUDIO_IMPLEMENTATION
#include "./deps/miniaudio.h"
#include "./deps/nom/nom.h"
#include <assert.h>
#include <ncurses.h>
#include <sys/time.h>

typedef void* (*callback)(void*);

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;
  pthread_mutex_t mutex;
  callback callback;
  void* arg;
} thread;

typedef struct {
  ma_device device;
  ma_context ctx;
  ma_decoder decoder;
} Audio;

Audio audio = {0};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

void InitThread(thread* thread, callback callback, void* arg) {
  pthread_attr_init(&thread->attr);
  pthread_create(&thread->thread, &thread->attr, callback, arg);
  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutex_init(&thread->mutex, &mattr);
}

bool InitAudio() {
  ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
  device_config.playback.format = ma_format_f32;
  device_config.playback.channels = 2;
  device_config.sampleRate = 0;
  device_config.dataCallback = data_callback;
  device_config.pUserData = NULL;
  ma_result result;
  result = ma_device_init(NULL, &device_config, &audio.device);
  assert(result == MA_SUCCESS);
  result = ma_device_start(&audio.device);
  assert(result == MA_SUCCESS);
  return true;
}

void* PlaySong(void* filename) {
  ma_decoder_config config = ma_decoder_config_init_default();
  ma_result result = ma_decoder_init_file(filename, &config, &audio.decoder);
  assert(result == MA_SUCCESS);
  assert(audio.decoder.outputSampleRate > 0);
  audio.device.pUserData = &audio.decoder;
  return NULL;
}

void AsyncPlaySong(char* filename) {
  thread thread = {0};
  InitThread(&thread, PlaySong, filename);
  pthread_join(thread.thread, NULL);
}

int main(int argc, char** argv) {
  initscr();
  raw();
  noecho();
  InitAudio();
  Nom_cmd queue = {0};
  struct dirent* dirent;
  DIR* dir = opendir("../stuff/");
  while((dirent = readdir(dir))) {
    char* dname = dirent->d_name;
    if(strlen(dname) <= 2 && dname[0] == '.' || dname[1] == '.') {
      continue;
    }
    char* buff = calloc(1, 4096);
    strcat(buff, "../stuff/");
    strcat(buff, dname);
    nom_cmd_append(&queue, buff);
  }
  ma_device_set_master_volume(&audio.device, 0.3);
  bool should_close = false;
  AsyncPlaySong("./stuff/Disturbed - Down with the Sickness.mp3");
  int ch;
  while(should_close != true) {
    ch = getch();
    switch(ch) {
    case 'q':
      should_close = true;
      noraw();
      echo();
      endwin();
      break;
    default:
    }
  }
  noraw();
  echo();
  endwin();
  return 0;
}
