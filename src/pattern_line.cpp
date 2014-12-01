#include "patterns.h"

void Pattern_line::activate(void *arg)
{
    m_s0 = pick_corner();
    m_s1 = pick_corner();
    m_e0 = pick_corner();
    m_e1 = pick_corner();
    m_t = 0;
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
	m_e1 = pick_corner();
    } while (!is_valid_move(m_s0, m_s1, m_e0, m_e1));
}

Coord Pattern_line::pick_corner()
{
    int x = random(2) == 0 ? 0 : flip_x(0);
    int y = random(2) == 0 ? 0 : flip_y(0);

    return Coord(x, y);
}

bool Pattern_line::is_valid_move(Coord const &s0, Coord const &s1,
				 Coord const &e0, Coord const &e1)
{
    // No movement at all
    bool p0_same = s0 == e0;
    bool p1_same = s1 == e1;
    if (p0_same && p1_same) {
	return false;
    }

    // Don't allow diagonal flip with one stationary point
    bool p0_flipped = s0.is_flipped(e0);
    bool p1_flipped = s1.is_flipped(e1);
    if ((p0_same && p1_flipped) || (p1_same && p0_flipped)) {
	return false;
    }
    
    // Don't allow a single point to have a non-moving point
    bool s_same = s0 == s1;
    bool e_same = e0 == e1;
    if ((s_same || e_same) && (p0_same || p1_same)) {
	return false;
    }

    return true;
}
