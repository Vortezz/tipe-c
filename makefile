build:
	gcc -o main main.c `sdl2-config --cflags --libs` -lcjson

clear:
	rm -f main

run:
	./main

all: clear build run