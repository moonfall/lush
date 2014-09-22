#include <OctoWS2811.h>
#include "lush.h"

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
#endif

bool Pattern_spectrum_bars::display()
{
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	Bin_type bin = g_bins[x];
#ifdef PEAK_HOLD
	if (bin > g_peaks[x]) {
	    g_peaks[x] = bin;
	    g_peak_leds[x] = make_current_hue();
	}
#endif
	// How many rows are lit up because of the current sample.
	int rows = bin;
#ifdef PEAK_HOLD
	// How many rows are lit up because of peak hold.
	int peak_rows = g_peaks[x];
#endif
	for (int y = 0; y < ROW_COUNT; ++y) {
	    if (y < rows) {
		draw_pixel(x, flip_y(y), make_current_hue());
#ifdef PEAK_HOLD
	    } else if (y < peak_rows) {
		draw_pixel(x, flip_y(y), g_peak_leds[x]);
#endif
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

