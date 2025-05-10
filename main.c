#include <time.h>
#include "grid.c"

/**
 * Main function of the program
 * <p>
 * The program can be launched with the following arguments:
 * <ul>
 * <li>--model [model]: The model of the grid (0-2)</li>
 * <li>--count [count]: The number of grids to simulate</li>
 * <li>--iterations [iterations]: The max number of iterations</li>
 * <li>--enable_graphics [0/1]: Whether graphics are disabled</li>
 * <li>--tick [ms]: The number of milliseconds between each tick</li>
 * <li>--export_csv: Export grids in csv format</li>
 * <li>--export_png: Export grids in png format</li>
 * <li>--wind_direction [direction]: The wind direction (0 to 360)</li>
 * <li>--wind_speed [speed]: The wind speed</li>
 * <li>--generate_mean: Generate the mean of the grids (useful only if you export the grids)</li>
 * <li>--help: Display the help message</li>
 * </ul>
 * </p>
 * @param argc The number of arguments
 * @param argv The arguments
 * @return The exit code
 */
int main(int argc, char * argv[]) {
	// Command arguments management
	int model = 0;
	int count = 1;
	int iterations = -1;
	int tick_ms = 10;
	bool enable_graphics = true;
	bool export_csv = false;
	bool export_png = false;
	double wind_direction = 0;
	double wind_speed = 0;
	bool generate_mean = false;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--model") == 0) {
				if (i + 1 < argc) {
					model = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--count") == 0) {
				if (i + 1 < argc) {
					count = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--iterations") == 0) {
				if (i + 1 < argc) {
					iterations = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--tick") == 0) {
				if (i + 1 < argc) {
					tick_ms = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--enable_graphics") == 0) {
				if (i + 1 < argc) {
					enable_graphics = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--help") == 0) {
				printf("Usage: %s --model [model] --count [count] --iterations [iterations] --enable_graphics [0/1] --tick [ms] --export_png --export_csv --wind_direction [direction] --wind_speed [speed] --generate_mean --help\n\nArguments:\n--model [model]: The model of the grid (0-2)\n--count [count]: The number of grids to simulate\n--iterations [iterations]: The max number of iterations\n--enable_graphics [0/1]: Whether graphics are disabled\n--tick [ms]: The number of milliseconds between each tick\n--help: Display this help message\n--export_csv: Export grids in csv format\n--export_png: Export grids in png format\n--wind_direction [direction]: The wind direction (0 to 360)\n--wind_speed [speed]: The wind speed\n--generate_mean: Generate the mean of the grids (useful only if you export the grids)\n--help: Display the help message\n",
					   argv[0]);
				return 0;
			} else if (strcmp(argv[i], "--export_csv") == 0) {
				export_csv = true;
			} else if (strcmp(argv[i], "--export_png") == 0) {
				export_png = true;
			} else if (strcmp(argv[i], "--wind_direction") == 0) {
				if (i + 1 < argc) {
					wind_direction = atof(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--wind_speed") == 0) {
				if (i + 1 < argc) {
					wind_speed = atof(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--generate_mean") == 0) {
				generate_mean = true;
			}
		}
	}

	printf("Launching simulation\nModel %d\nCount %d\nIterations %d\nGraphics %d\n", model, count, iterations,
		   enable_graphics);

	srandom(time(NULL));

	if (tick_ms < 2) {
		printf("Invalid tick, setting to 2\n");
		tick_ms = 2;
	}

	if (count <= 0) {
		printf("Invalid count, setting to 1\n");
		count = 1;
	}

	int remaining = count;
	Grid * grids = malloc(count * sizeof(*grids));

	// Choose the number of grids to display per line and per column
	int max_x;
	int max_y;
	if (count == 1) {
		max_x = 1;
		max_y = 1;
	} else if (count <= 2) {
		max_x = 2;
		max_y = 1;
	} else if (count <= 6) {
		max_x = 3;
		max_y = 2;
	} else if (count <= 18) {
		max_x = 6;
		max_y = 3;
	} else if (count <= 36) {
		max_x = 9;
		max_y = 4;
	} else {
		max_x = 14;
		max_y = 7;
	}

	remove("grids.csv");
	remove("grids_png");

	// Create the window and the grids
	Window window;
	if (enable_graphics) {
		window = create_window(max_x, max_y);
	} else {
		window = (Window) {
				.window = NULL,
				.surface = NULL
		};
	}

	for (int i = 0; i < count; i++) {
		grids[i] = create_grid(model, window, i % max_x, i / max_x, export_csv, export_png);
		grids[i].wind_direction = wind_direction;
		grids[i].wind_speed = wind_speed;
	}

	// Main loop to update the grids and tick until all grids have ended
	do {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Used to close the window if the user clicks on the close button
			if (event.type == SDL_QUIT) {
				for (int i = 0; i < count; i++) {
					destroy_grid(grids[i]);
				}

				free(grids);
				return 0;
			}
		}

		// Update the grids
		for (int i = 0; i < count; i++) {
			if (!grids[i].ended) {
				tick(&grids[i]);

				grids[i].ended = is_ended(grids[i]);
				// If the grid has ended, we decrease the number of remaining grids
				if (grids[i].ended) {
					remaining--;
				}
			}
		}

		wait(tick_ms);
	} while (remaining > 0 && --iterations != -1);

	wait(2500);

	// DO SOMETHING WITH GRIDS IF NEEDED
	if (generate_mean) {
		int *** data = malloc(GRID_SIZE * sizeof(*data));
		for (int i = 0; i < GRID_SIZE; i++) {
			data[i] = (int **) malloc(GRID_SIZE * sizeof(*data[i]));
			for (int j = 0; j < GRID_SIZE; j++) {
				data[i][j] = malloc(TILE_TYPE_SIZE * sizeof(*data[i][j]));
			}
		}

		for (int i = 0; i < count; i++) {
			for (int x = 0; x < GRID_SIZE; x++) {
				for (int y = 0; y < GRID_SIZE; y++) {
					data[x][y][grids[i].data[x][y].current_type]++;
				}
			}
		}

		Tile ** tiles = malloc(GRID_SIZE * sizeof(*tiles));

		for (int x = 0; x < GRID_SIZE; x++) {
			tiles[x] = malloc(GRID_SIZE * sizeof(*tiles[x]));
			for (int y = 0; y < GRID_SIZE; y++) {
				tiles[x][y] = (Tile) {
						.default_type = TILE_TYPE_SIZE,
						.current_type = TILE_TYPE_SIZE,
						.state = 0
				};

				int max = 0;
				int max_type = 0;
				for (int i = 0; i < TILE_TYPE_SIZE; i++) {
					if (data[x][y][i] > max) {
						max = data[x][y][i];
						max_type = i;
					}
				}

				tiles[x][y].current_type = max_type;
			}
		}

		Grid grid = (Grid) {
				.data = tiles,
				.window = window,
				.model = model,
				.ended = true,
				.coord_x = -1,
				.coord_y = -1,
				.export_png = export_png,
				.export_csv = export_csv,
				.wind_direction = wind_direction,
				.wind_speed = wind_speed
		};

		destroy_grid(grid);

		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				free(data[i][j]);
			}
			free(data[i]);
		}
		free(data);
	}

	// Free the memory and close the window
	for (int i = 0; i < count; i++) {
		destroy_grid(grids[i]);
	}

	if (enable_graphics) {
		destroy_window(window);
	}
	free(grids);

	return 0;
}
