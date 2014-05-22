#include <OctoWS2811.h>
#include "lush.h"

Pattern_counter::Pattern_counter()
    : m_counter(0, 0, 99, true)
{
}

void Pattern_counter::setup()
{
    m_counter.set_velocity(1, 250);
}

bool Pattern_counter::display()
{
    int counter = m_counter.get();
    Colour c = make_current_hue();
    draw_char(0, 0, counter / 10 + '0', c, &COLOUR_BLACK);
    draw_char(FONT_WIDTH, 0, counter % 10 + '0', c, &COLOUR_BLACK);

    for (int y = FONT_HEIGHT; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    draw_pixel(x, y, COLOUR_BLACK);
	}
    }

    return true;
}

