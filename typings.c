#include <SDL2/SDL.h>
#include <stdbool.h>

/**
 * Represents the size of the grid
 */
const int GRID_SIZE = 256;
/**
 * Represents the size of a tile
 */
int TILE_SIZE = 2;

/**
 * Represents a window
 */
typedef struct {
	/**
	 * The SDL window
	 */
	SDL_Window * window;
	/**
	 * The SDL surface
	 */
	SDL_Surface * surface;
} Window;

/**
 * Represents a color
 */
typedef struct {
	/**
	 * The red component of the color (0-255)
	 */
	int r;
	/**
	 * The green component of the color (0-255)
	 */
	int g;
	/**
	 * The blue component of the color (0-255)
	 */
	int b;
} Color;

/**
 * Represents a point
 */
typedef struct {
	/**
	 * The x coordinate of the point
	 */
	int x;
	/**
	 * The y coordinate of the point
	 */
	int y;
} Point;

/**
 * Represents a tile type
 */
typedef enum {
	/**
	 * A tree tile
	 */
	TREE,
	/**
	 * A dense tree tile
	 */
	DENSE_TREE,
	/**
	 * A water tile
	 */
	WATER,
	/**
	 * A grass tile
	 */
	GRASS,
	/**
	 * A fire tile
	 */
	FIRE,
	/**
	 * A burnt tile
	 */
	BURNT,
	/**
	 * A trench tile
	 */
	TRENCH,
	/**
	 * Just to have a size for the enum
	 */
	TILE_TYPE_SIZE
} TileType;

/**
 * Represents a tile
 */
typedef struct {
	/**
	 * The default type of the tile
	 */
	TileType default_type;
	/**
	 * The current type of the tile
	 */
	TileType current_type;
	/**
	 * The state of the tile (for example, the state of a fire)
	 */
	int state;
} Tile;

/**
 * Represents a grid
 */
typedef struct {
	/**
	 * The data of the grid
	 */
	Tile ** data;
	/**
	 * The window of the grid
	 */
	Window window;
	/**
	 * The model of the grid
	 */
	int model;
	/**
	 * Whether the grid has ended
	 */
	bool ended;
	/**
	 * The x coordinate of the grid
	 */
	int coord_x;
	/**
	 * The y coordinate of the grid
	 */
	int coord_y;
	/**
	 * Whether to save the content into a png file
	 */
	bool export_png;
	/**
	 * Whether to save the content into a csv file
	 */
	bool export_csv;
	/**
	 * Wind direction
	 */
	double wind_direction;
	/**
	 * Wind speed
	 */
	double wind_speed;
} Grid;

/**
 * Get a color according to a tile type and a state
 *
 * @param type The type of the tile
 * @param state The state of the tile
 * @return The color of the tile
 */
Color get_color(TileType type, int state) {
	switch (type) {
		case TREE:
			return (Color) {30, 81, 52};
		case DENSE_TREE:
			return (Color) {18, 49, 33};
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
		case TRENCH:
			return (Color) {77, 5, 0};
	}

	return (Color) {0, 0, 0};
}