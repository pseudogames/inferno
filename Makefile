CFLAGS=-g

all: inferno

clean:
	rm -fv inferno inferno.exe *.png *.o *.wav *.ogg *.jpg *.ttf acid.c adler.c mapa.c hero.c zombie.c


# FIXME troll mode
m60.wav: ~/Dropbox/inferno/m60.wav 
	cp -v $< $@
pick.wav: ~/Dropbox/inferno/pick.wav
	cp -v $< $@
shot.wav: ~/Dropbox/inferno/shot.wav
	cp -v $< $@
punch0.wav: ~/Dropbox/inferno/punch0.wav
	cp -v $< $@
punch1.wav: ~/Dropbox/inferno/punch1.wav
	cp -v $< $@
punch2.wav: ~/Dropbox/inferno/punch2.wav
	cp -v $< $@
punch3.wav: ~/Dropbox/inferno/punch3.wav
	cp -v $< $@
punch4.wav: ~/Dropbox/inferno/punch4.wav
	cp -v $< $@
acid.ttf: ~/Dropbox/inferno/acid.ttf
	cp -v $< $@
adler.ttf: ~/Dropbox/inferno/adler.ttf
	cp -v $< $@
menu.wav: ~/Dropbox/inferno/menu.wav
	cp -v $< $@
menu_select.wav: ~/Dropbox/inferno/menu_select.wav
	cp -v $< $@
zombie.png: ~/Dropbox/inferno/inferno_zombie_1.png 
	cp -v $< $@
hero.png: ~/Dropbox/inferno/inferno_hero.png 
	cp -v $< $@

%.c : %.wav
	xxd -i $< > $@

%.c : %.ttf
	xxd -i $< > $@

%.c: %.png
	xxd -i $^ > $@

%.c: %.jpg
	xxd -i $^ > $@

%.o : %.c
	gcc $(CFLAGS) $< -c -o $@

%.c : %.ogg
	xxd -i $< > $@

music.ogg: ~/Dropbox/inferno/music.ogg
	cp $< $@

music_menu.ogg: ~/Dropbox/inferno/music_menu.ogg
	cp $< $@

music_ingame.ogg: ~/Dropbox/inferno/music_ingame.ogg
	cp $< $@

mapa.jpg: ~/Dropbox/inferno/mapa.jpg
	cp $< $@

main.o: main.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

sound.o: sound.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

font.o: font.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

acid.c: ~/Dropbox/inferno/acid.ttf
adler.c: ~/Dropbox/inferno/adler.ttf

inferno: main.o sound.o font.o m60.o pick.o shot.o punch0.o punch1.o punch2.o punch3.o punch4.o adler.o acid.o mapa.o menu.o menu_select.o music_ingame.o music_menu.o hero.o zombie.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -lSDL_gfx -lSDL_image -lSDL_mixer -lSDL_ttf `sdl-config --libs` -o $@

debug: inferno
	gdb $<


