#include <OctoWS2811.h>
#include "blocks.h"

void shuffle_array(int *array, int count)
{
    for (int i = 0; i < count; ++i) {
	int j = random(i, count);
	swap(array[i], array[j]);
    }
}

void reverse_array(int *array, int count)
{
    for (int i = 0; i < count / 2; ++i) {
	swap(array[i], array[count - i - 1]);
    }
}

void make_shuffled_array(int *array, int count)
{
    for (int i = 0; i < count; ++i) {
	int j = random(i + 1);
	if (j != i) {
	    array[i] = array[j];
	}
	array[j] = i;
    }
}

void Generator::fill_array(int *array, int count)
{
    for (int i = 0; i < count; ++i) {
	array[current()] = i;
	next();
    }
}

void expand_array_1d(int *array, int count, int scale)
{
    if (scale == 1) {
	return;
    }

    // Work back to front. 
    for (int i = count - 1; i >= 0; --i) {
	int orig = array[i];
	for (int j = 0; j < scale; ++j) {
	    array[i * scale + j] = orig * scale + j;
	}
    }
}

void expand_array_2d(int *array, int columns, int rows, int scale)
{
    if (scale == 1) {
	return;
    }

    // Work back to front. 
    for (int i = columns * rows - 1; i >= 0; --i) {
	int orig = array[i];
	int orig_x;
	int orig_y;
	get_xy(i, orig_x, orig_y, columns);
	int j = 0;
	for (int y = scale * orig_y; y < scale * (orig_y + 1); ++y) {
	    for (int x = scale * orig_x; x < scale * (orig_x + 1); ++x, ++j) {
		// New column count after scaling.
		int index = get_led(x, y, columns * scale);
		array[index] = orig * scale * scale + j;
	    }
	}
    }
}
