#include "lush.h"
#include "patterns.h"

Pattern_composite::Pattern_composite(const Mode *modes, unsigned num_modes)
    : m_modes(modes), m_num_modes(num_modes), m_force_update(true)
{
}

void Pattern_composite::ui_callback(Element_id id, Element const &element)
{
    get_top()->ui_callback(id, element);
}

void Pattern_composite::ui_hook()
{
    get_top()->ui_hook();
}

void Pattern_composite::activate(void *arg)
{
    for (unsigned i = 0; i < m_num_modes; ++i) {
	const Mode &mode = m_modes[i];
	mode.m_pattern->activate(mode.m_arg);
    }
    m_force_update = true;
}

bool Pattern_composite::display()
{
    bool needs_update = m_force_update;
    for (unsigned i = 0; i < m_num_modes; ++i) {
	const Mode &mode = m_modes[i];
	if (mode.m_pattern->display()) {
	    needs_update = true;
	}
    }
    return needs_update;
}

Pattern *Pattern_composite::get_top()
{
    return m_num_modes == 0 ? NULL : m_modes[m_num_modes - 1].m_pattern;
}
