#include <OctoWS2811.h>
#include "lush.h"

Pattern_huey::Pattern_huey()
    : m_hue_offset(1, 0, 255, true)
{
}

void Pattern_huey::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    if (g_ui.m_knob2_button.get_current().m_value) {
		m_hue_offset.modify(element.get_current_change());
	    } else {
		g_hue.modify(element.get_current_change());
	    }
	default:
	    break;
    }
}

bool Pattern_huey::display()
{
    int hue = g_hue.get();
    int offset = m_hue_offset.get();
    for (int led = 0; led < LED_COUNT; ++led, hue += offset) {
	draw_pixel(led, make_hue(hue));
    }

    return true;
}
