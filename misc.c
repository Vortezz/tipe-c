#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Get the maximum of two integers
 *
 * @param a The first integer
 * @param b The second integer
 * @return The maximum of the two integers
 */
int max(int a, int b) {
	return a > b ? a : b;
}
/**
 * Get the minimum of two double
 */
int max_3(double a, double b) {
	return a > b ? a : b;
}

/**
 * Get the minimum of two integers
 *
 * @param a The first integer
 * @param b The second integer
 * @return The minimum of the two integers
 */
int min(int a, int b) {
	return a < b ? a : b;
}

/**
 * Get the minimum of two double
 */
int min_3(double a, double b) {
	return a < b ? a : b;
}

int signe(double x) {
    if (x > 0.0) return 1;
    if (x < 0.0) return -1;
    return 0;
}

/**
 * Read a file and return its content as a string
 *
 * @param file The file to read
 * @return The content of the file
 */
char * readfile(FILE * file) {
	// Check if the file is null or if the seek failed
	if (file == NULL || fseek(file, 0, SEEK_END)) {
		return NULL;
	}

	long length = ftell(file);
	rewind(file);
	// Check if the length is invalid
	if (length == -1 || (unsigned long) length >= SIZE_MAX) {
		return NULL;
	}

	// Convert from long to size_t
	size_t ulength = (size_t) length;
	char * buffer = malloc(ulength + 1);
	// Check if the buffer is null or if the read failed
	if (buffer == NULL || fread(buffer, 1, ulength, file) != ulength) {
		free(buffer);
		return NULL;
	}
	// Finish the string
	buffer[ulength] = '\0';

	return buffer;
}

/**
 * Get a random number between 0 and max (excluded)
 *
 * @param max The maximum value
 * @return The random number
 */
int get_random(int max) {
	return (int) (random() % max);
}

/**
 * Get a random number between 0. and 1.
 */
double get_random_3() {
	return (double) rand()/RAND_MAX;
}