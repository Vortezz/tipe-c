#include "draw.c"
#include <cjson/cJSON.h>
#include <unistd.h>
#include <png.h>
#include <sys/stat.h>
#include <math.h>

/**
 * Model 0 constants
 *
 * The following probabilities are 1 / [number]
 */
/**
 * The probability for a tree tile to burn
 */
const int M0_PROBA_TREE_BURN = 8;
/**
 * The probability for a grass tile to burn
 */
const int M0_PROBA_GRASS_BURN = 8;
/**
 * The probability for a tile to change state between fire and burnt
 */
const int M0_PROBA_STATE_CHANGE = 16;

/**
 * Model 1 constants
 *
 * The following probabilities are 1 / [number]
 */
/**
 * The probability for a tree tile to burn (in direct neighbors)
 */
const int M1_C_PROBA_TREE_BURN = 8;
/**
 * The probability for a grass tile to burn (in direct neighbors)
 */
const int M1_C_PROBA_GRASS_BURN = 8;
/**
 * The probability for a tree tile to burn (in diagonal neighbors)
 */
const int M1_D_PROBA_TREE_BURN = 16;
/**
 * The probability for a grass tile to burn (in diagonal neighbors)
 */
const int M1_D_PROBA_GRASS_BURN = 16;
/**
 * The probability for a tile to change state between fire and burnt
 */
const int M1_PROBA_STATE_CHANGE = 16;

const double M3_PROBA_V_BURN = 1./8.;
const double M3_PROBA_STATE_CHANGE = 1./16.;
/**
 * Typical constants for mixed forest + medium/coarse timber
 */
const double B = 0.46;
const double C_WIND = 2.93*pow(1.14, -0.5);
const double C_SLOPE = 5.275*pow(0.08, -0.3);


void write_to_file(Grid grid);
Tile ** copy_grid(Tile ** data);
Point * get_direct_neighbors(Grid * grid, Point point);
Point * get_diagonal_neighbors(Grid * grid, Point point);
bool is_valid(Point point);
void write_png(Grid grid);

/**
 * Create a grid
 *
 * @param model The model of the grid
 * @param window The window to draw the grid
 * @param coord_x The x coordinate of the grid
 * @param coord_y The y coordinate of the grid
 * @return The created grid
 */
Grid create_grid(int model, Window window, int coord_x, int coord_y, bool export_csv, bool export_png) {
	// Create the grid
	Grid grid = {
			.data = (Tile **) malloc(GRID_SIZE * sizeof(*grid.data)),
			.window = window,
			.model = model,
			.ended = false,
			.coord_x = coord_x,
			.coord_y = coord_y,
			.export_csv = export_csv,
			.export_png = export_png,
			.n_intervals = 0
	};

	// Initialize the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		grid.data[i] = (Tile *) malloc(GRID_SIZE * sizeof(*grid.data[i]));
	}

	// Load the grid from a json file if it exists, otherwise create a random grid
	if (access("grid.json", F_OK) == 0) {
		// The json file exists, we load the grid from it
		cJSON * grid_json = cJSON_Parse(readfile(fopen("grid.json", "r")));
		cJSON * grid_json_object = cJSON_GetObjectItem(grid_json, "grid");

		for (int i = 0; i < GRID_SIZE; i++) {
			cJSON * row = cJSON_GetArrayItem(grid_json_object, i);
			for (int j = 0; j < GRID_SIZE; j++) {
				// Get the value of the tile and set it to the grid
				int value = cJSON_GetArrayItem(row, j)->valueint;

				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
				grid.data[i][j].altitude = 0;
			}
		}
	} else {
		// The json file does not exist, we create a random grid
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				// Get a random value between 0 and 3 and set it to the grid
				int value = get_random(4);

				grid.data[i][j].current_type = value;
				grid.data[i][j].default_type = value;
				grid.data[i][j].state = 0;
				grid.data[i][j].altitude = 0;
			}
		}

		// The automaton iterates over the random grid
		for (int k = 0; k<6; ++k){
			//write_png(grid);
			//++grid.n_intervals;
			for (int l = 0; l<5; ++l){
			
			Tile ** copy = copy_grid(grid.data);
			for (int i = 0; i < GRID_SIZE; i++) {
				for (int j = 0; j < GRID_SIZE; j++) {
					Point point = (Point) {i, j};
					int occ[TILE_TYPE_SIZE] = {0};
					++occ[grid.data[i][j].current_type];
					Point* n = get_direct_neighbors(&grid, point);
					Point* diagn = get_diagonal_neighbors(&grid, point);
					for (int l = 0; l<4; ++l){
						if (is_valid(n[l])){
							TileType type1 = grid.data[n[l].x][n[l].y].current_type;
							++occ[type1];
						}
						if (is_valid(diagn[l])){
							TileType type2 = grid.data[diagn[l].x][diagn[l].y].current_type;
							++occ[type2];
						}
					}
					free(n);
					free(diagn);

					if (occ[WATER] > occ[GRASS] && occ[WATER] > occ[TREE]){
						copy[i][j].current_type = WATER;
						copy[i][j].default_type = WATER;
					} else if (occ[GRASS]>occ[TREE]){
						copy[i][j].current_type = GRASS;
						copy[i][j].default_type = GRASS;
					} else {
						copy[i][j].current_type = TREE;
						copy[i][j].default_type = TREE;
					}

				}
			}
			for (int i = 0; i < GRID_SIZE; ++i) {
				free(grid.data[i]);
			}
			free(grid.data);
			grid.data = copy;
			} 
		}

		
		grid.data[GRID_SIZE/6][GRID_SIZE/2].current_type = FIRE;
		grid.data[GRID_SIZE/6][GRID_SIZE/2].default_type = FIRE;
	}

	return grid;
};

