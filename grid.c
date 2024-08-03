#include "draw.c"
#include <cjson/cJSON.h>
#include <unistd.h>

/**
 * Model 0 constants
 *
 * The following probabilities are 1 / [number]
 */
/**
 * The probability for a tree tile to burn
 */
const int M0_PROBA_TREE_BURN = 8;
/**
 * The probability for a grass tile to burn
 */
const int M0_PROBA_GRASS_BURN = 8;
/**
 * The probability for a tile to change state between fire and burnt
 */
const int M0_PROBA_STATE_CHANGE = 16;

/**
 * Model 1 constants
 *
 * The following probabilities are 1 / [number]
 */
/**
 * The probability for a tree tile to burn (in direct neighbours)
 */
const int M1_C_PROBA_TREE_BURN = 8;
/**
 * The probability for a grass tile to burn (in direct neighbours)
 */
const int M1_C_PROBA_GRASS_BURN = 8;
/**
 * The probability for a tree tile to burn (in diagonal neighbours)
 */
const int M1_D_PROBA_TREE_BURN = 16;
/**
 * The probability for a grass tile to burn (in diagonal neighbours)
 */
const int M1_D_PROBA_GRASS_BURN = 16;
/**
 * The probability for a tile to change state between fire and burnt
 */
const int M1_PROBA_STATE_CHANGE = 16;

/**
 * Create a grid
 *
 * @param model The model of the grid
 * @param window The window to draw the grid
 * @param coord_x The x coordinate of the grid
 * @param coord_y The y coordinate of the grid
 * @return The created grid
 */
Grid create_grid(int model, Window window, int coord_x, int coord_y) {
	// Create the grid
	Grid grid = {
			.data = (Tile **) malloc(GRID_SIZE * sizeof(*grid.data)),
			.window = window,
			.model = model,
			.ended = false,
			.coord_x = coord_x,
			.coord_y = coord_y
	};

	// Initialize the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		grid.data[i] = (Tile *) malloc(GRID_SIZE * sizeof(*grid.data[i]));
	}

	// Load the grid from a json file if it exists, otherwise create a random grid
	if (access("grid.json", F_OK) == 0) {
		// The json file exists, we load the grid from it
		cJSON * grid_json = cJSON_Parse(readfile(fopen("grid.json", "r")));
		cJSON * grid_json_object = cJSON_GetObjectItem(grid_json, "grid");

		for (int i = 0; i < GRID_SIZE; i++) {
			cJSON * row = cJSON_GetArrayItem(grid_json_object, i);
			for (int j = 0; j < GRID_SIZE; j++) {
				// Get the value of the tile and set it to the grid
				int value = cJSON_GetArrayItem(row, j)->valueint;

				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
			}
		}
	} else {
		// The json file does not exist, we create a random grid
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				// Get a random value between 0 and 3 and set it to the grid
				int value = get_random(4);

				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
			}
		}
	}

	return grid;
}

/**
 * Copy a grid
 *
 * @param data The grid to copy
 * @return The copied grid
 */
Tile ** copy_grid(Tile ** data) {
	// Malloc the copy of the grid
	Tile ** copy = (Tile **) malloc(GRID_SIZE * sizeof(*copy));

	// Fill the copy with the data of the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		copy[i] = (Tile *) malloc(GRID_SIZE * sizeof(*copy[i]));
		memcpy(copy[i], data[i], GRID_SIZE * sizeof(*copy[i]));
	}

	return copy;
}

/**
 * Get the tile at a point
 *
 * @param grid The grid
 * @param point The point
 * @return The tile at the point
 */
Tile get_tile(Grid grid, Point point) {
	return grid.data[point.x][point.y];
}

/**
 * Get the direct neighbours of a point
 *
 * @param grid The grid
 * @param point The point
 * @return The neighbours of the point
 */
Point * get_direct_neighbours(Grid * grid, Point point) {
	Point * neighbours = (Point *) malloc(4 * sizeof(*neighbours));

	neighbours[0] = (Point) {point.x - 1, point.y};
	neighbours[1] = (Point) {point.x + 1, point.y};
	neighbours[2] = (Point) {point.x, point.y - 1};
	neighbours[3] = (Point) {point.x, point.y + 1};

	return neighbours;
}

/**
 * Get the diagonal neighbours of a point
 *
 * @param grid The grid
 * @param point The point
 * @return The neighbours of the point
 */
