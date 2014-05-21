#include <OctoWS2811.h>
#include "lush.h"

void Pattern_huey::setup()
{
    m_hue_offset.set(1);
}

bool Pattern_huey::display()
{
    int hue = g_hue.get();
    int offset = m_hue_offset.get();
    for (int led = 0; led < LED_COUNT; ++led, hue += offset) {
	draw_pixel(led, make_hue(hue));
    }

    return true;
}

