#include <time.h>
#include "grid.c"

/**
 * Main function of the program
 * <p>
 * The program can be launched with the following arguments:
 * <ul>
 * <li>--model [model]: The model of the grid (0 or 1)</li>
 * <li>--count [count]: The number of grids to simulate</li>
 * <li>--iterations [iterations]: The max number of iterations</li>
 * <li>--enable_graphics [0/1]: Whether graphics are disabled</li>
 * <li>--tick [ms]: The number of milliseconds between each tick</li>
 * <li>--export_csv: Export grids in csv format</li>
 * <li>--export_png: Export grids in png format</li>
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
				printf("Usage: %s --model [model] --count [count] --iterations [iterations] --enable_graphics [0/1] --tick [ms] --help\n\nArguments:\n--model [model]: The model of the grid (0 or 1)\n--count [count]: The number of grids to simulate\n--iterations [iterations]: The max number of iterations\n--enable_graphics [0/1]: Whether graphics are disabled\n--tick [ms]: The number of milliseconds between each tick\n--help: Display this help message\n",
					   argv[0]);
				return 0;
			} else if (strcmp(argv[i], "--export_csv") == 0) {
				export_csv = true;
			} else if (strcmp(argv[i], "--export_png") == 0) {
				export_png = true;
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
	} while (remaining > 0 && --iterations != 0);

	wait(2500);

	// DO SOMETHING WITH GRIDS IF NEEDED

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