Point * get_diagonal_neighbours(Grid * grid, Point point) {
	Point * neighbours = (Point *) malloc(4 * sizeof(*neighbours));

	neighbours[0] = (Point) {point.x - 1, point.y - 1};
	neighbours[1] = (Point) {point.x + 1, point.y - 1};
	neighbours[2] = (Point) {point.x - 1, point.y + 1};
	neighbours[3] = (Point) {point.x + 1, point.y + 1};

	return neighbours;
}

/**
 * Check if a point is valid (ie inside the grid)
 *
 * @param point The point to check
 * @return True if the point is valid, false otherwise
 */
bool is_valid(Point point) {
	return point.x >= 0 && point.x < GRID_SIZE && point.y >= 0 && point.y < GRID_SIZE;
}

/**
 * Check if the grid is ended
 *
 * @param grid The grid to check
 * @return True if the grid is ended, false otherwise
 */
bool is_ended(Grid grid) {
	if (grid.model == 0 || grid.model == 1) {
		bool is_fire = false;

		// Check if there is no more fire, if there is no more fire, the grid is ended
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				if (grid.data[i][j].current_type == FIRE) {
					is_fire = true;
					break;
				}
			}
		}

		return !is_fire;
	} else {
		// Unknown model
		return true;
	}
}

/**
 * Check a probability : if the tile is of the given type and the probability is valid
 *
 * @param grid The grid
 * @param point The point to check
 * @param type The type to check
 * @param proba The probability
 * @return True if the probability is valid, false otherwise
 */
bool check_probability(Grid * grid, Point point, TileType type, int proba) {
	return get_tile(*grid, point).current_type == type && get_random(proba) == 0;
}

/**
 * Apply the rules to a cell (model 0 and 1)
 *
 * @param grid The grid
 * @param copy The copy of the grid
 * @param point The point to apply the rules to
 * @param neighbours The neighbours of the point
 * @param tree_burn The probability for a tree tile to burn
 * @param grass_burn The probability for a grass tile to burn
 * @param state_change The probability for a tile to change state between fire and burnt
 */
void apply_to_cell(Grid * grid, Tile ** copy, Point point, Point * neighbours, int tree_burn, int grass_burn,
				   int state_change) {
	// First step, change the state of the neighbours based on the probability
	for (int k = 0; k < 4; k++) {
		if (is_valid(neighbours[k])) {
			if (check_probability(grid, neighbours[k], TREE, tree_burn) ||
				check_probability(grid, neighbours[k], GRASS, grass_burn)) {
				Tile * tile_copy = &copy[neighbours[k].x][neighbours[k].y];

				tile_copy->current_type = FIRE;
				tile_copy->state = 0;
			}
		}
	}

	// Second step, change the state of the point based on the probability to a new state or to burnt
	Tile point_tile = get_tile(*grid, point);
	if (check_probability(grid, point, FIRE, state_change)) {
		Tile * tile_copy = &copy[point.x][point.y];

		// If the tile is newly on fire, we increment the state of the tile, otherwise we set it to burnt
		if (point_tile.state == 0) {
			tile_copy->state++;
		} else {
			tile_copy->current_type = BURNT;
			tile_copy->state = 0;
		}
	}

	free(neighbours);
}

/**
 * Update the grid
 *
 * @param grid The grid to update
 */
void tick(Grid * grid) {
	Tile ** copy = copy_grid(grid->data);

	// Update the grid based on the model
	if (grid->model == 0) {
		// MODEL 0 -> 4 neighbours
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue
				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				// Apply the rules to the cell
				apply_to_cell(grid, copy, point, get_direct_neighbours(grid, point), M0_PROBA_TREE_BURN,
							  M0_PROBA_GRASS_BURN,
							  M0_PROBA_STATE_CHANGE);
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);
	} else if (grid->model == 1) {
		// MODEL 1 -> 8 neighbours (same as model 0 but with diagonal neighbours)
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue
				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				// Apply the rules to the cell
				apply_to_cell(grid, copy, point, get_direct_neighbours(grid, point), M1_C_PROBA_TREE_BURN,
							  M1_C_PROBA_GRASS_BURN,
							  M1_PROBA_STATE_CHANGE);
				apply_to_cell(grid, copy, point, get_diagonal_neighbours(grid, point), M1_D_PROBA_TREE_BURN,
							  M1_D_PROBA_GRASS_BURN,
							  M1_PROBA_STATE_CHANGE);
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);
	} else {
		// Unknown model :(
		free(copy);
	}
}

/**
 * Destroy a grid
 *
 * @param grid The grid to destroy
 */
void destroy_grid(Grid grid) {
	// Free the data of the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		free(grid.data[i]);
	}

	free(grid.data);
}