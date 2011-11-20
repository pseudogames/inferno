#import "sound.h"

Mix_Music *music = NULL;
Mix_Chunk *punchptr[5];
Mix_Chunk *shotptr;
Mix_Chunk *pickptr;
Mix_Chunk *menu_confirm_ptr;
Mix_Chunk *menu_select_ptr;

IAudio a; 

int punchChannel, shotChannel, pickChannel  = -1;

char punch[50];

void loadEffects(){
    int i;
    for (i = 0; i < 5; i++) {
        sprintf(punch, "./audio/punch/punch%d.wav", i);
        punchptr[i] = Mix_LoadWAV(punch);
    }

    shotptr = Mix_LoadWAV("./audio/shot/m60.wav");
    pickptr = Mix_LoadWAV("./audio/shot/pick.wav");
    menu_select_ptr = Mix_LoadWAV("./audio/shot/pick.wav");
    menu_confirm_ptr = Mix_LoadWAV("./audio/shot/shot.wav");
}


void playPunch(){ 
    punchChannel = Mix_PlayChannel(-1, punchptr[rand() %  5], 0);
}

void playShot() {
    shotChannel = Mix_PlayChannel(-1, shotptr, 0);
}

void playPick() {
    pickChannel = Mix_PlayChannel(-1, pickptr, 0);
}

void play_menu_select() {
    Mix_PlayChannel(-1, menu_select_ptr, 0);
}

void play_menu_confirm() {
    Mix_PlayChannel(-1, menu_confirm_ptr, 0);
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
