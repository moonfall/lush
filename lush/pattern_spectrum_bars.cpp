#include <OctoWS2811.h>
#include "lush.h"

void Pattern_spectrum_bars::activate()
{
    g_bin_count.set(COLUMN_COUNT);
}

#define PEAK_HOLD
#ifdef PEAK_HOLD
// TODO: Change to constant.
float g_peaks[8];
Colour g_peak_leds[8];
const float PEAK_FADE = 0.9;
#endif

bool Pattern_spectrum_bars::display()
{
    int hue = g_hue.get();

    for (int x = 0; x < COLUMN_COUNT; ++x) {
	float intensity = g_bins[x];
#ifdef PEAK_HOLD
	if (intensity > g_peaks[x]) {
	    g_peaks[x] = intensity;
	    g_peak_leds[x] = make_current_hue();
	}
#endif
	// How many rows are lit up because of the current sample.
	int rows = intensity * ROW_COUNT;
#ifdef PEAK_HOLD
	// How many rows are lit up because of peak hold.
	int peak_rows = g_peaks[x] * ROW_COUNT;
#endif
	for (int y = 0; y < ROW_COUNT; ++y) {
	    int led = get_led(x, flip_y(y));
	    if (y < rows) {
		draw_pixel(led, make_current_hue());
#ifdef PEAK_HOLD
	    } else if (y < peak_rows) {
		draw_pixel(led, g_peak_leds[x]);
#endif
	    } else {
		draw_pixel(led, COLOUR_BLACK);
	    }
	}
#ifdef PEAK_HOLD
	// Change to time based.
	g_peaks[x] = g_peaks[x] * PEAK_FADE;
#endif
    }

    return true;
}

