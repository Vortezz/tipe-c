#include <SDL2/SDL.h>
#include <stdbool.h>

// Represents the size of the window in tiles
const int GRID_SIZE = 64;
// Represents the size of a tile in pixels
int TILE_SIZE = 8;

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
	GRASS,
	FIRE,
	BURNT
} TileType;

typedef struct {
	TileType default_type;
	TileType current_type;
	int state;
} Tile;

typedef struct {
	Tile ** data;
	Window window;
	int model;
	bool ended;
	int coord_x;
	int coord_y;
} Grid;

Color get_color(TileType type, int state) {
	switch (type) {
		case TREE:
			return (Color) {0, 150, 0};
		case WATER:
			return (Color) {0, 0, 255};
		case GRASS:
			return (Color) {0, 255, 0};
		case FIRE:
			switch (state) {
				case 0:
					return (Color) {255, 0, 0};
				case 1:
					return (Color) {150, 0, 0};
				default:
					return (Color) {255, 0, 0};
			}
		case BURNT:
			return (Color) {100, 100, 100};
	}
}