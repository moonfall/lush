#include "lush.h"
#include "patterns.h"

const uint32_t LOCK_MS = 1000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_selector::Pattern_selector(const Mode *modes, unsigned num_modes)
    : Pattern_set(modes, num_modes)
{
}

void Pattern_selector::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB1_BUTTON:
	    if (element.get_current_change() > 0) {
		Serial.print("press at ");
		Serial.println(millis());
		m_unhandled_button_press_ms = millis();
	    } else if (element.get_current_change() < 0) {
		Serial.print("release at ");
		Serial.println(millis());
		m_unhandled_button_press_ms = 0;
	    }
	default:
	    break;
    }

    if (!m_locked) {
	switch (id) {
	    case UI_KNOB1_ENCODER:
		m_current_mode.modify(element.get_current_change());
		Serial.print("mode changed to ");
		Serial.println(m_current_mode.get());
		activate_child();
		return;
	    default:
		// fallthrough
		break;
	}

	// fallthrough
    }

    Pattern_set::ui_callback(id, element);
}

void Pattern_selector::ui_hook()
{
    uint32_t now = millis();
    if (m_unhandled_button_press_ms &&
	now > m_unhandled_button_press_ms + LOCK_MS) {
	m_locked = !m_locked;
	m_unhandled_button_press_ms = 0;
	display_overlay_until(now + SHOW_STATUS_MS);
    }

    Pattern_set::ui_hook();
}

void Pattern_selector::activate(void *arg)
{
    m_locked = false;
    Pattern_set::activate(arg);
}

void Pattern_selector::display_overlay()
{
    display_status_string(m_locked ? "L" : "U");
}
