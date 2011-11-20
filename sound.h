#include <SDL_mixer.h>

typedef struct{ 
    int rate;
    Uint16 format; /* 16-bit stereo */
    int channels;
    int buffers;
} IAudio;

void initMusic();
void doneMusic();
void loadEffects();
void load_musci();
void playPunch();
void play_menu_select();
void play_menu_confirm();

void handle_ingame_music();
void handle_menu_music();

void halt_music();
