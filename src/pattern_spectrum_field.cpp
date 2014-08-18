#include <OctoWS2811.h>
#include "lush.h"

void Pattern_spectrum_field::activate(void *arg)
{
    g_bin_count.set(LED_COUNT);
}

bool Pattern_spectrum_field::display()
{
    int hue = g_hue.get();

    Bin_type *intensity = g_bins;
    for (int y = ROW_COUNT - 1; y >= 0; --y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) { 
	    int lightness = *intensity * g_brightness.get();
	    draw_pixel(get_led(x, y),
		       make_hsv(hue, MAX_SATURATION, lightness));
	    ++intensity;
	}
    }

    return true;
}

