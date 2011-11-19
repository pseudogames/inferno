#include <SDL_mixer.h>


struct audio_info { 
  int rate;
  Uint16 format; /* 16-bit stereo */
  int channels;
  int buffers;
};

void initMusic(); 
void doneMusic();
