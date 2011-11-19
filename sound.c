#import "sound.h"

Mix_Music *music = NULL;
Mix_Chunk *punch = NULL;
int punchChannel = -1;

IAudio a; 


void loadEffects(){
    punch = Mix_LoadWAV("punch.wav");
}


void playPunch(){ 
    /*if(punchChannel < 0) {*/
        punchChannel = Mix_PlayChannel(-1, punch, 0);
        /*Mix_HaltChannel(punchChannel);*/
    /*}*/
}

void initMusic() { 
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

void handleMusic() { 
    if(music == NULL) {
        music = Mix_LoadMUS("music.ogg");
        Mix_PlayMusic(music, 1);
        Mix_HookMusicFinished(doneMusic);

    } else {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
}

void doneMusic() {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    music = NULL;
}
