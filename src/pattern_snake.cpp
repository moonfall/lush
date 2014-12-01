#include "displaylist.h"
#include "patterns.h"

Pattern_snake::Pattern_snake()
    : m_max_length(MAX_LENGTH / 4, 1, MAX_LENGTH),
      m_length(0),
      m_x(0),
      m_y(0)
{
}

void Pattern_snake::setup()
{
    m_x = random(COLUMN_COUNT);
    m_y = random(ROW_COUNT);
}

void Pattern_snake::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    m_max_length.modify(element.get_current_change());
	default:
	    break;
    }
}

bool Pattern_snake::display()
{
    advance();
    for (unsigned i = 0; i < m_length; ++i) {
	m_segments[i].draw();
    }

    return true;
}

bool Pattern_snake::is_valid(int x, int y) const
{
    return (x >= 0 && x < COLUMN_COUNT && y >= 0 && y < ROW_COUNT);
}

bool Pattern_snake::is_collision(int x, int y) const
{
    // Check all existing segments.
    for (unsigned i = 0; i < m_length; ++i) {
	if (m_segments[i].m_x == x && m_segments[i].m_y == y) { 
	    return true;
	}
    }

    return false;
}

void Pattern_snake::advance()
{
    // Count how many open directions there are.
    unsigned num_open = 0;
    int x;
    int y;
    for (int i = 0; i < NUM_DIRECTIONS; ++i) {
	x = m_x;
	y = m_y;
	make_neighbour((Direction) i, x, y);
	if (is_valid(x, y) && !is_collision(x, y)) {
	    ++num_open;
	}
    }

    // Find the next spot to move to, ignoring invalid spots and collisions
    // unless necessary.
    do {
	x = m_x;
	y = m_y;
	Direction d = (Direction) random(NUM_DIRECTIONS);
	make_neighbour(d, x, y);
    } while (!is_valid(x, y) ||
	     (num_open != 0 && is_collision(x, y)));
    m_x = x;
    m_y = y;

    while ((int) m_length >= m_max_length.get()) {
	memmove(m_segments, m_segments + 1,
		sizeof(Pixel) * (m_length - 1));
	--m_length;
    }

    m_segments[m_length] = Pixel(m_x, m_y, make_current_hue());
    ++m_length;
}

