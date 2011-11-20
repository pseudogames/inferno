#include <SDL_ttf.h>

#define DEFAULT_PTSIZE18
#define DEFAULT_TEXT "The quick brown fox jumped over the lazy dog"
#define NUM_COLORS      256


TTF_Font *font;

void init_font();
void text_write(SDL_Surface *screen, int x, int y, char *term, int selected);
