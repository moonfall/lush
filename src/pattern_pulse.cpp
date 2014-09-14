#include <OctoWS2811.h>
#include "lush.h"

const int DARK_MS = 2000;
const int PULSE_COUNT = 1;
const int PULSE_MS = 150;
const int PAUSE_MS = 300;
const int CYCLE_MS = PULSE_COUNT * (PULSE_MS + PAUSE_MS) + DARK_MS;

void Pattern_pulse::activate(void *arg)
{
    draw_pixels(COLOUR_BLACK);
    m_activate_ms = millis();
    m_was_dark = true;
    m_wheel = millis() % MAX_WHEEL;
    m_last_colour = COLOUR_BLACK;
}

bool Pattern_pulse::display()
{
    uint32_t t = (millis() - m_activate_ms) % CYCLE_MS;

    Colour c = COLOUR_BLACK;
    if (t < CYCLE_MS - DARK_MS) {
	if (m_was_dark) {
	    // Pick a colour and stick with it.
	    m_wheel = millis() % MAX_WHEEL;
	    m_was_dark = false;
	}

	int cycle_t = t % (PULSE_MS + PAUSE_MS);
	if (cycle_t >= PULSE_MS) {
	    c = COLOUR_BLACK;
	} else {
	    int brightness = g_brightness.get() *
			     sin(M_PI * cycle_t / PULSE_MS);
	    c = make_wheel(m_wheel, brightness);
	}
    } else {
	m_was_dark = true;
    }

    if (c == m_last_colour) {
	return false;
    }
    m_last_colour = c;

    draw_pixels(c);

    return true;
}
