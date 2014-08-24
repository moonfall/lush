#include <OctoWS2811.h>
#include "lush.h"

const int HUE_OFFSET = 50;

/*
 * scrolling:
 * - current position
 * - character at current position
 * - current speed
 * commands:
 * - reset
 */

Pattern_marquee::Pattern_marquee()
    : m_pos(0)
{
    strcpy(m_text, "HELLO");
}

void Pattern_marquee::setup()
{
    m_pos.set_velocity(-1, 100);
}

void Pattern_marquee::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    m_pos.modify(element.get_current_change());
	default:
	    break;
    }
}

bool Pattern_marquee::display()
{
    draw_pixels(COLOUR_BLACK);

    Colour c = make_current_hue();

    draw_string(m_pos.get(), 1, m_text, c, &COLOUR_BLACK, DIR_RIGHT, 1);
    if (m_pos.get() < -FONT_WIDTH * (int) (strlen(m_text) + 2)) {
	m_pos.set(COLUMN_COUNT);
    }

    return true;
}

