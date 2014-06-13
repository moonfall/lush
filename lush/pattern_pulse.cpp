#include <OctoWS2811.h>
#include "lush.h"

const int DARK_MS = 2000;
const int PULSE_COUNT = 2;
const int PULSE_MS = 150;
const int PAUSE_MS = 300;
const int CYCLE_MS = DARK_MS + PULSE_COUNT * (PULSE_MS + PAUSE_MS);

static bool was_dark = true;
static int wheel = 0;

static Colour g_last_colour = COLOUR_BLACK;

bool Pattern_pulse::display()
{
    int t = millis() % CYCLE_MS;

    Colour c = COLOUR_BLACK;
    if (t < DARK_MS) {
	was_dark = true;
    } else {
	if (was_dark) {
	    // Pick a colour and stick with it.
	    wheel = millis() % MAX_WHEEL;
	    was_dark = false;
	}

	int cycle_t = (t - DARK_MS) % (PULSE_MS + PAUSE_MS);
	if (cycle_t >= PULSE_MS) {
	    c = COLOUR_BLACK;
	} else {
	    int brightness = g_brightness.get() * sin(M_PI * cycle_t / PULSE_MS);
	    c = make_wheel(wheel, brightness);
	}
    }

    if (c == g_last_colour) {
	return false;
    }
    g_last_colour = c;

    for (int led = 0; led < LED_COUNT; ++led) {
	draw_pixel(led, c);
    }

    return true;
}
