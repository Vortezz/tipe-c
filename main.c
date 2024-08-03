#include <time.h>
#include "grid.c"

int main(int argc, char * argv[]) {
	// Command syntax main --model [model] --count [count] --help --tick [ms]
	int model = 0;
	int count = 1;
	int tick_ms = 10;

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
			} else if (strcmp(argv[i], "--tick") == 0) {
				if (i + 1 < argc) {
					tick_ms = atoi(argv[i + 1]);
				}
			} else if (strcmp(argv[i], "--help") == 0) {
				printf("Usage: %s --model [model] --count [count] --tick [ms] --help\n", argv[0]);
				return 0;
			}
		}
	}

	printf("Launching simulation with model %d and count %d\n", model, count);

	srandom(time(NULL));

	int remaining = count;
	Grid * grids = malloc(count * sizeof(*grids));

	if (tick_ms < 2) {
		printf("Invalid tick, setting to 2\n");
		tick_ms = 2;
	}

	if (count <= 0) {
		printf("Invalid count, setting to 1\n");
		count = 1;
	}

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

	Window window = create_window(max_x, max_y);

	for (int i = 0; i < count; i++) {
		grids[i] = create_grid(model, window, i % max_x, i / max_x);
	}

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

		for (int i = 0; i < count; i++) {
			if (!grids[i].ended) {
				tick(&grids[i]);

				grids[i].ended = is_ended(grids[i]);
				if (grids[i].ended) {
					remaining--;
				}
			}
		}

		wait(tick_ms);
	} while (remaining > 0);

	wait(2500);

	for (int i = 0; i < count; i++) {
		destroy_grid(grids[i]);
	}

	destroy_window(window);
	free(grids);

	return 0;
}
