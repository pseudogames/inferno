#include <stdio.h>

#include <SDL.h>
#include <SDL_mixer.h>

typedef struct{ 
    int rate;
    Uint16 format; /* 16-bit stereo */
    int channels;
    int buffers;
} IAudio;

void initMusic() { 

    IAudio a; 

    a.rate = 22050;
    a.format = AUDIO_S16; /* 16-bit stereo */
    a.channels = 2;
    a.buffers = 4046;

    if(Mix_OpenAudio(a.rate, a.format, a.channels, a.buffers)) {
        printf("Unable to open audio!\n");
        exit(1);
    }

    Mix_QuerySpec(&a.rate, &a.format, &a.channels);
}

int main() { 
}

