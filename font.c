#include "font.h"

extern unsigned char adler_ttf[];
extern unsigned int adler_ttf_len;
extern unsigned char acid_ttf[];
extern unsigned int acid_ttf_len;

void init_font() { 

    TTF_Init();

	font = TTF_OpenFontRW(SDL_RWFromMem(adler_ttf, adler_ttf_len), 1, 72);

    int renderstyle = TTF_STYLE_NORMAL;
    int outline = 0;
    int kerning       = 1;

    TTF_SetFontStyle(font, renderstyle);

}

void text_write(SDL_Surface *screen, int x, int y, char *term, int selected ){ 
    SDL_Rect dstrect;
    SDL_Color red = {0xFF, 0X00, 0x00};
    SDL_Color white = {0xFF, 0XFF, 0xFF};
    SDL_Color color;

    color = (selected) ? white : red;

    SDL_Surface *text;
    text = TTF_RenderText_Solid(font, term, color); 

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = text->w;
    dstrect.h = text->h;

    SDL_BlitSurface(text, NULL, screen, &dstrect);

    SDL_FreeSurface(text);
}
