#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <stdio.h>
#include <math.h>

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

typedef struct { int x,y; } point;
typedef struct { float x,y; } vec;

typedef struct {
	point p;
	vec v;
	float angle;
	float health;
	int frame;
};

int main( int argc, char* args[] )
{
    SDL_Surface* screen = NULL;
    SDL_Surface* sprite = NULL;
    SDL_Surface* icon = NULL;
	SDL_Surface *player_frame = NULL;

	// init
	int sprite_width = 130;
	int sprite_height = 130;
	int sprite_bx = 520+10/*center*/;
	int sprite_by = 650-16/*center*/;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );
	sprite = IMG_Load_RW( SDL_RWFromMem(sprite_jpg, sprite_jpg_len), 1 );
	player_frame = SDL_CreateRGBSurface(SDL_SWSURFACE, sprite_width, sprite_height, 32,
				rmask, gmask, bmask, amask);

	// window manager
	{
		icon = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 32,
				rmask, gmask, bmask, amask);

		SDL_Rect src = {32,32,64,64};
		SDL_Rect dst = {0,0,0,0};
		SDL_BlitSurface( sprite, &src, icon, &dst );
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface( icon );
		SDL_WM_SetCaption("inferno", "inferno");
	}

	screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );

	// player setup
	int x=0, y=0;
	int frame=0, step=0;
	int speed=8;
	int up=0,down=0,left=0,right=0;
	float accel=.2;
	float angle=0;

	// main loop
	int running = 1;
	unsigned int t=0,lt=0;
	int pressed[SDLK_LAST] = {0};
	while(running) {

		SDL_Event event;
		if( SDL_PollEvent( &event ) )
		{
			switch(event.type) {
			case SDL_QUIT:
				running=0;
			break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					running = 0;
				break;
				}
			//nobreak, slip...
			case SDL_KEYUP:
				pressed[event.key.keysym.sym] = event.type == SDL_KEYDOWN;
			break;
			}
		}

		// move player
		up   =   up*(1-accel)+pressed[SDLK_UP   ]*speed*accel;
		down = down*(1-accel)+pressed[SDLK_DOWN ]*speed*accel;
		left = left*(1-accel)+pressed[SDLK_LEFT ]*speed*accel;
		right=right*(1-accel)+pressed[SDLK_RIGHT]*speed*accel;
		//printf("x %d y %d up %d dw %d le %d ri %d acc %f\n",x,y,up,down,left,right,accel);
		int dx=right-left;
		int dy=down-up;
		x+=dx;
		y+=dy;
		if(dx||dy) angle = atan2(-dy,dx);
		//printf("%d %d %f\n",dx,dy, angle);

		// TODO collision

		// animation
		if(dx||dy) {
			step++;
			if(step%4==0)
				frame=(frame+((rand()%20)==0?2:1))%5;
		}

		// clean screen
		SDL_FillRect(screen,NULL, 0);

		// draw player
		SDL_Rect dst = {x,y,0,0};
#if 0
		SDL_FillRect(screen,&dst, 0xffffff);
#else
		SDL_Rect src = {sprite_bx+frame*sprite_width,sprite_by,sprite_width,sprite_height};
		SDL_BlitSurface( sprite, &src, player_frame, NULL );
		SDL_Surface *player_rotozoom = rotozoomSurface(player_frame, (angle)*180/M_PI, 1, SMOOTHING_ON);
		SDL_BlitSurface( player_rotozoom, NULL, screen, &dst );
		SDL_FreeSurface(player_rotozoom);
#endif

		SDL_Flip( screen );

		// timing control
		lt = t; t = SDL_GetTicks();
		int actual_delta = t-lt;
		int expected_delta = 1000/FPS;
		int delay = MAX(0, expected_delta - actual_delta);
		SDL_Delay(delay);
	}

    SDL_FreeSurface( sprite );

    SDL_Quit();

    return 0;
}
