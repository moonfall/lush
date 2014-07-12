#include <OctoWS2811.h>
#include "lush.h"

const int HOLD_MS = 1000;
#if 0
const int PIXEL_VELOCITY = 200;
#else
const int PIXEL_VELOCITY = 20;
const int WATERMARK_ROW = 0;
#endif

Pattern_dropper::Pattern_dropper()
{
}

void Pattern_dropper::activate()
{
    for (int led = 0; led < LED_COUNT; ++led) {
	m_field[led] = COLOUR_BLACK;
    }
    m_drop.reset();
    m_reset_ms = 0;
}

bool Pattern_dropper::display()
{
    if (m_reset_ms) {
	// In reset mode.
	if (millis() > m_reset_ms + HOLD_MS) {
	    activate();
	} else {
	    return false;
	}
    }

    for (int led = 0; led < LED_COUNT; ++led) {
	draw_pixel(led, m_field[led]);
    }

    int x = m_drop.m_x.get();
    int y = m_drop.m_y.get();

    bool done = false;
    if (m_field[get_led(x, y)] != COLOUR_BLACK) {
	// Collision, back drop up.
	--y;
	done = true;
    } else if (y == ROW_COUNT - 1) {
	done = true;
    }

    draw_pixel(x, y, m_drop.m_c);

    if (done) {
	m_field[get_led(x, y)] = m_drop.m_c;

	bool filled = true;
	for (x = 0; x < COLUMN_COUNT; ++x) {
	    if (m_field[get_led(x, 0)] == COLOUR_BLACK) {
		filled = false;
		break;
	    }
	}

	if (filled) {
	    m_reset_ms = millis();
	} else {
	    // Don't pick a column that is already full.
	    do {
		m_drop.reset();
	    } while(m_field[get_led(m_drop.m_x.get(), WATERMARK_ROW)] !=
		    COLOUR_BLACK);
	}
    }

    return true;
}

Pattern_dropper::Drop::Drop()
{
    reset();
}

void Pattern_dropper::Drop::reset()
{
    m_c = make_wheel(random(MAX_WHEEL), g_brightness.get());
    m_x.set(random(COLUMN_COUNT));
    m_y.set(0);
    m_y.set_velocity(1, PIXEL_VELOCITY);
}

