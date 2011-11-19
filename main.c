#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <strings.h>
#include <stdio.h>
#include <math.h>

#include "sound.h"

#define FPS 9
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define RAD2DEG(a) ((a)*180/M_PI)

int running = 1;

extern unsigned char sprite_png[];
extern unsigned int sprite_png_len;

typedef struct { int x,y; } point;
typedef struct { float x,y; } vec;

typedef enum { 
	ACTION_MOVE=0, 
	ACTION_ATTACK, 
	ACTION_DEATH, 
	ACTION_COUNT
} Action;

typedef enum { 
    GAME_MENU = 0,
    GAME_PLAY = 1, 
    GAME_PAUSE = 2,
    GAME_CREDIT = 3
} Game; 

typedef struct {
	point origin;
	point frame_size;
	int count;
	SDL_Surface *source;
	point rotated_frame_size;
	SDL_Surface *rotated;
} Sprite;

typedef struct {
	point pos;
	vec vel;
	float max_vel;
	int angle; // degree
	int max_health;
	int health;
	Action action;
	int frame;
	Sprite *sprite;
} Body;

void sprite_origin_rect(Sprite *sprite, Action action, int frame, SDL_Rect *rect)
{
	frame = frame % sprite->count;
	rect->x = sprite->origin.x + frame *sprite->frame_size.x;
	rect->y = sprite->origin.y + action*sprite->frame_size.y;
	rect->w = sprite->frame_size.x;
	rect->h = sprite->frame_size.y;
}

#define ANGLE_STEP 45
#define ZOOM 1

void sprite_rotated_rect(Sprite *sprite, Action action, int frame, int angle, SDL_Rect *rect)
{
	frame = frame % sprite->count;
	int angle_index = ((int)(360+angle+ANGLE_STEP/2) % 360) / ANGLE_STEP;
	rect->x = frame *sprite->rotated_frame_size.x
	        + sprite->rotated_frame_size.x*sprite->count*angle_index;
	rect->y = action*sprite->rotated_frame_size.y;
	rect->w = sprite->rotated_frame_size.x;
	rect->h = sprite->rotated_frame_size.y;
}

void sprite_gen_rotation(Sprite *sprite)
{
	rotozoomSurfaceSize(
		sprite->frame_size.x,
		sprite->frame_size.y,
		45, // to maximize size
		ZOOM,  // no zoom
		&sprite->rotated_frame_size.x,
		&sprite->rotated_frame_size.y
	);

	if(sprite->rotated)
		SDL_FreeSurface(sprite->rotated);

	sprite->rotated = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			sprite->rotated_frame_size.x * sprite->count * 360/ANGLE_STEP,
			sprite->rotated_frame_size.y * ACTION_COUNT,
			32, 0,0,0,0);
	printf("rotation cache size %d %d for %d angles\n", sprite->rotated->w, sprite->rotated->h, 360/ANGLE_STEP);

	SDL_Surface *element = SDL_CreateRGBSurface(SDL_SWSURFACE, 
			sprite->frame_size.x, 
			sprite->frame_size.y,
			32, 0,0,0,0);

	int frame, action, angle;
	for(action=0; action<ACTION_COUNT; action++) {
		for(frame=0; frame<sprite->count; frame++) {
			SDL_Rect src;
			sprite_origin_rect(sprite, action, frame, &src);
			for(angle=0; angle<360; angle+=ANGLE_STEP) {
				SDL_Rect dst;
				sprite_rotated_rect(sprite, action, frame, angle, &dst);
				SDL_FillRect(element, NULL, 0);
				SDL_BlitSurface( sprite->source, &src, element, NULL );
				SDL_Surface *rotozoom = rotozoomSurface(element, angle, ZOOM, SMOOTHING_ON);
				dst.x += dst.w/2 - rotozoom->w/2;
				dst.y += dst.h/2 - rotozoom->h/2; // center
				SDL_BlitSurface(rotozoom, NULL, sprite->rotated, &dst );
				//SDL_BlitSurface(rotozoom, NULL, screen, &dst ); SDL_Flip(screen); SDL_Delay(50); // debug
				SDL_FreeSurface(rotozoom);
			}
		}
	}

	SDL_FreeSurface(element);
}

