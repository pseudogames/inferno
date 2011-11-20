#import "sound.h"

Mix_Music *music = NULL;
Mix_Chunk *punchptr[5];
Mix_Chunk *shotptr;
Mix_Chunk *pickptr;
Mix_Chunk *menu_confirm_ptr;
Mix_Chunk *menu_select_ptr;

IAudio a; 

// Effects 
extern unsigned char punch0_wav[];
extern unsigned int punch0_wav_len;
extern unsigned char punch1_wav[];
extern unsigned int punch1_wav_len;
extern unsigned char punch2_wav[];
extern unsigned int punch2_wav_len;
extern unsigned char punch3_wav[];
extern unsigned int punch3_wav_len;
extern unsigned char punch4_wav[];
extern unsigned int punch4_wav_len;
extern unsigned char pick_wav[];
extern unsigned int pick_wav_len;
extern unsigned char shot_wav[];
extern unsigned int shot_wav_len;
extern unsigned char m60_wav[];
extern unsigned int m60_wav_len;
extern unsigned char menu_wav[];
extern unsigned int menu_wav_len;
extern unsigned char menu_select_wav[];
extern unsigned int menu_select_wav_len;

//Music 

extern unsigned char music_menu_ogg[];
extern unsigned int music_menu_ogg_len;

extern unsigned char music_ingame_ogg[];
extern unsigned int music_ingame_ogg_len;

int punchChannel, shotChannel, pickChannel  = -1;
SDL_RWops *rw = NULL;

void loadEffects(){
    punchptr[0] = Mix_LoadWAV_RW(SDL_RWFromMem(punch0_wav, punch0_wav_len), 1 );
    punchptr[1] = Mix_LoadWAV_RW(SDL_RWFromMem(punch1_wav, punch1_wav_len), 1 );
    punchptr[2] = Mix_LoadWAV_RW(SDL_RWFromMem(punch2_wav, punch2_wav_len), 1 );
    punchptr[3] = Mix_LoadWAV_RW(SDL_RWFromMem(punch3_wav, punch3_wav_len), 1 );
    punchptr[4] = Mix_LoadWAV_RW(SDL_RWFromMem(punch4_wav, punch4_wav_len), 1 );

    shotptr = Mix_LoadWAV_RW(SDL_RWFromMem(m60_wav, m60_wav_len), 1 );
    pickptr = Mix_LoadWAV_RW(SDL_RWFromMem(pick_wav, pick_wav_len), 1 );
    menu_select_ptr = Mix_LoadWAV_RW(SDL_RWFromMem(menu_wav, menu_wav_len), 1 );
    menu_confirm_ptr = Mix_LoadWAV_RW(SDL_RWFromMem(menu_select_wav, menu_select_wav_len), 1 );
}


void load_music() {
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

void handle_menu_music() { 
    rw = SDL_RWFromMem(music_menu_ogg, music_menu_ogg_len);
    music = Mix_LoadMUS_RW(rw);
    Mix_PlayMusic(music, -1);
}

void handle_ingame_music() { 
    rw = SDL_RWFromMem(music_ingame_ogg, music_ingame_ogg_len); 
    music = Mix_LoadMUS_RW(rw);
    Mix_PlayMusic(music, -1);
}

void halt_music(){ 
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    /*SDL_FreeRW(rw);*/
    music = NULL;
}