/**
 * Copy a grid
 *
 * @param data The grid to copy
 * @return The copied grid
 */
Tile ** copy_grid(Tile ** data) {
	// Malloc the copy of the grid
	Tile ** copy = (Tile **) malloc(GRID_SIZE * sizeof(*copy));

	// Fill the copy with the data of the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		copy[i] = (Tile *) malloc(GRID_SIZE * sizeof(*copy[i]));
		memcpy(copy[i], data[i], GRID_SIZE * sizeof(*copy[i]));
	}

	return copy;
}

/**
 * Get the tile at a point
 *
 * @param grid The grid
 * @param point The point
 * @return The tile at the point
 */
Tile get_tile(Grid grid, Point point) {
	return grid.data[point.x][point.y];
}

/**
 * Get the direct neighbors of a point
 *
 * @param grid The grid
 * @param point The point
 * @return The neighbors of the point
 */
Point * get_direct_neighbors(Grid * grid, Point point) {
	Point * neighbors = (Point *) malloc(4 * sizeof(*neighbors));

	neighbors[0] = (Point) {point.x - 1, point.y};
	neighbors[1] = (Point) {point.x + 1, point.y};
	neighbors[2] = (Point) {point.x, point.y - 1};
	neighbors[3] = (Point) {point.x, point.y + 1};

	return neighbors;
}

/**
 * Get the diagonal neighbors of a point
 *
 * @param grid The grid
 * @param point The point
 * @return The neighbors of the point
 */
Point * get_diagonal_neighbors(Grid * grid, Point point) {
	Point * neighbors = (Point *) malloc(4 * sizeof(*neighbors));

	neighbors[0] = (Point) {point.x - 1, point.y - 1};
	neighbors[1] = (Point) {point.x + 1, point.y - 1};
	neighbors[2] = (Point) {point.x - 1, point.y + 1};
	neighbors[3] = (Point) {point.x + 1, point.y + 1};

	return neighbors;
}

/**
 * Check if a point is valid (ie inside the grid)
 *
 * @param point The point to check
 * @return True if the point is valid, false otherwise
 */
bool is_valid(Point point) {
	return point.x >= 0 && point.x < GRID_SIZE && point.y >= 0 && point.y < GRID_SIZE;
}

/**
 * Check if the grid is ended
 *
 * @param grid The grid to check
 * @return True if the grid is ended, false otherwise
 */
