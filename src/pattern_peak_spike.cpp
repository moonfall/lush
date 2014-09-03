#include <OctoWS2811.h>
#include <Audio.h>
#include "lush.h"

bool Pattern_peak::display()
{
#define LOG_PEAKS
#ifdef LOG_PEAKS
    Serial.print("peak ");
    Serial.print(get_peak());
    const int max_dots = 64;
    char dots[max_dots + 2];
    memset(dots, ' ', sizeof(dots));
    dots[max_dots] = '|';
    dots[max_dots + 1] = '\0';
    memset(dots, '.', get_mapped_peak(max_dots));
    Serial.print(" ");
    Serial.println(dots);
#endif

    // Clean slate.
    draw_pixels(COLOUR_BLACK);

    Colour c = make_hue(g_hue.get());

#if 0
    int leds = get_mapped_peak((COLUMN_COUNT + 1) * ROW_COUNT / 2);
    int start_x = 0;
    int x = start_x;
    int y = ROW_COUNT - 1;
    for (int led = 0; led < leds; ++led) {
	draw_pixel(x, y, c);	
	--x;
	--y;
	if (x < 0) {
	    ++start_x;
	    x = start_x;
	    y = ROW_COUNT - 1;
	}
	if (y < 0) {
	    break;
	}
    }
#endif
#if 1
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
#endif

    return true;
}

