#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

const int HUE_OFFSET = 50;

Pattern_alphabet::Pattern_alphabet()
    : m_counter(FONT_START, FONT_START, FONT_END - 1, true)
{
}

void Pattern_alphabet::setup()
{
    m_counter.set_velocity(1, 250);
}

void Pattern_alphabet::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    m_counter.modify(element.get_current_change());
	default:
	    break;
    }
}

bool Pattern_alphabet::display()
{
    Colour c = make_current_hue();

    char s[2];
    s[0] = (char) m_counter.get();
    s[1] = '\0';

    draw_centered_string(0, 0, COLUMN_COUNT, ROW_COUNT, s, c, &COLOUR_BLACK);

    return true;
}

