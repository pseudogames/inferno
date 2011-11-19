PROGRAM=inferno


all: $(PROGRAM)

clean:
	rm -v $(PROGRAM)

$(PROGRAM): main.c
	gcc $^ `sdl-config --cflags --libs` -o $@
	
