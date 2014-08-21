#include "lush.h"

Pattern_set::Pattern_set(Mode *modes, unsigned num_modes)
    : m_modes(modes), m_num_modes(num_modes),
      m_current_mode(0, 0, m_num_modes - 1, true),
      m_force_update(true)
{
}

void Pattern_set::ui_callback(Element_id id, Element const &element)
{
    get_child()->ui_callback(id, element);
}

void Pattern_set::ui_hook()
{
    get_child()->ui_hook();
}

void Pattern_set::activate(void *arg)
{
    activate_child();
}

bool Pattern_set::display()
{
    bool needs_update = get_child()->display() || m_force_update;
    m_force_update = false;
    return needs_update;
}

Pattern *Pattern_set::get_child()
{
    Mode &mode = m_modes[m_current_mode.get()];
    return mode.m_pattern;
}

void Pattern_set::activate_child()
{
    Mode &mode = m_modes[m_current_mode.get()];
    mode.m_pattern->activate(mode.m_arg);
    m_force_update = true;
}