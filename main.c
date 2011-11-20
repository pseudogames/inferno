#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <strings.h>
#include <stdio.h>
#include <math.h>

#include "sound.h"

#define FPS 9
#define MAX_ENEMIES 9
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

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
    STATE_MENU = 0,
    STATE_GAME, 
    STATE_PAUSE,
    STATE_CREDIT,
    STATE_QUIT
} State; 

typedef enum { 
    MENU_START = 0,
    MENU_CREDIT, 
    MENU_QUIT,
    MENU_COUNT
} MenuItem; 

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


typedef struct{
    Sprite zombie;
    Body player;
    Body enemy[MAX_ENEMIES];
    int pressed[SDLK_LAST];
} Game;

typedef struct{
    SDL_Surface *image; 
    MenuItem selected;
} Menu;

typedef struct{
    SDL_Surface *image; 
} Credit;

typedef struct{
    SDL_Surface *screen; 
    Game game;
    Menu menu;
    Credit credit;
    State state;
} App;


void sprite_origin_rect(Sprite *sprite, Action action, int frame, SDL_Rect *rect)
{
    frame = frame % sprite->count;
    rect->x = sprite->origin.x + frame *sprite->frame_size.x;
    rect->y = sprite->origin.y + action*sprite->frame_size.y;
    rect->w = sprite->frame_size.x;
    rect->h = sprite->frame_size.y;
}

#define ANGLE_STEP 45
#define ZOOM .8

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

    sprite->rotated = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 
            sprite->rotated_frame_size.x * sprite->count * 360/ANGLE_STEP,
            sprite->rotated_frame_size.y * ACTION_COUNT,
            32, 0,0,0,0);
    if(sprite->rotated == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    printf("rotation cache size %d %d for %d angles\n", sprite->rotated->w, sprite->rotated->h, 360/ANGLE_STEP);
    SDL_SetAlpha(sprite->rotated, SDL_SRCALPHA, 0xff);

    SDL_Surface *element = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 
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
                SDL_FillRect(element, NULL, 0xff0000);
                SDL_BlitSurface( sprite->source, &src, element, NULL );
                SDL_Surface *rotozoom = rotozoomSurface(element, angle, ZOOM, SMOOTHING_ON);
                dst.x += dst.w/2 - rotozoom->w/2;
                dst.y += dst.h/2 - rotozoom->h/2; // center
                SDL_SetAlpha(rotozoom,SDL_SRCALPHA,0xff);
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
    sprite->count = c;
    sprite->source = IMG_Load_RW( SDL_RWFromMem(img, img_size), 1 );
    sprite->rotated = NULL;
    sprite_gen_rotation(sprite);
}

void body_init(Body *body, Sprite *sprite, int max_health, float max_vel, int x, int y)
{
    memset(body,0,sizeof(Body));
    body->health = body->max_health = max_health;
    body->max_vel = max_vel;
    body->sprite = sprite;
    body->pos.x = x;
    body->pos.y = y;
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

State menu_event(Menu *menu, SDL_Event *event) { 
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_ESCAPE:
                    return STATE_GAME;
                case SDLK_UP:
                    menu->selected = (menu->selected - 1 ) % MENU_COUNT;
                    break;
                case SDLK_DOWN:
                    menu->selected = (menu->selected + 1 ) % MENU_COUNT;
                    break;

                case SDLK_SPACE:
                case SDLK_RETURN:
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
                    handleMusic();
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

void game_render(Game *game, SDL_Surface *screen)
{
    // move player
    float up=0,down=0,left=0,right=0;
    float accel=.2;
    int i;

    up   =   up*(1-accel)+game->pressed[SDLK_UP   ]*accel;
    down = down*(1-accel)+game->pressed[SDLK_DOWN ]*accel;
    left = left*(1-accel)+game->pressed[SDLK_LEFT ]*accel;
    right=right*(1-accel)+game->pressed[SDLK_RIGHT]*accel;
    float dx=right-left;
    float dy=down-up;
    if(fabs(dx)>0.1||fabs(dy)>0.1) {
        int angle = (int)(720+atan2(-dy,dx)*180/M_PI)%360;
        body_move(&game->player, angle);
        for(i=0;i<MAX_ENEMIES;i++) {
            body_move(&game->enemy[i], angle+rand()%10);
        }
    }


    // clean game.screen
    SDL_FillRect(screen,NULL, 0);

    for(i=0;i<MAX_ENEMIES;i++) {
        body_draw(&game->enemy[i], screen);
    }

    body_draw(&game->player, screen);
}

void menu_render(Menu *menu, SDL_Surface *screen)
{
    // TODO
}

void credit_render(Credit *credit, SDL_Surface *screen)
{
    // TODO
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

    App app; 

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

    app.screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );

    // player setup
    sprite_init(&app.game.zombie, 
            0, 0, // origin
            114, 114, 13, // frame size and count
            sprite_png, sprite_png_len // source
            );

    body_init(&app.game.player, &app.game.zombie,
            100, // health
            10, // speed
            app.screen->w/2-app.game.zombie.frame_size.x/2,
            app.screen->h/2-app.game.zombie.frame_size.y/2
            );

    int i;
    for(i=0;i<MAX_ENEMIES;i++) {
        body_init(
                &app.game.enemy[i],
                &app.game.zombie,
                25, // health
                5, // speed
                rand() % app.screen->w, // x
                rand() % app.screen->h // y
                );
    }

    memset(app.game.pressed, 0, sizeof(app.game.pressed));

    // main loop

    while(app.state != STATE_QUIT) {
        Uint32 start = SDL_GetTicks();
        SDL_Event event;
        if( SDL_PollEvent( &event ) )
        {
            switch(app.state) {
                case STATE_GAME:   app.state = game_event  (&app.game,   &event); break;
                case STATE_MENU:   app.state = menu_event  (&app.menu,   &event); break;
                case STATE_CREDIT: app.state = credit_event(&app.credit, &event); break;
            }

            switch(event.type) {
                case SDL_QUIT:
                    app.state = STATE_QUIT;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_q:
                            app.state = STATE_QUIT;
                    }
            }


        }

        SDL_FillRect(app.screen, NULL, 0);
        switch(app.state) {
            case STATE_GAME:   game_render  (&app.game,   app.screen); break;
            case STATE_MENU:   menu_render  (&app.menu,   app.screen); break;
            case STATE_CREDIT: credit_render(&app.credit, app.screen); break;
        }
        SDL_Flip( app.screen );

        timing_control(start);
    }

    // TODO free surfaces, like SDL_FreeSurface( sprite );

    SDL_Quit();

    return 0;
}
