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
 * Read a file and return its content as a string
 *
 * @param file The file to read
 * @return The content of the file
 */
char * readfile(FILE * file) {
	// f invalid? fseek() fail?
	if (file == NULL || fseek(file, 0, SEEK_END)) {
		return NULL;
	}

	long length = ftell(file);
	rewind(file);
	// Did ftell() fail?  Is the length too long?
	if (length == -1 || (unsigned long) length >= SIZE_MAX) {
		return NULL;
	}

	// Convert from long to size_t
	size_t ulength = (size_t) length;
	char * buffer = malloc(ulength + 1);
	// Allocation failed? Read incomplete?
	if (buffer == NULL || fread(buffer, 1, ulength, file) != ulength) {
		free(buffer);
		return NULL;
	}
	buffer[ulength] = '\0'; // Now buffer points to a string

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