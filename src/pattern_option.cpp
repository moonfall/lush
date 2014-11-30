#include "lush.h"
#include "patterns.h"

Pattern_option::Pattern_option(char const *name, Value &value, bool show_value)
    : m_name(name), m_value(value), m_show_value(show_value)
{
}

void Pattern_option::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    m_value.modify(element.get_current_change());
	    Serial.print(m_name);
	    Serial.print(" changed to ");
	    Serial.println(m_value.get());
	    break;

	default:
	    break;
    }
}

bool Pattern_option::display()
{
    draw_pixels(COLOUR_BLACK);

    Colour c = make_rgb(0, g_brightness.get(), 0);
    draw_centered_string(0, 0, COLUMN_COUNT, FONT_HEIGHT, m_name, c,
		         &COLOUR_BLACK);

    c = make_rgb(0, 0, g_brightness.get());
    if (m_show_value) {
	int value = m_value.get();
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    // Display bottom row of bits.
	    int bit = flip_x(x);
	    draw_pixel(x, ROW_COUNT - 1,
		       (value & (1 << bit)) ? c : COLOUR_BLACK);

	    // Display top row of bits.
	    bit = COLUMN_COUNT + flip_x(x);
	    draw_pixel(x, ROW_COUNT - 2,
		       (value & (1 << bit)) ? c : COLOUR_BLACK);
	}

    }

    return true;
}

