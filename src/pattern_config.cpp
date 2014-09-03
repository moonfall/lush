#include "lush.h"

const uint32_t LOCK_MS = 1000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_config::Pattern_config(Mode *modes, unsigned num_modes)
    : Pattern_set(modes, num_modes)
{
}

void Pattern_config::ui_callback(Element_id id, Element const &element)
{
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

    Pattern_set::ui_callback(id, element);
}
