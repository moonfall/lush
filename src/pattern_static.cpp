#include "lush.h"

const uint32_t LOCK_MS = 1000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_static::Pattern_static(Mode *modes, unsigned num_modes)
    : Pattern_set(modes, num_modes)
{
}

void Pattern_static::ui_callback(Element_id id, Element const &element)
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

    if (!m_locked) {
	switch (id) {
	    case UI_KNOB1_ENCODER:
		m_current_mode.modify(element.get_current_change());
		Serial.printf("mode changed to %d\n", m_current_mode.get());
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

void Pattern_static::ui_hook()
{
    uint32_t now = millis();
    if (m_unhandled_button_press_ms &&
	now > m_unhandled_button_press_ms + LOCK_MS) {
	m_locked = !m_locked;
	m_unhandled_button_press_ms = 0;
	m_status_start_ms = now;
    }

    Pattern_set::ui_hook();
}

void Pattern_static::activate(void *arg)
{
    m_locked = false;
    Pattern_set::activate(arg);
}

bool Pattern_static::display()
{
    bool needs_update = Pattern_set::display();
    if (millis() < m_status_start_ms + SHOW_STATUS_MS) {
	display_status();
	needs_update = true;
    }
    return needs_update;
}

void Pattern_static::display_status()
{
}
