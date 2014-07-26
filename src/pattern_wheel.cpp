#include <OctoWS2811.h>
#include "lush.h"

const int CYCLE_MS = 5;

static Colour g_last_colour = COLOUR_BLACK;

bool Pattern_wheel::display()
{
    int wheel = (millis() / CYCLE_MS) % MAX_WHEEL;
    Colour c = make_wheel(wheel, g_brightness.get()); 
    if (c == g_last_colour) {
	return false;
    }

    g_last_colour = c;

    for (int led = 0; led < LED_COUNT; ++led) {
	draw_pixel(led, c);
    }

    return true;
}
