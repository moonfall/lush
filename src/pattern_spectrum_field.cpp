#include <OctoWS2811.h>
#include "lush.h"

const unsigned SCALE_FACTOR = 100;
const unsigned SCALE_DENOMINATOR = SCALE_FACTOR * 3;

void Pattern_spectrum_field::activate(void *arg)
{
    set_fft_bin_count(LED_COUNT);
    set_fft_scale_factor(SCALE_FACTOR);
}

bool Pattern_spectrum_field::display()
{
    int hue = g_hue.get();

    Bin_type *intensity = g_bins;
    for (int y = ROW_COUNT - 1; y >= 0; --y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) { 
	    int lightness = *intensity * g_brightness.get() / SCALE_DENOMINATOR;
	    draw_pixel(x, y, make_hsv(hue, MAX_SATURATION, lightness));
	    ++intensity;
	}
    }

    return true;
}

