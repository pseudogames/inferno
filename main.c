#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sound.h"
#include "font.h"

#define FPS 18
#define MAX_ENEMIES 666
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360)

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define RGBA_FORMAT 32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000
#define RGB_FORMAT  24,0x00ff0000,0x0000ff00,0x000000ff,0x00000000

extern unsigned char hero_png[];
extern unsigned int hero_png_len;
extern unsigned char zombie_png[];
extern unsigned int zombie_png_len;

extern unsigned char mapa_jpg[];
extern unsigned int mapa_jpg_len;

extern unsigned char logo_pseudo_png[];
extern unsigned int logo_pseudo_png_len;

extern unsigned char creditos_jpg[];
extern unsigned int creditos_jpg_len;

extern unsigned char player_hud_png[];
extern unsigned int player_hud_png_len;

extern unsigned char stats_hud_png[];
extern unsigned int stats_hud_png_len;

typedef struct { int x,y; } point;
typedef struct { float x,y; } vec;

typedef enum { 
    ACTION_MOVE=0, 
    ACTION_ATTACK, 
    ACTION_DEATH, 
    ACTION_COUNT
} Action;

typedef enum { 
    STATE_MENU = 0,
    STATE_GAME, 
    STATE_PAUSE,
    STATE_CREDIT,
    STATE_GAMEOVER,
    STATE_QUIT
} State; 

typedef enum { 
    MENU_START = 0,
    MENU_CONTINUE,
    MENU_CREDIT, 
    MENU_QUIT,
    MENU_COUNT
} MenuItem; 

typedef struct {
    point origin;
    point frame_size;
    int frame_count;
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
    int fear; 
    int stamina; 

    Action action;
    int frame;
    Sprite *sprite;
	int hitmap_x;
	int hitmap_y;
	int hitmap_in;
} Body;

typedef struct{
    Sprite zombie;
    Sprite hero;
    Body player;
    Body enemy[MAX_ENEMIES];
	int enemy_count;
    int pressed[SDLK_LAST];
    SDL_Surface *background;
    
    SDL_Surface *player_hud;

    SDL_Surface *stats_hud;

    int body_count; 
    int ammo; 
    int keys;

	int hitmap_dec;
	int hitmap_w;
	int hitmap_h;
	int hitmap_len;
	Uint8 *hitmap;
	int heatmap;

    Uint32 started; 
    Uint32 elapsed; 
} Game;

typedef struct{
    SDL_Surface *image; 
    MenuItem selected;
    SDL_Surface *background;
} Menu;

typedef struct{
    SDL_Surface *image; 
    SDL_Surface *background;
} Credit;

typedef struct{
    SDL_Surface *screen; 
    Game game;
    Menu menu;
    Credit credit;
    State state;
    SDL_Surface *background;
} App;


SDL_Color red = {0xFF, 0X00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};

void sprite_origin_rect(Sprite *sprite, Action action, int frame, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    rect->x = sprite->origin.x + frame *sprite->frame_size.x;
    rect->y = sprite->origin.y + action*sprite->frame_size.y;
    rect->w = sprite->frame_size.x;
    rect->h = sprite->frame_size.y;
}

#define ANGLE_STEP 30
#define ZOOM 1

