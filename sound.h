#include <SDL_mixer.h>

typedef struct{ 
    int rate;
    Uint16 format; /* 16-bit stereo */
    int channels;
    int buffers;
} IAudio;

void initMusic();
void handleMusinc();
void doneMusic();
void loadEffects();
void playPunch();
