#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

const uint32_t MOVE_MS = 20;

void Pattern_border::activate(void *arg)
{
    m_origin = random(min(ROW_COUNT / 2, COLUMN_COUNT / 2) - 1);
    m_clockwise = random() % 2 == 0;
    m_x = m_origin;
    m_y = m_origin;

    display_current();
    m_last_move_ms = millis();
}

bool Pattern_border::display()
{
    uint32_t now = millis();
    if (now <= m_last_move_ms + MOVE_MS) {
	return false;
    }
    m_last_move_ms = now;

    move_along_box(m_origin, m_clockwise, m_x, m_y);
    display_current();

    return true;
}

void Pattern_border::display_current()
{
    draw_pixels(COLOUR_BLACK);
    draw_pixel(m_x, m_y, make_current_hue());
}

