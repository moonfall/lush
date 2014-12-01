#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

const int HUE_OFFSET = 50;

Pattern_counter::Pattern_counter()
    : m_counter(0)
{
}

void Pattern_counter::setup()
{
    m_counter.set_velocity(1, 250);
}

void Pattern_counter::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    m_counter.modify(element.get_current_change());
	default:
	    break;
    }
}

bool Pattern_counter::display()
{
    draw_pixels(COLOUR_BLACK);

    int counter = m_counter.get() % 100;
    Colour c = make_current_hue();
    draw_char(0, 1, counter / 10 + '0', c, &COLOUR_BLACK);
    draw_char(FONT_WIDTH, 1, counter % 10 + '0', c, &COLOUR_BLACK);

    c = make_hue(g_hue.get() + HUE_OFFSET);
    int binary = m_counter.get() % (1 << COLUMN_COUNT);
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	int bit = flip_x(x);
	draw_pixel(x, ROW_COUNT - 1, (binary & (1 << bit)) ? c : COLOUR_BLACK);
    }

    return true;
}