void sprite_rotated_rect(Sprite *sprite, Action action, int frame, int angle, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    int angle_index = ((int)(360+angle+ANGLE_STEP/2) % 360) / ANGLE_STEP;
    rect->x = frame *sprite->rotated_frame_size.x;
    rect->y = action*sprite->rotated_frame_size.y+
        + sprite->rotated_frame_size.y*ACTION_COUNT*angle_index;
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

    sprite->rotated = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->rotated_frame_size.x * sprite->frame_count,
            sprite->rotated_frame_size.y * ACTION_COUNT * 360/ANGLE_STEP,
            RGBA_FORMAT);
    if(sprite->rotated == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    printf("cache size %dx%d for %d angles\n", sprite->rotated->w, sprite->rotated->h, 360/ANGLE_STEP);

    SDL_Surface *element = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->frame_size.x, 
            sprite->frame_size.y,
            RGBA_FORMAT);

    SDL_SetAlpha(sprite->source,0,0xff);
    SDL_SetAlpha(element,0,0xff);
    SDL_SetAlpha(sprite->rotated,SDL_SRCALPHA,0xff);

    int frame, action, angle;
    for(action=0; action<ACTION_COUNT; action++) {
        for(frame=0; frame<sprite->frame_count; frame++) {
            SDL_Rect src;
            sprite_origin_rect(sprite, action, frame, &src);
            for(angle=0; angle<360; angle+=ANGLE_STEP) {
                SDL_Rect dst;
                sprite_rotated_rect(sprite, action, frame, angle, &dst);
                SDL_FillRect(element, NULL, 0x00000000);
                SDL_BlitSurface( sprite->source, &src, element, NULL );
                SDL_Surface *rotozoom = rotozoomSurface(element, angle, ZOOM, SMOOTHING_ON);
                SDL_SetAlpha(rotozoom,0,0);
                SDL_SetColorKey(rotozoom,0,0);
                dst.x += dst.w/2 - rotozoom->w/2;
                dst.y += dst.h/2 - rotozoom->h/2; // center
                SDL_BlitSurface(rotozoom, NULL, sprite->rotated, &dst );
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
    sprite->frame_count = c;
    sprite->source = IMG_Load_RW( SDL_RWFromMem(img, img_size), 1 );
    sprite->rotated = NULL;
    sprite_gen_rotation(sprite);
}



void body_move(Game *game, Body *body, int angle)
{
	if(body->action == ACTION_DEATH)
		return;
	float v = body->max_vel;
    if(body->hitmap_in) {
		int x0 = MAX(body->hitmap_x-1, 0);
		int y0 = MAX(body->hitmap_y-1, 0);
		int x1 = MIN(body->hitmap_x+1, game->hitmap_w-1);
		int y1 = MIN(body->hitmap_y+1, game->hitmap_h-1);
		int x,y;
		int high_x, high_y, high = 0;
		for(y=y0; y<=y1; y++) {
			for(x=x0; x<=x1; x++) {
				int v = game->hitmap[x+y*game->hitmap_w];
				if(v > high) {
					high_x = x;
					high_y = y;
					high = v;
				}
			}
		}
		if(high>0x50) {
			float k = high/256.;
			int a = ATAN2(
				high_x-body->pos.x,
				high_y-body->pos.y
			);
			if(fabs(angle - a) > 180) {
				a += 360;
			}
			angle = angle*(1-k) + k*a;
			v *= .5;
		}
		if(high>0x90) {
			body->health *= .90;
			v *= .25;
		}
	}

    if(fabs(body->angle - angle) > 180) {
		angle += 360;
	}
    float f = .2;
	body->angle = (int)(720 + body->angle * (1-f) + angle * f) % 720;
    float a = body->angle * M_PI / 180;
    body->pos.x += cos(a) * v;
    body->pos.y -= sin(a) * v;
    body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;

}

void body_draw(Game *game, Body *body, SDL_Surface *screen)
{
    SDL_Rect dst = {
		screen->w/2 + body->pos.x - game->player.pos.x - body->sprite->rotated_frame_size.x/2,
		screen->h/2 + body->pos.y - game->player.pos.y - body->sprite->rotated_frame_size.y/2,
		0,0
	};
	int x = (dst.x+body->sprite->rotated_frame_size.x/2)/game->hitmap_dec;
	int y = (dst.y+body->sprite->rotated_frame_size.y/2)/game->hitmap_dec;
	body->hitmap_in = 0;
	if(x>0 && x<game->hitmap_w &&
	   y>0 && y<game->hitmap_h) {
		int i = x+y*game->hitmap_w;
		body->hitmap_in = 1;
		body->hitmap_x = x;
		body->hitmap_y = y;
		game->hitmap[i] = MIN(game->hitmap[i] + body->health/2, 0x80);
		if(game->hitmap[i]>64)
			printf("hitmap x %d y %d = %d\n", x,y,game->hitmap[i] );
	}

    SDL_Rect src;
    sprite_rotated_rect(body->sprite, body->action, body->frame, body->angle, &src);
    SDL_BlitSurface( body->sprite->rotated, &src, screen, &dst );
}

State credit_event(Credit *credit, SDL_Event *event) { 
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_SPACE:
                case SDLK_RETURN:
                    return STATE_MENU;
            }
    }
    return STATE_CREDIT;
}

