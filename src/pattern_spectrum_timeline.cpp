#include <OctoWS2811.h>
#include "lush.h"

const unsigned SCALE_FACTOR = 100;
const unsigned SCALE_DENOMINATOR = SCALE_FACTOR * 3;

void Pattern_spectrum_timeline::activate(void *arg)
{
    set_fft_bin_count(COLUMN_COUNT);
    set_fft_scale_factor(SCALE_FACTOR);
}

// -1 or +1 for reverse or forward.
const int TIMELINE_DIRECTION = +1;

bool Pattern_spectrum_timeline::display()
{
    int hue = g_hue.get();

    // Copy previous rows.
    int start_row = TIMELINE_DIRECTION > 0 ? 0 : ROW_COUNT - 1;
    int end_row = flip_y(start_row);
    for (int y = end_row; y != start_row; y -= TIMELINE_DIRECTION) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    draw_pixel(x, y, get_pixel(x, y - TIMELINE_DIRECTION));
	}
    }
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	int lightness = g_bins[x] * g_brightness.get() / SCALE_DENOMINATOR;
	draw_pixel(x, start_row, make_hsv(hue, MAX_SATURATION, lightness));
    }

    return true;
}

