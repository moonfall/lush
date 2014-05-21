#include <OctoWS2811.h>
#include "lush.h"

void Pattern_huey::setup()
{
}

Value hue_offset(1);
bool Pattern_huey::display()
{
    int hue = g_hue.get();
    int offset = hue_offset.get();
    for (int led = 0; led < LED_COUNT; ++led, hue += offset) {
	draw_pixel(led, make_hsv(hue, MAX_SATURATION, g_brightness.get()));
    }

    return true;
}