State gameover_event(Credit *credit, SDL_Event *event) { 
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_SPACE:
                case SDLK_RETURN:
                    return STATE_MENU;
            }
    }
    return STATE_GAMEOVER;
}

State menu_event(Menu *menu, SDL_Event *event) { 
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_ESCAPE:
                    return STATE_GAME;
                case SDLK_UP:
                    play_menu_select();
                    menu->selected = (menu->selected - 1 ) % MENU_COUNT;
                    break;
                case SDLK_DOWN:
                    play_menu_select();
                    menu->selected = (menu->selected + 1 ) % MENU_COUNT;
                    break;

                case SDLK_SPACE:
                case SDLK_RETURN:
                    play_menu_confirm();
                    switch(menu->selected) {
                        case MENU_START:  return STATE_GAME;
                        case MENU_CREDIT: return STATE_CREDIT;
                        case MENU_QUIT:   return STATE_QUIT;
                    }
                    break;
            }
    }
    return STATE_MENU;
}

State game_event(Game *game, SDL_Event *event) { 
    switch(event->type) {
        case SDL_QUIT:
            return STATE_QUIT;
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_ESCAPE:
                    return STATE_MENU;
                case SDLK_m:
                    break;

                case SDLK_p:
                    playPunch();
                    game->player.action = ACTION_ATTACK;
                    break;

                case SDLK_s:
                    playShot();
                    game->player.action = ACTION_MOVE;
                    break;

                case SDLK_i:
                    playPick();
                    break;
            }
            //nobreak, slip...
        case SDL_KEYUP:
            game->pressed[event->key.keysym.sym] = event->type == SDL_KEYDOWN;
            break;
    }
    return STATE_GAME;
}

int ysort_cmp(const void *a, const void *b)
{
	Body *aa = *(Body**)a, *bb = *(Body**)b;
	return (aa->pos.y > bb->pos.y) - (aa->pos.y < bb->pos.y);
}

void hud_setup(Game *game, SDL_Surface *screen){
   game->player_hud = IMG_Load_RW( SDL_RWFromMem(player_hud_png, player_hud_png_len), 1 );
}

void hud_timer(Game *game, char *timer) { 

    Uint32 elapsed = game->elapsed;
    Uint32 miliseconds = elapsed % 1000;
    Uint32 time = elapsed / 1000;
    Uint32 seconds = time % 60;
    Uint32 minutes = time % 3600 / 60;
    Uint32 hours = time / 3600;

    sprintf(timer, "%02d:%02d:%02d::%d", hours, minutes, seconds, miliseconds );
}

void hud_draw(Game *game, SDL_Surface *screen ){
    int health = (game->player.health < 0) ? 0 : game->player.health; 

    SDL_Rect src = {45, 15, health * 2, 15};
    SDL_FillRect(screen, &src, 0x0000ffff);
    SDL_BlitSurface( game->player_hud, NULL, screen, NULL );

    SDL_Rect hista_src = {45, 42, game->player.max_vel * 19 , 15};
    SDL_FillRect(screen, &hista_src, 0x00ff00ff);
    SDL_BlitSurface( game->player_hud, NULL, screen, NULL );


    char timer[40];
    hud_timer(game, &timer[0]);
    text_write_raw(screen, 300, 10, timer, white, 45);
}


