#include "grid.c"

int main(void) {
	Grid grid = create_grid();

	tick(grid);
	wait(2000);

	destroy_grid(grid);
	return 0;
}
