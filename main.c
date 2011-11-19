#include "SDL.h"
#include "stdio.h"

int main( int argc, char* args[] )
{
    SDL_Surface* sprite = NULL;
    SDL_Surface* screen = NULL;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );

	screen = SDL_SetVideoMode( 1024, 768, 32, SDL_SWSURFACE );

	sprite = SDL_LoadBMP( "sprite.bmp" );

	int dx=0, dy=0;
	int x=0, y=0;
	int i=0;
	int d=4;

	int running = 1;
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
		SDL_Delay(100);
	}

    SDL_FreeSurface( sprite );

    SDL_Quit();

    return 0;
}