State game_render(Game *game, SDL_Surface *screen)
{
    Uint32 ticks = SDL_GetTicks();
    game->elapsed = ticks - game->started;

	State state = STATE_GAME;
    // move player
    int i,n,x,y;

	if(game->enemy_count <= MAX_ENEMIES && (rand()%FPS) == 0)
		game->enemy_count ++;

    float dx=game->pressed[SDLK_RIGHT]-game->pressed[SDLK_LEFT];
    float dy=game->pressed[SDLK_DOWN]-game->pressed[SDLK_UP];
    if(fabs(dx)>0.1||fabs(dy)>0.1) {
        int angle = ATAN2(dx,dy);
        body_move(game, &game->player, angle);

    }

	// enemy move
	for(i=0; i < game->enemy_count; i++) {
		int angle, a_player = 
				ATAN2(
					game->player.pos.x-game->enemy[i].pos.x,
					game->player.pos.y-game->enemy[i].pos.y
				);
		
		if(game->enemy[i].health < game->enemy[i].max_health * .10)
			angle = rand()%360;
		else if(game->enemy[i].health < game->enemy[i].max_health * .20)
			angle = a_player + 180;
		else if(game->enemy[i].health < game->enemy[i].max_health * .33)
			angle = rand()%360;
		else
			angle = a_player;
			
		angle += (rand()%60)-30;
		body_move(game, &game->enemy[i], angle);
	}

    // CAMERA 
	int wsx0 = game->player.pos.x - screen->w/2;
	int wsy0 = game->player.pos.y - screen->h/2;
	int wsx1 = game->player.pos.x + screen->w/2;
	int wsy1 = game->player.pos.y + screen->h/2;
	int x0 = floor((float)wsx0/game->background->w);
	int y0 = floor((float)wsy0/game->background->h);
	int x1 = floor((float)wsx1/game->background->w);
	int y1 = floor((float)wsy1/game->background->h);
	for(x=x0; x<=x1; x++) {
		for(y=y0; y<=y1; y++) {
			int wmx0 = x*game->background->w;
			int wmy0 = y*game->background->h;
			int wmx1 = wmx0 + game->background->w;
			int wmy1 = wmy0 + game->background->h;

			int wix0 = MAX(wmx0, wsx0);
			int wiy0 = MAX(wmy0, wsy0);
			int wix1 = MIN(wmx1, wsx1);
			int wiy1 = MIN(wmy1, wsy1);

			int mix0 = wix0 - wmx0;
			int miy0 = wiy0 - wmy0;
			int mix1 = wix1 - wmx1;
			int miy1 = wiy1 - wmy1;

			int six0 = wix0 - wsx0;
			int siy0 = wiy0 - wsy0;
			int six1 = wix1 - wsx1;
			int siy1 = wiy1 - wsy1;

			//printf("a) q %d %d : ws %d %d : wm %d %d : wi %d %d : mi %d %d : si %d %d\n", x,y, wsx0,wsy0, wmx0,wmy0, wix0,wiy0, mix0,miy0, six0,siy0);
			//printf("b) q %d %d : ws %d %d : wm %d %d : wi %d %d : mi %d %d : si %d %d\n", x,y, wsx1,wsy1, wmx1,wmy1, wix1,wiy1, mix1,miy1, six1,siy1);
			SDL_Rect src = {mix0, miy0, mix1-mix0, miy1-miy0};
			SDL_Rect dst = {six0, siy0, six1-six0, siy1-siy0};
			SDL_BlitSurface( game->background, &src, screen, &dst );
		}
	}

    //SDL_LockSurface( screen );

	for(y=0; y<game->hitmap_h; y++) {
		Uint32 *p = (Uint32*)(((Uint8*)screen->pixels)+screen->pitch*(y*game->hitmap_dec+game->hitmap_dec/2)+game->hitmap_dec/2);
		for(x=0; x<game->hitmap_w; x++,p+=game->hitmap_dec) {
			Uint8 *c = (Uint8*)p;
			int v = (int)(c[1]+c[2]+1)/(c[3]/32+1);
			if(v<64) v = 0;
			game->hitmap[x+y*game->hitmap_w] = v;
		}
		//printf("\n");
	}
	
    //SDL_UnlockSurface( screen );

	// BODIES
	Body *body[1+MAX_ENEMIES];
	n=0;
	body[n++] = &game->player;
    for(i=0; i < game->enemy_count; i++,n++) {
		body[n] = &game->enemy[i];

		int dist = fabs(body[n]->pos.x-game->player.pos.x)+
				 fabs(body[n]->pos.y-game->player.pos.y);

		if(body[n]->action == ACTION_DEATH) {
			if(++body[n]->frame >= body[n]->sprite->frame_count) {
				body[n]->action = ACTION_MOVE;
				body[n]->health = body[n]->max_health;
				body[n]->max_vel = game->player.max_vel*(2+rand()%4)/5;
				float a = (rand()%180)/M_PI;
				int r = MAX(screen->w,screen->h);
				body[n]->pos.x = game->player.pos.x + cos(a) * r;
				body[n]->pos.y = game->player.pos.y + sin(a) * r;
				printf("ressurect %d %d\n",body[n]->pos.x, body[n]->pos.y);
			}
		} else if(
				(body[n]->health <= 0) || dist > (screen->w + screen->h)
		) {
			body[n]->action = ACTION_DEATH;
			body[n]->frame = 0;
			body[n]->angle = 0;
		} else {
			int attack_dist = 
				(body[n]->sprite->rotated_frame_size.x+
				 body[n]->sprite->rotated_frame_size.y);
				if((body[n]->health > body[n]->max_health * .25 ) && (dist < attack_dist/4)) {
					body[n]->action = ACTION_ATTACK;
					if(dist < attack_dist/8)
						game->player.health -= game->enemy[i].health*.25 *(1 - ((float)dist / (attack_dist/8)));
				} else {
					body[n]->action = ACTION_MOVE;
				}
		}

	}
	qsort(body, n, sizeof(body[0]), ysort_cmp); // blend ordering

    for(i=0;i<n;i++) {
        body_draw(game, body[i], screen);
    }

	if(game->player.action == ACTION_DEATH) {
		if(++game->player.frame >= game->player.sprite->frame_count) {
			game->player.action = ACTION_MOVE;
			game->player.health = game->player.max_health;
			game->player.pos.x =
			game->player.pos.y = 0;
			memset(game->pressed,0,sizeof(game->pressed));
			state = STATE_GAMEOVER;
		}
	} else if(game->player.health <= 0) {
		game->player.action = ACTION_DEATH;
		game->player.frame = 0;
	}


	if(game->heatmap) { // DEBUG HITMAP
		SDL_Surface *tile = SDL_CreateRGBSurface(SDL_HWSURFACE, 
				game->hitmap_dec, 
				game->hitmap_dec,
				RGB_FORMAT);
		SDL_FillRect(tile, NULL, 0xffffffff);

		for(y=0; y<game->hitmap_h; y++) {
			for(x=0; x<game->hitmap_w; x++) {
				SDL_Rect dst = {
					x*game->hitmap_dec,
					y*game->hitmap_dec,
					game->hitmap_dec,
					game->hitmap_dec
				};
				SDL_SetAlpha(tile, SDL_SRCALPHA, game->hitmap[x+y*game->hitmap_w]);
				SDL_BlitSurface( tile, NULL, screen, &dst );
			}
		}

		SDL_FreeSurface(tile);
	}
	
    hud_draw(game, screen);

	return state;
}

