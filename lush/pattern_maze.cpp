#include <OctoWS2811.h>
#include "lush.h"

const uint8_t WALL_VALUE = 0;

const int RESET_MS = 2000;
const int UPDATE_MS = 500;
const int HUE_OFFSET = 3;

Pattern_maze::Pattern_maze()
{
}

void Pattern_maze::activate()
{
    m_maze.reset();

    m_update_ms = millis();
    m_finished_ms = 0;
}

bool Pattern_maze::display()
{
    int now = millis();
    if (m_finished_ms) {
	if (now > m_finished_ms + RESET_MS) {
	    m_maze.reset();
	    m_update_ms = now;
	    m_finished_ms = 0;
	}
    } else if (now > m_update_ms + UPDATE_MS) {
	// Try to expand and record what happened.
	if (m_maze.expand()) {
	    m_update_ms = now;
	} else {
	    m_finished_ms = now;
	}
    }


    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int index = get_led(x, y);
	    int order = m_maze.get_order(index);
	    int hue = g_hue.get();
	    Colour c = COLOUR_BLACK;
	    if (order) {
		c = make_hue(hue + HUE_OFFSET * order);
	    }
#ifdef SHOW_POSSIBILITES
	    if (m_maze.in_wall_list(index)) {
		c = make_hue(hue + 50);
	    }
#endif
	    draw_pixel(index, c);
	}  
    }

    return true;
}
