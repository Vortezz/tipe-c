#include "grid.c"

int main(void) {
	Grid grid = create_grid(0);

	do {
		tick(grid);
		wait(500);
	} while (!is_ended(grid));

	wait(2500);

	destroy_grid(grid);
	return 0;
}