void menu_render(Menu *menu, SDL_Surface *screen)
{
    Uint32 ticks = SDL_GetTicks();

    int x = (1+cos(ticks/15000.0))*(menu->background->w/2-screen->w/2);
    int y = (1+sin(ticks/15000.0))*(menu->background->h/2-screen->h/2);

    SDL_Rect src = {x, y, screen->w, screen->h};
    SDL_BlitSurface( menu->background, &src, screen, NULL );

    text_write_raw(screen, 300, 100, "INFERNO", red, 96);
    text_write(screen, 100, 250, "new game", menu->selected ^ 0);
    text_write(screen, 100, 350, "continue game", menu->selected ^ 1);
    text_write(screen, 100, 450, "credits", menu->selected ^ 2);
    text_write(screen, 100, 550, "exit", menu->selected ^ 3);
}

void gameover_render(Game *game, SDL_Surface *screen)
{
    /*game_render(game, screen);*/
    SDL_BlitSurface( game->background, NULL, screen, NULL );
    text_write_raw(screen, 165, 250, "GAME OVER", red, 120);

    char timer[40];
    hud_timer(game, &timer[0]);
    text_write_raw(screen, 100, 400, "You survived, but not enough", white, 45);
    text_write_raw(screen, 530, 450, timer, white, 45);
    /*text_write_raw(screen, 200, 250, "programming", red, 36);*/
        /*text_write_raw(screen, 200, 300, "Carlo \"zED\" Caputo", white, 26);*/
        /*text_write_raw(screen, 200, 350, "Fernando Meyer", white, 26);*/

    /*text_write_raw(screen, 200, 400, "art", red, 36);*/
        /*text_write_raw(screen, 200, 450, "Cristine Ronchi", white, 26);*/

    /*text_write_raw(screen, 200, 500, "music", red, 36);*/
        /*text_write_raw(screen, 200, 550, "the DARK WOODS of FANTASY (C)1994 D.Barber. ", white, 16);*/
        /*text_write_raw(screen, 200, 570, "the fORESt RiveR (C)1994 D.Barber. ", white, 16);*/
        /*text_write_raw(screen, 200, 590, "spacewalk (c)1996 CB/Analogue ", white, 16);*/
        /*text_write_raw(screen, 200, 610, "effects by AKAI & Roland phanton soundengine", white, 16);*/
    /*[>text_write_raw(screen, 200, 550, "textures", red, 36);<]*/

}
void credit_render(Credit *credit, SDL_Surface *screen)
{
    Uint32 ticks = SDL_GetTicks();

    int x = (1+cos(ticks/15000.0))*(credit->background->w/2-screen->w/2);
    int y = (1+sin(ticks/15000.0))*(credit->background->h/2-screen->h/2);

    SDL_Rect src = {x, y, screen->w, screen->h};
    SDL_BlitSurface( credit->background, &src, screen, NULL );

    text_write_raw(screen, 300, 100, "CREDITS", red, 96);
    text_write_raw(screen, 200, 250, "programming", red, 36);
        text_write_raw(screen, 200, 300, "Carlo \"zED\" Caputo", white, 26);
        text_write_raw(screen, 200, 350, "Fernando Meyer", white, 26);

    text_write_raw(screen, 200, 400, "art", red, 36);
        text_write_raw(screen, 200, 450, "Cristine Ronchi", white, 26);

    text_write_raw(screen, 200, 500, "music", red, 36);
        text_write_raw(screen, 200, 550, "the DARK WOODS of FANTASY (C)1994 D.Barber. ", white, 16);
        text_write_raw(screen, 200, 570, "the fORESt RiveR (C)1994 D.Barber. ", white, 16);
        text_write_raw(screen, 200, 590, "spacewalk (c)1996 CB/Analogue ", white, 16);
        text_write_raw(screen, 200, 610, "effects by AKAI & Roland phanton soundengine", white, 16);
    /*text_write_raw(screen, 200, 550, "textures", red, 36);*/

}

