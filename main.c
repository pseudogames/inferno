#include "SDL.h"
#include "SDL_image.h"

#define FPS 30
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

extern unsigned char sprite_jpg[];
extern unsigned int sprite_jpg_len;

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
Uint32 rmask = 0xff000000;
Uint32 gmask = 0x00ff0000;
Uint32 bmask = 0x0000ff00;
Uint32 amask = 0x000000ff;
#else
Uint32 rmask = 0x000000ff;
Uint32 gmask = 0x0000ff00;
Uint32 bmask = 0x00ff0000;
Uint32 amask = 0xff000000;
#endif


int main( int argc, char* args[] )
{
    SDL_Surface* screen = NULL;
    SDL_Surface* sprite = NULL;
    SDL_Surface* icon = NULL;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );

	sprite = IMG_Load_RW( SDL_RWFromMem(sprite_jpg, sprite_jpg_len), 1 );

	{
		icon = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 32,
				rmask, gmask, bmask, amask);

		SDL_Rect src = {32,32,64,64};
		SDL_Rect dst = {0,0,0,0};
		SDL_BlitSurface( sprite, &src, icon, &dst );
		SDL_WM_SetIcon(icon, NULL);
	}
	SDL_WM_SetCaption("inferno", "inferno");

	screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );

	int dx=0, dy=0;
	int x=0, y=0;
	int i=0;
	int d=4;

	int running = 1;
	unsigned int t=0,lt=0;
	while(running) {
		SDL_Event event;
		if( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_KEYUP )
			{
				dx=dy=0;
			} else if( event.type == SDL_KEYDOWN )
			{

				switch( event.key.keysym.sym )
				{
					case SDLK_UP: dy=-d; break;
					case SDLK_DOWN: dy=+d; break;
					case SDLK_LEFT: dx=-d; break;
					case SDLK_RIGHT: dx=+d; break;
				}
			}

			else if( event.type == SDL_QUIT )
			{
				running = 0;
			}
		}
		x+=dx;y+=dy;

		if(dx||dy)
			i=(i+1)%5;
		SDL_FillRect(screen,NULL, 0);
		SDL_Rect src = {(5+i)*130,0,130,115};
		SDL_Rect dst = {x,y,0,0};
		SDL_BlitSurface( sprite, &src, screen, &dst );
		//SDL_FillRect(screen,&dst, 0xffffff);
		SDL_Flip( screen );
		lt = t; t = SDL_GetTicks();
		int actual_delta = t-lt;
		int expected_delta = 1000/FPS;
		int delay = MAX(0, expected_delta - actual_delta);
		SDL_Delay(delay);
	}

    SDL_FreeSurface( sprite );
    //SDL_FreeSurface( icon );

    SDL_Quit();

    return 0;
}
