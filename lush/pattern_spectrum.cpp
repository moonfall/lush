#include <OctoWS2811.h>
#include "lush.h"

void Pattern_spectrum::setup()
{
}

bool Pattern_spectrum::display()
{
    int hue = g_hue.get();
    for (int led = 0; led < LED_COUNT; ++led) {
	int value = g_brightness.get() * g_magnitudes[led];
	draw_pixel(led, make_hsv(hue, MAX_SATURATION, value));
    }

    return true;
}

