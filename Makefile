CFLAGS=-g

all: inferno

clean:
	rm -fv inferno inferno.exe *.png *.o *.wav *.ogg *.jpg *.ttf acid.c adler.c mapa.c hero.c zombie.c fogo.c


# FIXME troll mode
m60.wav: ./data/m60.wav 
	cp -v $< $@
pick.wav: ./data/pick.wav
	cp -v $< $@
shot.wav: ./data/shot.wav
	cp -v $< $@
punch0.wav: ./data/punch0.wav
	cp -v $< $@
punch1.wav: ./data/punch1.wav
	cp -v $< $@
punch2.wav: ./data/punch2.wav
	cp -v $< $@
punch3.wav: ./data/punch3.wav
	cp -v $< $@
punch4.wav: ./data/punch4.wav
	cp -v $< $@
acid.ttf: ./data/acid.ttf
	cp -v $< $@
adler.ttf: ./data/adler.ttf
	cp -v $< $@
menu.wav: ./data/menu.wav
	cp -v $< $@
menu_select.wav: ./data/menu_select.wav
	cp -v $< $@
zombie.png: ./data/inferno_zombie_1.png 
	cp -v $< $@
hero.png: ./data/inferno_hero.png 
	cp -v $< $@
fogo.png: ./data/fogo.png 
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

music.ogg: ./data/music.ogg
	cp $< $@

music_menu.ogg: ./data/music_menu.ogg
	cp $< $@

music_ingame.ogg: ./data/music_ingame.ogg
	cp $< $@

music_credit.ogg: ./data/music_credit.ogg
	cp $< $@

mapa.jpg: ./data/mapa.jpg
	cp $< $@

player_hud.png: ./data/player_hud.png
	cp $< $@

stats_hud.png: ./data/stats_hud.png
	cp $< $@

caveira.png: ./data/caveira.png
	cp $< $@

logo_pseudo.png: ./data/logo_pseudo.png
	cp $< $@

creditos.jpg: ./data/creditos.jpg
	cp $< $@

main.o: main.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

sound.o: sound.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

font.o: font.c
	gcc $(CFLAGS) $< `sdl-config --cflags` -c -o $@

acid.c: ./data/acid.ttf
adler.c: ./data/adler.ttf

inferno: main.o highscore.o sound.o font.o m60.o pick.o shot.o punch0.o punch1.o punch2.o punch3.o punch4.o adler.o acid.o mapa.o menu.o menu_select.o music_credit.o    music_ingame.o music_menu.o hero.o zombie.o logo_pseudo.o creditos.o  player_hud.o stats_hud.o fogo.o caveira.o

	gcc $(CFLAGS) $(LDFLAGS) $^ -lSDL_gfx -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL_net `sdl-config --libs` -o $@

debug: inferno
	gdb $<


