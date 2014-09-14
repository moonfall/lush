#include <OctoWS2811.h>
#include "lush.h"

// TODO: set velocity based on beat
const int PIXEL_VELOCITY = 40;

Pattern_rain::Pattern_rain()
{
}

void Pattern_rain::activate(void *arg)
{
    m_mode = (Mode)(intptr_t)(arg);
    if (m_mode == RAIN_RANDOM ||
	(unsigned) m_mode >= RAIN_NUM_MODES) {
	m_mode = (Mode) random(1, RAIN_NUM_MODES);
    }
    if (m_mode == RAIN_SINGLE_RANDOM_COLOUR) {
	m_wheel = random(MAX_WHEEL);
    }
    m_drop.reset(get_colour());
}

bool Pattern_rain::display()
{
    draw_pixels(COLOUR_BLACK);

    int y = m_drop.m_y.get();
    if (y >= ROW_COUNT) {
	m_drop.reset(get_colour());
	y = m_drop.m_y.get();
    }
    int x = m_drop.m_x.get();

    draw_pixel(x, y, m_drop.m_c);

    return true;
}

Colour Pattern_rain::get_colour() const
{
    switch (m_mode) {
	case RAIN_RANDOM_COLOUR:
	default:
	    return make_wheel(random(MAX_WHEEL), g_brightness.get());
	case RAIN_CURRENT_HUE:
	    return make_hue(g_hue.get());
	case RAIN_SINGLE_RANDOM_COLOUR:
	    return make_wheel(m_wheel, g_brightness.get());
	case RAIN_PURE_WHITE:
	    return make_rgb(MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    }
}

Pattern_rain::Drop::Drop(Colour c)
{
    reset(c);
}

void Pattern_rain::Drop::reset(Colour c)
{
    m_c = c;
    m_x.set(random(COLUMN_COUNT));
    m_y.set(0);
    m_y.set_velocity(1, PIXEL_VELOCITY);
}

