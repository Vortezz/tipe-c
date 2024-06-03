#include "draw.c"
#include <cjson/cJSON.h>
#include <unistd.h>

/**
 * Model 0 constants
 *
 * The following probabilities are 1 / [number]
 */
const int M0_PROBA_TREE_BURN = 8;
const int M0_PROBA_GRASS_BURN = 8;
const int M0_PROBA_STATE_CHANGE = 16;

Grid create_grid(int model, Window window, int coord_x, int coord_y) {
	Grid grid = {
			.data = (TileType **) malloc(GRID_SIZE * sizeof(*grid.data)),
			.window = window,
			.model = model,
			.ended = false,
			.coord_x = coord_x,
			.coord_y = coord_y
	};

	for (int i = 0; i < GRID_SIZE; i++) {
		grid.data[i] = (TileType *) malloc(GRID_SIZE * sizeof(*grid.data[i]));
	}

	if (access("grid.json", F_OK) == 0) {
		// The json file exists
		printf("Reading grid.json\n");

		cJSON * grid_json = cJSON_Parse(readfile(fopen("grid.json", "r")));
		cJSON * grid_json_object = cJSON_GetObjectItem(grid_json, "grid");

		for (int i = 0; i < GRID_SIZE; i++) {
			cJSON * row = cJSON_GetArrayItem(grid_json_object, i);
			for (int j = 0; j < GRID_SIZE; j++) {
				grid.data[i][j] = cJSON_GetArrayItem(row, j)->valueint;
			}
		}

		printf("grid.json read\n");
	} else {
		printf("Creating random grid\n");

		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				// Just for OwO purposes
				grid.data[i][j] = rand() % 4;
			}
		}

		printf("Random grid created\n");
	}

	return grid;
}

TileType ** copy_grid(TileType ** data) {
	TileType ** copy = (TileType **) malloc(GRID_SIZE * sizeof(*copy));

	for (int i = 0; i < GRID_SIZE; i++) {
		copy[i] = (TileType *) malloc(GRID_SIZE * sizeof(*copy[i]));
		memcpy(copy[i], data[i], GRID_SIZE * sizeof(*copy[i]));
	}

	return copy;
}

TileType get_tile(Grid grid, Point point) {
	return grid.data[point.x][point.y];
}

Point * get_direct_neighbours(Point point) {
	Point * neighbours = (Point *) malloc(4 * sizeof(*neighbours));

	neighbours[0] = (Point) {point.x - 1, point.y};
	neighbours[1] = (Point) {point.x + 1, point.y};
	neighbours[2] = (Point) {point.x, point.y - 1};
	neighbours[3] = (Point) {point.x, point.y + 1};

	return neighbours;
}

bool is_valid(Point point) {
	return point.x >= 0 && point.x < GRID_SIZE && point.y >= 0 && point.y < GRID_SIZE;
}

bool is_ended(Grid grid) {
	if (grid.model == 0) {
		// TODO :O
		bool is_fire = false;

		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				if (grid.data[i][j] == NEW_FIRE || grid.data[i][j] == OLD_FIRE) {
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

void tick(Grid * grid) {
	TileType ** copy = copy_grid(grid->data);

	if (grid->model == 0) {
		// MODEL 0
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				if (get_tile(*grid, point) != NEW_FIRE && get_tile(*grid, point) != OLD_FIRE) {
					continue;
				}

				Point * neighbours = get_direct_neighbours(point);

				for (int k = 0; k < 4; k++) {
					if (is_valid(neighbours[k])) {
						int random = rand();

						if (get_tile(*grid, neighbours[k]) == TREE && random % M0_PROBA_TREE_BURN == 0 ||
							get_tile(*grid, neighbours[k]) == GRASS && random % M0_PROBA_GRASS_BURN == 0) {
							copy[neighbours[k].x][neighbours[k].y] = NEW_FIRE;
						}
					}
				}

				free(neighbours);

				if (rand() % M0_PROBA_STATE_CHANGE == 0) {
					if (get_tile(*grid, point) == NEW_FIRE) {
						copy[point.x][point.y] = OLD_FIRE;
					} else {
						copy[point.x][point.y] = BURNT;
					}
				}
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

void destroy_grid(Grid grid) {
	for (int i = 0; i < GRID_SIZE; i++) {
		free(grid.data[i]);
	}

	free(grid.data);
}