#include <OctoWS2811.h>
#include "lush.h"

bool Pattern_huey::display()
{
    int hue = g_hue.get();
    int offset = 1;
    for (int led = 0; led < LED_COUNT; ++led, hue += offset) {
	draw_pixel(led, make_hue(hue));
    }

    return true;
}

