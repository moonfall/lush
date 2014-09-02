#include <OctoWS2811.h>
#include "lush.h"

const unsigned PULSE_COUNT = 2;
const uint32_t PULSE_MS = 250;
const uint32_t PAUSE_MS = 400;
const uint32_t DARK_MS = 2000;
const uint32_t CYCLE_MS = PULSE_COUNT * (PULSE_MS + PAUSE_MS) + DARK_MS;

const unsigned HEART[] = {
	0x66,
	0xff,
	0xff,
	0x7e,
	0x3c,
	0x18,
};
const unsigned HEART_ROWS = sizeof(HEART) / sizeof(HEART[0]);

void Pattern_heart::activate(void *arg)
{
    draw_pixels(COLOUR_BLACK);
    m_activate_ms = millis();
    m_last_colour = COLOUR_BLACK;
}

bool Pattern_heart::display()
{
    uint32_t t = (millis() - m_activate_ms) % CYCLE_MS;

    Colour c = COLOUR_BLACK;
    if (t < CYCLE_MS - DARK_MS) {
	uint32_t cycle_t = t % (PULSE_MS + PAUSE_MS);
	if (cycle_t >= PULSE_MS) {
	    c = COLOUR_BLACK;
	} else {
	    int brightness = 2 * g_brightness.get() *
		    	     sin(M_PI * cycle_t / PULSE_MS);
	    c = make_rgb(brightness, 0, 0);
	}
    } else {
	c = COLOUR_BLACK;
    }

    if (c == m_last_colour) {
	return false;
    }
    m_last_colour = c;

    int start_x = (COLUMN_COUNT - 8) / 2;
    int start_y = (ROW_COUNT - HEART_ROWS) / 2;
    for (int y_pos = 0; y_pos < (int) HEART_ROWS; ++y_pos) {
	draw_mask8(start_x, start_y + y_pos, HEART[y_pos], c, &COLOUR_BLACK);
    }

    return true;
}
