#include "draw.c"
#include <cjson/cJSON.h>
#include <unistd.h>

Grid create_grid(int model) {
	Window window = create_window();

	Grid grid = {
			.data = (TileType **) malloc(GRID_SIZE * sizeof(*grid.data)),
			.window = window,
			.model = model
	};

	for (int i = 0; i < GRID_SIZE; i++) {
		grid.data[i] = (TileType *) malloc(GRID_SIZE * sizeof(*grid.data[i]));
	}

	if (access("grid.json", F_OK) == 0) {
		// The json file exists
		cJSON * grid_json = cJSON_Parse(readfile(fopen("grid.json", "r")));
		cJSON * grid_json_object = cJSON_GetObjectItem(grid_json, "grid");

		for (int i = 0; i < GRID_SIZE; i++) {
			cJSON * row = cJSON_GetArrayItem(grid_json_object, i);
			for (int j = 0; j < GRID_SIZE; j++) {
				grid.data[i][j] = cJSON_GetArrayItem(row, j)->valueint;
			}
		}
	} else {
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				// Just for OwO purposes
				grid.data[i][j] = rand() % 3;
			}
		}
	}

	return grid;
}

bool is_ended(Grid grid) {
	return true;
}

void tick(Grid grid) {
	draw_grid(grid.window, grid);
	// TODO :O
}

void destroy_grid(Grid grid) {
	for (int i = 0; i < GRID_SIZE; i++) {
		free(grid.data[i]);
	}

	free(grid.data);

	destroy_window(grid.window);
}