bool is_ended(Grid grid) {
	if (grid.model == 0 || grid.model == 1 || grid.model == 2 || grid.model == 3) {
		bool is_fire = false;

		// Check if there is no more fire, if there is no more fire, the grid is ended
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

/**
 * Check a probability : if the tile is of the given type and the probability is valid
 *
 * @param grid The grid
 * @param point The point to check
 * @param type The type to check
 * @param proba The probability
 * @return True if the probability is valid, false otherwise
 */
bool check_probability(Grid * grid, Point point, TileType type, int proba) {
	return get_tile(*grid, point).current_type == type && get_random(proba) == 0;
}

bool check_probability_3(Grid * grid, Point point, TileType type, double proba) {
	return get_tile(*grid, point).current_type == type && get_random_3() < proba;
}

/**
 * Apply the rules to a cell (model 0 and 1)
 *
 * @param grid The grid
 * @param copy The copy of the grid
 * @param point The point to apply the rules to
 * @param neighbors The neighbors of the point
 * @param tree_burn The probability for a tree tile to burn
 * @param grass_burn The probability for a grass tile to burn
 * @param state_change The probability for a tile to change state between fire and burnt
 */
void apply_to_cell(Grid * grid, Tile ** copy, Point point, Point * neighbors, int tree_burn, int grass_burn,
				   int state_change) {
	// First step, change the state of the neighbors based on the probability
	for (int k = 0; k < 4; k++) {
		if (is_valid(neighbors[k])) {
			if (check_probability(grid, neighbors[k], TREE, tree_burn) ||
				check_probability(grid, neighbors[k], GRASS, grass_burn)) {
				Tile * tile_copy = &copy[neighbors[k].x][neighbors[k].y];

				tile_copy->current_type = FIRE;
				tile_copy->state = 0;
			}
		}
	}

	// Second step, change the state of the point based on the probability to a new state or to burnt
	Tile point_tile = get_tile(*grid, point);
	if (check_probability(grid, point, FIRE, state_change)) {
		Tile * tile_copy = &copy[point.x][point.y];

		// If the tile is newly on fire, we increment the state of the tile, otherwise we set it to burnt
		if (point_tile.state == 0) {
			tile_copy->state++;
		} else {
			tile_copy->current_type = BURNT;
			tile_copy->state = 0;
		}
	}

	free(neighbors);
}


/**
 * Get the burn probability (used for Alexandridis model)
 *
 * @return The burn probability
 */
double get_burn_probability(Tile tile, Point point, Point parent, Grid * grid) {
	double p_v;
	switch (tile.current_type) {
		case TREE:
		case DENSE_TREE:
			p_v = 0.3;
			break;
		case GRASS:
			p_v = -0.1;
			break;
		case TRENCH:
			p_v = -0.55;
			break;
		default:
			p_v = -1;
			break;
	}

	int dx = point.x - parent.x;
	int dy = point.y - parent.y;
	double theta = 0;

	switch (dx) {
		case 0: {
			if (dy == 0) {
				theta = 0; // Should not happen
			} else if (dy == 1) {
				theta = 90;
			} else {
				theta = 270;
			}
			break;
		}
		case 1: {
			if (dy == 0) {
				theta = 0;
			} else if (dy == 1) {
				theta = 45;
			} else {
				theta = 315;
			}
			break;
		}
		case -1: {
			if (dy == 0) {
				theta = 180;
			} else if (dy == 1) {
				theta = 135;
			} else {
				theta = 225;
			}
			break;
		}
	}

	theta = (grid->wind_direction - theta) * M_PI / 180;

	double p_d = 0; // TODO : Implement density
	if (tile.current_type == DENSE_TREE) {
		p_d = 0.3;
	}

	double p_h = 0.34; // TODO : Compute value, best value is 0.58 according to the paper
	double p_w =
			exp(0.045 * grid->wind_speed) * exp(grid->wind_speed * 0.131 * (cos(theta) - 1)); // TODO : Implement wind
	double p_s = exp(0.078 * 0 /* TODO : Add angle for slope */);

	return p_h * (1 + p_v) * (1 + p_d) * p_w * p_s;
}

/**
 * Get the slope between Point point and point v
 */
double get_slope(Point point, Point v, Grid* grid){
	if (!is_valid(v) || !is_valid(point) || v.x == point.x && v.y == point.y) return 0. ;
	double h = get_tile(*grid, v).altitude - get_tile(*grid, point).altitude;
	double dx = v.x - point.x;
	double dy = v.y - point.y;
	return h/(sqrt(dx*dx + dy*dy));
}

/**
 * Get the projected value of the wind vector onto the vector v-point
 */
double get_wind(Point point, Point v, Grid* grid){
	if (!is_valid(v) || !is_valid(point) || v.x == point.x && v.y == point.y) return 0. ;
	// Wind vector components
	double Ux = grid->wind_speed*sin(grid->wind_direction*M_PI/180.);
	double Uy = grid->wind_speed*cos(grid->wind_direction*M_PI/180.);
	// v-point vector components
	double dx = v.x - point.x;
	double dy = v.y - point.y;
	return dx*Ux + dy*Uy;
}

/**
 * Update the grid
 *
 * @param grid The grid to update
 */
void tick(Grid * grid) {
	Tile ** copy = copy_grid(grid->data);

	// Update the grid based on the model
	if (grid->model == 0) {
		// MODEL 0 -> 4 neighbors
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue
				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				// Apply the rules to the cell
				apply_to_cell(grid, copy, point, get_direct_neighbors(grid, point), M0_PROBA_TREE_BURN,
							  M0_PROBA_GRASS_BURN,
							  M0_PROBA_STATE_CHANGE);
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);
	} else if (grid->model == 1) {
		// MODEL 1 -> 8 neighbors (same as model 0 but with diagonal neighbors)
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue
				if (get_tile(*grid, point).current_type != FIRE) {
					continue;
				}

				// Apply the rules to the cell
				apply_to_cell(grid, copy, point, get_direct_neighbors(grid, point), M1_C_PROBA_TREE_BURN,
							  M1_C_PROBA_GRASS_BURN,
							  M1_PROBA_STATE_CHANGE);
				apply_to_cell(grid, copy, point, get_diagonal_neighbors(grid, point), M1_D_PROBA_TREE_BURN,
							  M1_D_PROBA_GRASS_BURN,
							  M1_PROBA_STATE_CHANGE);
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);
	} else if (grid->model == 2) { // Alexandridis
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue to the next tile
				Tile tile = get_tile(*grid, point);
				if (tile.current_type != FIRE) {
					continue;
				}

				Tile * copy_tile = &copy[point.x][point.y];

				if (tile.state == 0) {
					Point * direct_neighbors = get_direct_neighbors(grid, point);
					Point * diagonal_neighbors = get_diagonal_neighbors(grid, point);

					for (int k = 0; k < 4; k++) {
						Point direct_point = direct_neighbors[k];
						if (is_valid(direct_point)) {
							Tile direct_tile = get_tile(*grid, direct_point);
							double p_burn = get_burn_probability(direct_tile, direct_point, point, grid);

							if (get_random(1000000) < p_burn * 1000000) {
								Tile * copy_direct_tile = &copy[direct_point.x][direct_point.y];

								copy_direct_tile->current_type = FIRE;
								copy_direct_tile->state = 0;
							}
						}

						Point diagonal_point = diagonal_neighbors[k];
						if (is_valid(diagonal_point)) {
							Tile diagonal_tile = get_tile(*grid, diagonal_point);
							double p_burn = get_burn_probability(diagonal_tile, diagonal_point, point, grid);

							if (get_random(1000000) < p_burn * 1000000) {
								Tile * copy_diagonal_tile = &copy[diagonal_point.x][diagonal_point.y];

								copy_diagonal_tile->current_type = FIRE;
								copy_diagonal_tile->state = 0;
							}
						}
					}

					free(direct_neighbors);
					free(diagonal_neighbors);

					copy_tile->state = 1;
				} else {
					copy_tile->current_type = BURNT;
					copy_tile->state = 0;
				}
			}
		}

		free(grid->data);

		grid->data = copy;

		draw_grid(grid->window, *grid);

	} else if (grid->model == 3) { // Rothermel
		for (int i = 0; i < GRID_SIZE; i++) {
			for (int j = 0; j < GRID_SIZE; j++) {
				Point point = (Point) {i, j};

				// If the tile is not on fire, we continue to the next tile
				Tile tile = get_tile(*grid, point);
				if (tile.current_type != FIRE) {
					continue;
				}
				Point * neighbors = get_direct_neighbors(grid, point);
				for (int k = 0; k<4; ++k){
					if (is_valid(neighbors[k])) {
						double slope = get_slope(point, neighbors[k], grid);
						double wind = get_wind(point, neighbors[k], grid);
						double phi = signe(slope)*C_SLOPE*slope*slope + signe(wind)*C_WIND*pow(fabs(wind), B);
						double proba;
						if (phi<=-1.){
							proba = M3_PROBA_V_BURN*1./(fabs(phi));
						} else {
							proba = 1.-pow(1-M3_PROBA_V_BURN, 1.+phi);
						}
						//printf("FROM (%d,%d) TO (%d,%d): slope=%.3f wind=%.3f phi=%.3f proba=%.3f\n",point.x, point.y, neighbors[k].x, neighbors[k].y,slope, wind, phi, proba);

						// change the state of the neighbors based on the probability
						
						if (check_probability_3(grid, neighbors[k], TREE, proba) ||
						check_probability_3(grid, neighbors[k], GRASS, proba)) {
						
							Tile * tile_copy = &copy[neighbors[k].x][neighbors[k].y];

							tile_copy->current_type = FIRE;
							tile_copy->state = 0;
						}
					}

				}

				// change the state of the point based on the probability to a new state or to burnt
				 
				if (check_probability_3(grid, point, FIRE, M3_PROBA_STATE_CHANGE)) {
					Tile * tile_copy = &copy[point.x][point.y];

					// If the tile is newly on fire, we increment the state of the tile, otherwise we set it to burnt
					if (tile.state == 0) {
						tile_copy->state++;
					} else {
						tile_copy->current_type = BURNT;
						tile_copy->state = 0;
					}
				}

				free(neighbors);

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

/**
 * Write to png file
 *
 * @param grid The grid to write
 */
void write_png(Grid grid) {
	struct stat st = {0};
	if (stat("grids_png", &st) == -1) {
		mkdir("grids_png", 0700);
	}

	char * file_name = malloc(100 * sizeof(*file_name));
	sprintf(file_name, "grids_png/grid-%d-%d-%d.png", grid.coord_x, grid.coord_y, grid.n_intervals);

	FILE * fp = fopen(file_name, "wb");
	if (!fp) {
		fprintf(stderr, "Failed to open file %s for writing\n", file_name);
		return;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fprintf(stderr, "Failed to create png write struct\n");
		fclose(fp);
		return;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		fprintf(stderr, "Failed to create png info struct\n");
		png_destroy_write_struct(&png, NULL);
		fclose(fp);
		return;
	}

	if (setjmp(png_jmpbuf(png))) { // To handle errors
		printf("Error during png creation\n");
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return;
	}

	png_init_io(png, fp);

	// Write the header (8-bit color depth, RGB format)
	png_set_IHDR(png, info, 512, 512, 8, PNG_COLOR_TYPE_RGB,
				 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	png_bytep row = (png_bytep) malloc(3 * 512 * sizeof(png_byte));

	Tile tile;
	Color color;
	for (int y = 0; y < 512; y++) {
		for (int x = 0; x < 512; x++) {
			tile = grid.data[x / TILE_SIZE][y / TILE_SIZE];
			color = get_color(tile.current_type, tile.state);
			row[x * 3 + 0] = color.r; // Red
			row[x * 3 + 1] = color.g;   // Green
			row[x * 3 + 2] = color.b;   // Blue
		}
		png_write_row(png, row);
	}

	// Finish writing the file
	png_write_end(png, NULL);

	// Free resources
	fclose(fp);
	png_destroy_write_struct(&png, &info);
	free(row);

	free(file_name);
}

/**
 * Write to a csv file
 *
 * @param grid The grid to write
 */
void write_csv(Grid grid) {
	FILE * fp = fopen("grids.csv", "a");

	fprintf(fp, "NEW GRID\n"); // Grid Separator

	for (int x = 0; x < GRID_SIZE; x++) {
		for (int y = 0; y < GRID_SIZE; ++y) {
			fprintf(fp, "%d-%d-%d,", grid.data[x][y].current_type, grid.data[x][y].default_type, grid.data[x][y].state);
		}

		fprintf(fp, "\n");
	}

	fclose(fp);
}

/**
 * Destroy a grid
 *
 * @param grid The grid to destroy
 */
void destroy_grid(Grid grid) {
	if (grid.export_png) {
		write_png(grid);
	}

	if (grid.export_csv) {
		write_csv(grid);
	}

	// Free the data of the grid
	for (int i = 0; i < GRID_SIZE; i++) {
		free(grid.data[i]);
	}

	free(grid.data);
}