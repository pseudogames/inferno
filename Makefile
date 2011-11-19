CFLAGS=-g

all: inferno

clean:
	rm -fv inferno inferno.exe sprite.jpg sprite.c main.o sprite.o

sprite.jpg: ~/Dropbox/inferno/zombie_topdown.png 
	convert $< $@

sprite.c: sprite.jpg
	xxd -i $^ > $@

sprite.o: sprite.c
	gcc $(CFLAGS) $< -c -o $@

main.o: main.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

inferno: main.o sprite.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -lSDL_gfx -lSDL_image `sdl-config --libs` -o $@

debug: inferno
	gdb $<
