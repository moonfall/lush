#include <OctoWS2811.h>
#include <Audio.h>
#include "lush.h"

void Pattern_peak::activate()
{
}

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

    int leds = get_mapped_peak(LED_COUNT);
    Colour c = make_hue(g_hue.get());

#if 0
    leds = get_mapped_peak((COLUMN_COUNT + 1) * ROW_COUNT / 2);
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
    leds = get_mapped_peak(LED_COUNT / 2);

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

