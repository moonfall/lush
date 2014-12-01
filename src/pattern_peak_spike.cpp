#include <Audio.h>
#include "lush.h"
#include "patterns.h"

bool Pattern_peak_spike::display()
{
#undef LOG_PEAKS
#ifdef LOG_PEAKS
    const int max_dots = 64;
    char dots[max_dots + 2];
    memset(dots, ' ', sizeof(dots));
    dots[max_dots] = '|';
    dots[max_dots + 1] = '\0';
    memset(dots, '.', get_mapped_peak(max_dots));
    Serial.printf("peak %5u %s\n", get_peak(), dots);
#endif

    Colour c = make_hue(g_hue.get());

    int leds = get_mapped_peak(LED_COUNT / 2);

#define FADE_PEAK
#ifdef FADE_PEAK
    static Value held_leds(0, 0, LED_COUNT / 2);
    held_leds.set_velocity(-10, 60);
    if (leds > held_leds.get()) {
	held_leds.set(leds);
    }
    leds = held_leds.get();
#endif

    // Start drawing out from the bottom middle both up and horizontally.
    int start_x = COLUMN_COUNT / 2;
    int start_y = ROW_COUNT - 1;
    int x = start_x;
    int y = start_y;
    for (int led = 0; led < leds; ++led) {
	draw_pixel(x, y, c);
	draw_pixel(flip_x(x), y, c);
	++x;
	++y;
	if (x >= COLUMN_COUNT || y >= ROW_COUNT) {
	    --start_y;
	    if (start_y < 0) {
		start_y = 0;
		++start_x;
	    }
	    x = start_x;
	    y = start_y;
	}
    }

    return true;
}

