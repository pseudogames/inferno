CFLAGS=-g

all: inferno music.ogg

clean:
	rm -fv inferno inferno.exe sprite.jpg sprite.c main.o sprite.o

sprite.jpg: ~/Dropbox/inferno/zombie_topdown.png 
	convert $< $@

music.ogg: ~/Dropbox/inferno/music.ogg
	cp $< $@

sprite.c: sprite.jpg
	xxd -i $^ > $@

sprite.o: sprite.c
	gcc $(CFLAGS) $< -c -o $@

main.o: main.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

sound.o: sound.c
	gcc $< `sdl-config --cflags` -c -o $@

inferno: main.o sprite.o sound.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -lSDL_gfx -lSDL_image -lSDL_mixer `sdl-config --libs` -o $@

debug: inferno
	gdb $<
