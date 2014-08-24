#include "lush.h"

const uint32_t ADVANCE_MS = 3000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_main_menu::Pattern_main_menu(Mode *modes, unsigned num_modes)
    : Pattern_set(modes, num_modes)
{
}

void Pattern_main_menu::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB1_BUTTON:
	    if (element.get_current_change() > 0) {
		Serial.printf("press at %u\n", millis());
		m_unhandled_button_press_ms = millis();
	    } else if (element.get_current_change() < 0) {
		Serial.printf("release at %u\n", millis());
		m_unhandled_button_press_ms = 0;
	    }
	default:
	    break;
    }

    Pattern_set::ui_callback(id, element);
}

void Pattern_main_menu::ui_hook()
{
    uint32_t now = millis();
    if (m_unhandled_button_press_ms &&
	now > m_unhandled_button_press_ms + ADVANCE_MS) {
	m_current_mode.modify(1);
	activate_child();

	// Let someone hold to advance main menu modes.
	m_unhandled_button_press_ms = now - ADVANCE_MS + SHOW_STATUS_MS;
	display_overlay_until(now + SHOW_STATUS_MS);
    }

    Pattern_set::ui_hook();
}

void Pattern_main_menu::activate(void *arg)
{
    Pattern_set::activate(arg);
}

void Pattern_main_menu::display_overlay()
{
    display_status_string(m_modes[m_current_mode.get()].m_id);
}