void timing_control(Uint32 start) { 
    Uint32 end = SDL_GetTicks();
    int actual_delta = end - start;
    int expected_delta = 1000/FPS;
    int delay = MAX(0, expected_delta - actual_delta);
    SDL_Delay(delay);
}

// global runing
//


void load_screen(SDL_Surface *screen) { 
    SDL_Surface *tmp_bg = IMG_Load_RW( SDL_RWFromMem(logo_pseudo_png, logo_pseudo_png_len), 1 );
    SDL_Rect src = {-350, -250, 1022, 768};
    SDL_FillRect(screen, NULL, 0xFFFFFFFF);
    SDL_BlitSurface( tmp_bg, &src, screen, NULL );
    SDL_Flip(screen);
    SDL_FreeSurface(tmp_bg);
    SDL_Delay(1000);
    tmp_bg = IMG_Load_RW( SDL_RWFromMem(creditos_jpg, creditos_jpg_len), 1 );
    SDL_FillRect(screen, NULL, 0xFFFFFFFF);
    SDL_BlitSurface( tmp_bg, NULL, screen, NULL );
    SDL_Flip(screen);
    SDL_Delay(1000);
    SDL_FreeSurface(tmp_bg);
} 

int toggle_fullscreen(int fullscreen) {

    if(fullscreen) {
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,  SDL_DOUBLEBUF | SDL_ANYFORMAT);
    }
    else {
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_DOUBLEBUF | SDL_ANYFORMAT | SDL_FULLSCREEN);
    }
    fullscreen = !fullscreen;
    return(fullscreen);
}

int main( int argc, char* args[] )
{

    App app; 

    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );

#if 0
    { // window manager
        SDL_Surface* icon = SDL_CreateRGBSurface(SDL_HWSURFACE, 64, 64, RGB_FORMAT);
        SDL_Rect src = {32,32,64,64};
        SDL_Rect dst = {0,0,0,0};
        SDL_BlitSurface( sprite, &src, icon, &dst );
        SDL_WM_SetIcon(icon, NULL);
        SDL_FreeSurface( icon );
        SDL_WM_SetCaption("inferno", "inferno");
    }
