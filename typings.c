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
			return (Color) {30, 81, 52};
		case WATER:
			return (Color) {113, 175, 172};
		case GRASS:
			return (Color) {53, 105, 74};
		case FIRE:
			switch (state) {
				case 0:
					return (Color) {253, 54, 23};
				case 1:
					return (Color) {255, 108, 46};
				default:
					return (Color) {253, 54, 23};
			}
		case BURNT:
			return (Color) {78, 78, 78};
	}
}