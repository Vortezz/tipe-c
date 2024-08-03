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

const int M1_C_PROBA_TREE_BURN = 8;
const int M1_C_PROBA_GRASS_BURN = 8;
const int M1_D_PROBA_TREE_BURN = 4;
const int M1_D_PROBA_GRASS_BURN = 4;
const int M1_PROBA_STATE_CHANGE = 16;

Grid create_grid(int model, Window window, int coord_x, int coord_y) {
	Grid grid = {
			.data = (Tile **) malloc(GRID_SIZE * sizeof(*grid.data)),
			.window = window,
			.model = model,
			.ended = false,
			.coord_x = coord_x,
			.coord_y = coord_y};

	for (int i = 0; i < GRID_SIZE; i++) {
		grid.data[i] = (Tile *) malloc(GRID_SIZE * sizeof(*grid.data[i]));
	}

	if (access("grid.json", F_OK) == 0) {
		// The json file exists
		cJSON * grid_json = cJSON_Parse(readfile(fopen("grid.json", "r")));
		cJSON * grid_json_object = cJSON_GetObjectItem(grid_json, "grid");

		for (int i = 0; i < GRID_SIZE; i++) {
			cJSON * row = cJSON_GetArrayItem(grid_json_object, i);
			for (int j = 0; j < GRID_SIZE; j++) {
				int value = cJSON_GetArrayItem(row, j)->valueint;
				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
			}
		}
	} else {
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				int value = get_random(4);
				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
			}
		}
	}

	return grid;
}

Tile ** copy_grid(Tile ** data) {
	Tile ** copy = (Tile **) malloc(GRID_SIZE * sizeof(*copy));

	for (int i = 0; i < GRID_SIZE; i++) {
		copy[i] = (Tile *) malloc(GRID_SIZE * sizeof(*copy[i]));
		memcpy(copy[i], data[i], GRID_SIZE * sizeof(*copy[i]));
	}

	return copy;
}

Tile get_tile(Grid grid, Point point) {
	return grid.data[point.x][point.y];
}

Point * get_direct_neighbours(Grid * grid, Point point) {
	if (grid->model == 0) {
		Point * neighbours = (Point *) malloc(4 * sizeof(*neighbours));

		neighbours[0] = (Point) {point.x - 1, point.y};
		neighbours[1] = (Point) {point.x + 1, point.y};
		neighbours[2] = (Point) {point.x, point.y - 1};
		neighbours[3] = (Point) {point.x, point.y + 1};

		return neighbours;
	} else if (grid->model == 1) {
		Point * neighbours = (Point *) malloc(8 * sizeof(*neighbours));
		// côtés
		neighbours[0] = (Point) {point.x - 1, point.y};
		neighbours[1] = (Point) {point.x + 1, point.y};
		neighbours[2] = (Point) {point.x, point.y - 1};
		neighbours[3] = (Point) {point.x, point.y + 1};
		// diag
		neighbours[4] = (Point) {point.x + 1, point.y + 1};
		neighbours[5] = (Point) {point.x + 1, point.y - 1};
		neighbours[6] = (Point) {point.x - 1, point.y + 1};
		neighbours[7] = (Point) {point.x - 1, point.y - 1};

		return neighbours;
	} else {
		// Unknown model :(
		return NULL;
	}
}

bool is_valid(Point point) {
	return point.x >= 0 && point.x < GRID_SIZE && point.y >= 0 && point.y < GRID_SIZE;
}

bool is_ended(Grid grid) {
	if (grid.model == 0 || grid.model == 1) {
		// TODO :O
		bool is_fire = false;

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

void tick(Grid * grid) {
	Tile ** copy = copy_grid(grid->data);

	if (grid->model == 0) {
		// MODEL 0 -> 4 neighbors
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				Point * neighbours = get_direct_neighbours(grid, point);

				for (int k = 0; k < 4; k++) {
					if (is_valid(neighbours[k])) {
						if (get_tile(*grid, neighbours[k]).current_type == TREE &&
							get_random(M0_PROBA_TREE_BURN) == 0 ||
							get_tile(*grid, neighbours[k]).current_type == GRASS &&
							get_random(M0_PROBA_GRASS_BURN) == 0) {
							copy[neighbours[k].x][neighbours[k].y].current_type = FIRE;
							copy[neighbours[k].x][neighbours[k].y].state = 0;
						}
					}
				}

				free(neighbours);

				if (get_tile(*grid, point).current_type == FIRE && get_random(M0_PROBA_STATE_CHANGE) == 0) {
					if (get_tile(*grid, point).state == 0) {
						copy[point.x][point.y].state++;
					} else {
						copy[point.x][point.y].current_type = BURNT;
						copy[point.x][point.y].state = 0;
					}
				}
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);
	} else if (grid->model == 1) {
		// MODEL 1 -> 8 neighbors (côtés+diag)
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				Point * neighbours = get_direct_neighbours(grid, point);

				for (int k = 0; k < 4; k++) { // côtés
					if (is_valid(neighbours[k])) {
						if (get_tile(*grid, neighbours[k]).current_type == TREE &&
							get_random(M1_C_PROBA_TREE_BURN) == 0 ||
							get_tile(*grid, neighbours[k]).current_type == GRASS &&
							get_random(M1_C_PROBA_GRASS_BURN) == 0) {
							copy[neighbours[k].x][neighbours[k].y].current_type = FIRE;
							copy[neighbours[k].x][neighbours[k].y].state = 0;
						}
					}
				}

				for (int k = 4; k < 8; k++) { // diag
					if (is_valid(neighbours[k])) {
						if (get_tile(*grid, neighbours[k]).current_type == TREE &&
							get_random(M1_D_PROBA_TREE_BURN) == 0 ||
							get_tile(*grid, neighbours[k]).current_type == GRASS &&
							get_random(M1_D_PROBA_GRASS_BURN) == 0) {
							copy[neighbours[k].x][neighbours[k].y].current_type = FIRE;
							copy[neighbours[k].x][neighbours[k].y].state = 0;
						}
					}
				}

				free(neighbours);

				if (get_tile(*grid, point).current_type == FIRE && get_random(M0_PROBA_STATE_CHANGE) == 0) {
					if (get_tile(*grid, point).state == 0) {
						copy[point.x][point.y].state++;
					} else {
						copy[point.x][point.y].current_type = BURNT;
						copy[point.x][point.y].state = 0;
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