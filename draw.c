#include <stdbool.h>
#include <unistd.h>
#include "typings.c"
#include "misc.c"

/**
 * Draw a pixel on the window
 *
 * @param window The window to draw on
 * @param point The point to draw
 * @param color The color of the pixel
 * @param update Whether to update the window (ie to display the pixel)
 */
void draw_pixel(Window window, Point point, Color color, bool update) {
	// If the point is outside the window, do nothing
	if (point.x < 0 || point.x >= window.surface->w || point.y < 0 || point.y >= window.surface->h) {
		return;
	}

	// Get the pixel at the point and set its color
	Uint32 * pixel = (Uint32 *) window.surface->pixels + point.y * window.surface->pitch / 4 + point.x;
	*pixel = SDL_MapRGB(window.surface->format, color.r, color.g, color.b);

	// If we want to update the window, we update it
	if (update) {
		SDL_UpdateWindowSurface(window.window);
	}
}

/**
 * Draw a square on the window
 *
 * @param window The window to draw on
 * @param point The top-left corner of the square
 * @param size The size of the square
 * @param color The color of the square
 * @param update Whether to update the window (ie to display the square)
 */
void draw_square(Window window, Point point, int size, Color color, bool update) {
	// Draw a square of pixels
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			// Draw the pixel
			draw_pixel(window, (Point) {point.x + i, point.y + j}, color, false);
		}
	}

	// If we want to update the window, we update it
	if (update) {
		SDL_UpdateWindowSurface(window.window);
	}
}

/**
 * Draw the grid on the window
 *
 * @param window The window to draw on
 * @param grid The grid to draw
 */
void draw_grid(Window window, Grid grid) {
	// Graphics not enabled
	if (!window.window) {
		return;
	}

	// Draw the grid using the constants defined in typings.c, and translate the grid to the right position
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			Tile tile = grid.data[i][j];

			// Draw the tile as a square
			draw_square(window, (Point) {TILE_SIZE * (i + (GRID_SIZE + 1) * grid.coord_x),
										 TILE_SIZE * (j + (GRID_SIZE + 1) * grid.coord_y)}, TILE_SIZE,
						get_color(tile.current_type, tile.state), false);
		}
	}

	// Update the window to display the grid
	SDL_UpdateWindowSurface(window.window);
}

/**
 * Create a window
 *
 * @param max_x The maximum number of grids on the x axis
 * @param max_y The maximum number of grids on the y axis
 * @return The window
 */
Window create_window(int max_x, int max_y) {
	// The window and the surface of the window
	Window window = {
			.window = NULL,
			.surface = NULL
	};

	// Define the size of the tiles based on the number of grids (to ensure that the window is not too big)
	TILE_SIZE = TILE_SIZE - (1.5) * min(max_y - 1, 4);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// SDL initialization failed
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	} else {
		// Create the window
		window.window = SDL_CreateWindow(
				"TIPE",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				(max_x * (GRID_SIZE + 1) - 1) * TILE_SIZE,
				(max_y * (GRID_SIZE + 1) - 1) * TILE_SIZE,
				SDL_WINDOW_SHOWN
		);

		if (window.window == NULL) {
			// Window creation failed
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			exit(1);
		} else {
			window.surface = SDL_GetWindowSurface(window.window);
		}
	}

	return window;
}

/**
 * Wait for a certain amount of time
 *
 * @param ms The number of milliseconds to wait
 */
void wait(int ms) {
	usleep(ms * 1000);
}

/**
 * Destroy a window
 *
 * @param window The window to destroy
 */
void destroy_window(Window window) {
	// Destroy the window and quit SDL
	SDL_DestroyWindow(window.window);
	SDL_Quit();
}