#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

void Pattern_spectrum_bars::activate(void *arg)
{
    set_fft_bin_count(COLUMN_COUNT);
    set_fft_scale_factor(ROW_COUNT + 1);
}

#define PEAK_HOLD
#ifdef PEAK_HOLD
// TODO: Change to constant.
Bin_type g_peaks[COLUMN_COUNT];
Colour g_peak_leds[COLUMN_COUNT];
const int PEAK_FADE_TIME = 3;
#endif

bool Pattern_spectrum_bars::display()
{
    Colour c = make_current_hue();
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	Bin_type bin = g_bins[x];
#ifdef PEAK_HOLD
	if (bin * PEAK_FADE_TIME > g_peaks[x]) {
	    g_peaks[x] = bin * PEAK_FADE_TIME;
	    g_peak_leds[x] = c;
	    g_peak_leds[x] = make_invert_hue();
	}
#endif
	// How many rows are lit up because of the current sample.
	int rows = bin;
#ifdef PEAK_HOLD
	// How many rows are lit up because of peak hold.
	int peak_rows = g_peaks[x] / PEAK_FADE_TIME;
#endif
	for (int y = 0; y < ROW_COUNT; ++y) {
#ifdef PEAK_HOLD
	    // alternative: y < peak_rows
	    if (y == peak_rows - 1) {
		draw_pixel(x, flip_y(y), g_peak_leds[x]);
	    } else
#endif
	    if (y < rows) {
		draw_pixel(x, flip_y(y), c);
	    } else {
		draw_pixel(x, flip_y(y), COLOUR_BLACK);
	    }
	}
#ifdef PEAK_HOLD
	// Change to time based.
	if (g_peaks[x] > 0) {
	    --g_peaks[x];
	}
#endif
    }

    return true;
}

