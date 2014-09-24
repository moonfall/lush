#include "lush.h"

const uint32_t LOCK_MS = 1000;
const uint32_t SHOW_STATUS_MS = 1000;

Pattern_random::Pattern_random(const Mode *modes, unsigned num_modes)
    : Pattern_selector(modes, num_modes), m_duration_s(10)
{
}

void Pattern_random::ui_hook()
{
    uint32_t now = millis();

    // Randomly select a new mode when unlocked.
    if (!m_locked && now > m_last_select_ms + m_duration_s * 1000) {
	select_next();
	activate_child();
    }

    bool was_locked = m_locked;
    Pattern_selector::ui_hook();

    // If the lock status was changed, hold the current pattern for
    // a full cycle.
    if (was_locked != m_locked) {
	m_last_select_ms = now;
    }
}

void Pattern_random::activate(void *arg)
{
    select_next();
    Pattern_selector::activate(arg);
}

void Pattern_random::activate_child()
{
    Pattern_selector::activate_child();
    m_last_select_ms = millis();
}

void Pattern_random::select_next()
{
    m_current_mode.set(random(m_num_modes));
}

void Pattern_random::display_overlay()
{
    display_status_string(m_locked ? "L" : "U");
}
