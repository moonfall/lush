#include "patterns.h"

void Pattern_line::activate(void *arg)
{
    m_s0 = pick_corner();
    m_s1 = pick_corner();
    m_e0 = pick_corner();
    m_e1 = pick_corner();
}

const int MAX_T = 12;
bool Pattern_line::display()
{
    ++m_t;
    if (m_t >= MAX_T) {
	m_t = 0;
	reset();
    }

    Coord p0 = interpolate(m_s0, m_e0, m_t, MAX_T);
    Coord p1 = interpolate(m_s1, m_e1, m_t, MAX_T);

    Line l(p0, p1, make_current_hue());
    l.draw();

    return true;
}

void Pattern_line::reset()
{
    m_s0 = m_e0;
    m_s1 = m_e1;

    do {
	m_e0 = pick_corner();
#if 0
	if (m_s0.m_x == flip_x(m_e0.m_x) &&
	    m_s0.m_y == flip_y(m_e0.m_y)) {
	    continue;
	}
#endif
	m_e1 = pick_corner();
#if 0
	if (m_s1.m_x == flip_x(m_e1.m_x) &&
	    m_s1.m_y == flip_y(m_e1.m_y)) {
	    continue;
	}
#endif
    } while ((m_s0 == m_e0 && m_s1 == m_e1) || (m_e0 == m_e1));
}

Coord Pattern_line::pick_corner()
{
    int x = random(2) == 0 ? 0 : flip_x(0);
    int y = random(2) == 0 ? 0 : flip_y(0);

    return Coord(x, y);
}