#endif

    // init font system 
    init_font();

    // music manager
    initMusic();

    // effects manager 
    loadEffects();

    /*handle_menu_music();*/
    
    app.screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE );


    load_screen(app.screen);


    SDL_Surface *tmp_bg = IMG_Load_RW( SDL_RWFromMem(mapa_jpg, mapa_jpg_len), 1 );
    app.background = zoomSurface(tmp_bg, 2, 2, 1);
    app.menu.background = app.background;
    app.game.background = app.background;
    app.credit.background = app.background;
    SDL_FreeSurface(tmp_bg);

	app.game.heatmap = 0;

	app.game.hitmap_dec = 24;
	app.game.hitmap_w = app.screen->w/app.game.hitmap_dec;
	app.game.hitmap_h = app.screen->h/app.game.hitmap_dec;
	app.game.hitmap_len = app.game.hitmap_w*app.game.hitmap_h;
	app.game.hitmap = malloc(app.game.hitmap_len);
	memset(app.game.hitmap, 0, app.game.hitmap_len);
	printf("%d %d %d\n", app.game.hitmap_w,app.game.hitmap_h,app.game.hitmap_len);

    // player setup
    sprite_init(&app.game.hero, 
            0, 0, // origin
            114, 114, 13, // frame size and count
            hero_png, hero_png_len // source
            );
    sprite_init(&app.game.zombie, 
            0, 0, // origin
            114, 114, 13, // frame size and count
            zombie_png, zombie_png_len // source
            );

	app.game.player.sprite = &app.game.hero;
	app.game.player.max_vel = 10;
	app.game.player.health = 
	app.game.player.max_health = 100;
	app.game.player.action = ACTION_MOVE;
	app.game.player.frame = 0;
	app.game.player.pos.y = 
	app.game.player.pos.x = 0;

    int i;
    for(i=0;i<MAX_ENEMIES;i++) {
		app.game.enemy[i].sprite = &app.game.zombie;
		app.game.enemy[i].max_health = 25;
		app.game.enemy[i].max_vel = 5;
		app.game.enemy[i].health = 0;
		app.game.enemy[i].action = ACTION_DEATH;
		app.game.enemy[i].frame = app.game.enemy[i].sprite->frame_count;
    }

	app.game.enemy_count = 0;
    memset(app.game.pressed, 0, sizeof(app.game.pressed));
    app.state = STATE_MENU;
    handle_menu_music();
    app.menu.selected = 0; 
    int last_state = 0;
    int fullscreen = 0;


    hud_setup(&app.game, app.screen); 

    // main loop

    while(app.state != STATE_QUIT) {
        Uint32 start = SDL_GetTicks();
        SDL_Event event;
        if( SDL_PollEvent( &event ) )
        {
            switch(app.state) {
                case STATE_GAME: app.state = game_event  (&app.game,   &event); break;
                case STATE_MENU:   app.state = menu_event  (&app.menu,   &event); break;
                case STATE_CREDIT: app.state = credit_event(&app.credit, &event); break;
                case STATE_GAMEOVER: app.state = gameover_event(&app.credit, &event); break;
            }

            switch(event.type) {
                case SDL_QUIT:
                    app.state = STATE_QUIT;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_q:
                            app.state = STATE_QUIT;
							break;
                        case SDLK_h:
                            app.game.heatmap ^= 1;
							break;
                        case SDLK_f:
                            /*fullscreen = toggle_fullscreen(fullscreen);*/
                            break;
                    }
            }
        }

        SDL_FillRect(app.screen, NULL, 0);
        switch(app.state) {
            case STATE_GAME:   
                app.state = game_render  (&app.game,   app.screen); 
                if(last_state != STATE_GAME){
                    app.game.started = SDL_GetTicks();
                    halt_music();
                    handle_ingame_music(); 
                }
                break;
            case STATE_MENU:   
                menu_render  (&app.menu,   app.screen);
                if(last_state != STATE_MENU){
                    halt_music();
                    handle_menu_music(); 
                }
                break;

            case STATE_CREDIT: 
                credit_render(&app.credit, app.screen); 
                if(last_state != STATE_CREDIT){
                    halt_music();
                    handle_credit_music(); 
                }
                break;
            case STATE_GAMEOVER: 
                gameover_render(&app.game, app.screen); 
                if(last_state != STATE_CREDIT){
                    /*halt_music();*/
                    /*handle_gameover_music(); */
                }
                break;
        }
        SDL_Flip( app.screen );
        timing_control(start);
        last_state = app.state;
    }

    // TODO free surfaces, like SDL_FreeSurface( sprite ); hitmap

    SDL_Quit();

    return 0;
}
