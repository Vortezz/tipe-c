build:
	gcc -o main main.c `sdl2-config --cflags --libs` -lcjson -lpng -ldl -lm

clear:
	rm -f main

run:
	./main

all: clear build run