void sprite_init(Sprite *sprite, int ox, int oy, int fx, int fy, int c, void *img, int img_size)
{
	memset(sprite,0,sizeof(Sprite));
	sprite->origin.x = ox;
	sprite->origin.y = oy;
	sprite->frame_size.x = fx;
	sprite->frame_size.y = fy;
	sprite->count = c;
	sprite->source = IMG_Load_RW( SDL_RWFromMem(img, img_size), 1 );
	sprite->rotated = NULL;
	sprite_gen_rotation(sprite);
}

void body_init(Body *body, Sprite *sprite, int max_health, float max_vel)
{
	memset(body,0,sizeof(Body));
	body->health = body->max_health = max_health;
	body->max_vel = max_vel;
	body->sprite = sprite;
}

void body_move(Body *body, int angle)
{
	float f = .2, k = .8;
	body->angle = (int)(720 + body->angle * (1-f) + angle * f) % 720;
	//float a = ((int)(720 + body->angle * (1-k) + angle * k) % 720) * M_PI / 180;
	float a = body->angle * M_PI / 180;
	// FIXME virando para lado contrario quando tem que cruzar a borda entre 0 e 360
	body->frame = (body->frame+1) % body->sprite->count;
	body->pos.x += cos(a) * body->max_vel;
	body->pos.y -= sin(a) * body->max_vel;

	// TODO collision
}

void body_draw(Body *body, SDL_Surface *screen)
{
		SDL_Rect dst = {body->pos.x,body->pos.y,0,0};
		SDL_Rect src;
		sprite_rotated_rect(body->sprite, body->action, body->frame, body->angle, &src);
		SDL_BlitSurface( body->sprite->rotated, &src, screen, &dst );
}

void state_game_runing(SDL_Event event) { 
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
                case SDLK_m:
                    handleMusic();
                    break;

                case SDLK_p:
                    playPunch();
                    player.action = ACTION_ATTACK;
                    break;

                case SDLK_s:
                    playShot();
                    player.action = ACTION_MOVE;
                    break;

                case SDLK_i:
                    playPick();
                    break;
            }
            //nobreak, slip...
        case SDL_KEYUP:
            pressed[event.key.keysym.sym] = event.type == SDL_KEYDOWN;
            break;
    }
}

void timing_control(Uint32 start) { 
    Uint32 end = SDL_GetTicks();
    int actual_delta = end - start;
    int expected_delta = 1000/FPS;
    int delay = MAX(0, expected_delta - actual_delta);
    SDL_Delay(delay);
}

// global runing


int main( int argc, char* args[] )
{
	SDL_Surface* screen = NULL;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );

#if 0
	{ // window manager
		SDL_Surface* icon = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 32, 0,0,0,0);
		SDL_Rect src = {32,32,64,64};
		SDL_Rect dst = {0,0,0,0};
		SDL_BlitSurface( sprite, &src, icon, &dst );
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface( icon );
		SDL_WM_SetCaption("inferno", "inferno");
	}
#endif

    // music manager
    initMusic();
    
    // effects manager 
    loadEffects();

	screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );

	// player setup
	Sprite zombie;
	sprite_init(&zombie, 
		0, 0, // origin
		114, 114, 12, // frame size and count
		sprite_png, sprite_png_len // source
	);

	Body player;
	body_init(&player, &zombie, 100, 10);


	float up=0,down=0,left=0,right=0;
	float accel=.2;

	// main loop
	unsigned int t=0,lt=0;
	int pressed[SDLK_LAST] = {0};

	while(running) {
		Uint32 start = SDL_GetTicks();
		SDL_Event event;
		if( SDL_PollEvent( &event ) )
		{
            state_game_runing();
		}

		// move player
		up   =   up*(1-accel)+pressed[SDLK_UP   ]*accel;
		down = down*(1-accel)+pressed[SDLK_DOWN ]*accel;
		left = left*(1-accel)+pressed[SDLK_LEFT ]*accel;
		right=right*(1-accel)+pressed[SDLK_RIGHT]*accel;
		float dx=right-left;
		float dy=down-up;
		if(fabs(dx)>0.1||fabs(dy)>0.1) {
			body_move(&player, (int)(720+atan2(-dy,dx)*180/M_PI)%360);
		}

		// clean screen
		SDL_FillRect(screen,NULL, 0);

		body_draw(&player, screen);

		SDL_Flip( screen );

        timing_control(start);
	}

	// TODO free surfaces, like SDL_FreeSurface( sprite );

	SDL_Quit();

	return 0;
}
