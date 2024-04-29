#include <stdbool.h>
#include "typings.c"
#include "misc.c"

void draw_pixel(Window window, Point point, Color color, bool update) {
	if (point.x < 0 || point.x >= window.surface->w || point.y < 0 || point.y >= window.surface->h) {
		return;
	}

	Uint32 * pixel = (Uint32 *) window.surface->pixels + point.y * window.surface->pitch / 4 + point.x;
	*pixel = SDL_MapRGB(window.surface->format, color.r, color.g, color.b);

	if (update) {
		SDL_UpdateWindowSurface(window.window);
	}
}

void draw_square(Window window, Point point, int size, Color color, bool update) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			draw_pixel(window, (Point) {point.x + i, point.y + j}, color, false);
		}
	}

	if (update) {
		SDL_UpdateWindowSurface(window.window);
	}
}

void draw_grid(Window window, Grid grid) {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			draw_square(window, (Point) {i * TILE_SIZE, j * TILE_SIZE}, TILE_SIZE, get_color(grid.data[i][j]), false);
		}
	}

	SDL_UpdateWindowSurface(window.window);
}

Window create_window() {
	Window window = {
			.window = NULL,
			.surface = NULL
	};

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	} else {
		window.window = SDL_CreateWindow(
				"TIPE :O",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				GRID_SIZE * TILE_SIZE,
				GRID_SIZE * TILE_SIZE,
				SDL_WINDOW_SHOWN
		);

		if (window.window == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		} else {
			window.surface = SDL_GetWindowSurface(window.window);
		}
	}

	return window;
}

void wait(int ms) {
	SDL_Delay(ms);
}

void destroy_window(Window window) {
	SDL_DestroyWindow(window.window);
	SDL_Quit();
}