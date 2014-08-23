#include "lush.h"

const uint32_t LOCK_MS = 1000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_random::Pattern_random(Mode *modes, unsigned num_modes)
    : Pattern_set(modes, num_modes), m_duration_s(10)
{
}

void Pattern_random::ui_callback(Element_id id, Element const &element)
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

void Pattern_random::ui_hook()
{
    uint32_t now = millis();
    if (m_unhandled_button_press_ms &&
	now > m_unhandled_button_press_ms + LOCK_MS) {
	m_locked = !m_locked;
	m_last_select_ms = millis();
	m_unhandled_button_press_ms = 0;
	display_overlay_until(now + SHOW_STATUS_MS);
    }

    if (!m_locked && now > m_last_select_ms + m_duration_s * 1000) {
	select_next();
	activate_child();
    }

    Pattern_set::ui_hook();
}

void Pattern_random::activate(void *arg)
{
    m_locked = false;
    select_next();
    Pattern_set::activate(arg);
}

void Pattern_random::select_next()
{
    m_current_mode.set(random(m_num_modes));
    m_last_select_ms = millis();
}

void Pattern_random::display_overlay()
{
    display_status_string(m_locked ? "L" : "U");
}
