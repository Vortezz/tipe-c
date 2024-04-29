#include <SDL2/SDL.h>

// Represents the size of the window in tiles
const int GRID_SIZE = 32;
// Represents the size of a tile in pixels
const int TILE_SIZE = 16;

typedef struct {
	SDL_Window * window;
	SDL_Surface * surface;
} Window;

typedef struct {
	int r;
	int g;
	int b;
} Color;

typedef struct {
	int x;
	int y;
} Point;

typedef enum {
	TREE,
	WATER,
	GRASS
} TileType;

typedef struct {
	TileType ** data;
	Window window;
} Grid;

Color get_color(TileType type) {
	switch (type) {
		case TREE:
			return (Color) {0, 150, 0};
		case WATER:
			return (Color) {0, 0, 255};
		case GRASS:
			return (Color) {0, 255, 0};
	}
}