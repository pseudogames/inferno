
all: inferno

clean:
	rm -fv inferno inferno.exe sprite.jpg sprite.c main.o sprite.o

sprite.jpg: ~/Dropbox/inferno/zombie_topdown.png 
	convert $< $@

sprite.c: sprite.jpg
	xxd -i $^ > $@

sprite.o: sprite.c
	gcc $< -c -o $@

main.o: main.c
	gcc $< `sdl-config --cflags` -c -o $@

sound.o: sound.c
	gcc $< `sdl-config --cflags` -c -o $@

inferno: main.o sprite.o sound.o
	gcc $^ -lSDL_gfx -lSDL_image -lSDL_mixer `sdl-config --libs` -o $@

