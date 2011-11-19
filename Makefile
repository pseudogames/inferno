CFLAGS=-g

all: inferno

clean:
	rm -fv inferno inferno.exe sprite.png sprite.c main.o sprite.o

sprite.png: ~/Dropbox/inferno/inferno_zombie_1.png 
	convert $< $@

sprite.c: sprite.png
	xxd -i $^ > $@

sprite.o: sprite.c
	gcc $(CFLAGS) $< -c -o $@

main.o: main.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

sound.o: sound.c
	gcc $< `sdl-config --cflags` -c -o $@

inferno: main.o sprite.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -lSDL_gfx -lSDL_image -lSDL_mixer `sdl-config --libs` -o $@

debug: inferno
	gdb $<
