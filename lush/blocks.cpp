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
    Serial.println();
}
