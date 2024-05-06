#include <time.h>
#include "grid.c"

int main(void) {
	srand(time(NULL));
	Grid grid = create_grid(0);

	do {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Used to close the window if the user clicks on the close button
			if (event.type == SDL_QUIT) {
				destroy_grid(grid);
				return 0;
			}
		}

		tick(&grid);
		wait(25);
	} while (!is_ended(grid));

	wait(2500);

	destroy_grid(grid);
	return 0;
}
