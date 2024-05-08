#if 0
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_GENERATION
#endif

#include "../deps/miniaudio/extras/miniaudio_split/miniaudio.h"
#define mini "../deps/miniaudio/extras/miniaudio_split/miniaudio.c"
#include "../deps/nom/nom.h"
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if(pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}
