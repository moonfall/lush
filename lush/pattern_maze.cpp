#include <OctoWS2811.h>
#include "lush.h"

const uint8_t WALL_VALUE = 0;
const uint8_t MAZE_VALUE = 1;

const int RESET_MS = 2000;
const int UPDATE_MS = 100;
const int HUE_OFFSET = 3;

Pattern_maze::Pattern_maze()
{
}

void Pattern_maze::activate()
{
    reset();

    m_update_ms = millis();
    m_finished_ms = 0;
}

bool Pattern_maze::display()
{
    int now = millis();
    if (m_finished_ms) {
	if (now > m_finished_ms + RESET_MS) {
	    reset();
	    m_update_ms = now;
	    m_finished_ms = 0;
	}
    } else if (now > m_update_ms + UPDATE_MS) {
	// Try to expand and record what happened.
	if (expand()) {
	    m_update_ms = now;
	} else {
	    m_finished_ms = now;
	}
    }


    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int index = get_led(x, y);
	    int hue = g_hue.get() + HUE_OFFSET * m_order[index];
	    Colour c = (m_maze[index] == MAZE_VALUE) ?
		       make_hue(hue) : COLOUR_BLACK;
#ifdef SHOW_POSSIBILITES
	    for (int i = 0; i < m_wall_list_count; ++i) {
		if (m_wall_list[i] == index) {
		    c = make_hue(hue + 50);
		}
	    }
#endif
	    draw_pixel(index, c);
	}  
    }

    return true;
}

void Pattern_maze::reset()
{
    for (int i = 0; i < LED_COUNT; ++i) {
	m_maze[i] = WALL_VALUE;
    }

    m_wall_list_count = 0;

    m_queued_update = false;

    m_count = 0;

    add_maze(random(COLUMN_COUNT), random(ROW_COUNT), true);
}

void Pattern_maze::add_wall_list(int x, int y)
{
    int index = get_led(x, y);
    if (m_maze[index] != WALL_VALUE) {
	return;
    }

    m_wall_list[m_wall_list_count] = index;
    ++m_wall_list_count;
}

void Pattern_maze::remove_wall_list(int choice)
{
    if (m_wall_list_count > 0) {
	--m_wall_list_count;
	m_wall_list[choice] = m_wall_list[m_wall_list_count];
    }
}

void Pattern_maze::add_maze(int x, int y, bool add_to_list)
{
    if (x < 0 || x >= COLUMN_COUNT || y < 0 || y >= ROW_COUNT) {
	return;
    }

    int index = get_led(x, y);
    m_maze[index] = MAZE_VALUE;

    m_order[index] = m_count;
    ++m_count;

    for (int i = 0; i < m_wall_list_count; ++i) {
	if (m_wall_list[i] == index) {
	    remove_wall_list(i);
	}
    }

    if (!add_to_list) {
	return;
    }

    if (x > 0) {
	add_wall_list(x - 1, y);
    }

    if (x < COLUMN_COUNT - 1) {
	add_wall_list(x + 1, y);
    }

    if (y > 0) {
	add_wall_list(x, y - 1);
    }

    if (y < ROW_COUNT - 1) {
	add_wall_list(x, y + 1);
    }
}

void Pattern_maze::queue_maze(int x, int y)
{
#if 1
    m_queued_update = true;
    m_queued_x = x;
    m_queued_y = y;
#else
    add_maze(x, y, true);
#endif
}

bool Pattern_maze::expand()
{
    if (m_queued_update) {
	add_maze(m_queued_x, m_queued_y, true);
	m_queued_update = false;
	return true;
    }

    while (m_wall_list_count != 0) {
	int choice = int(random(m_wall_list_count));
	int index = m_wall_list[choice];
	remove_wall_list(choice);

	int x = 0;
	int y = 0;
	get_xy(index, x, y);

	int neighbours = 0;
	if (x > 0 && get_maze(x - 1, y) != WALL_VALUE) {
	    ++neighbours;
	}
	if (x < COLUMN_COUNT - 1 && get_maze(x + 1, y) != WALL_VALUE) {
	    ++neighbours;
	}
	if (y > 0 && get_maze(x, y - 1) != WALL_VALUE) {
	    ++neighbours;
	}
	if (y < ROW_COUNT - 1 && get_maze(x, y + 1) != WALL_VALUE) {
	    ++neighbours;
	}

	if (neighbours <= 1) {
	    add_maze(x, y, false);

	    if (x > 0 && get_maze(x - 1, y) != WALL_VALUE) {
		queue_maze(x + 1, y);
	    }
	    if (x < COLUMN_COUNT - 1 && get_maze(x + 1, y) != WALL_VALUE) {
		queue_maze(x - 1, y);
	    }
	    if (y > 0 && get_maze(x, y - 1) != WALL_VALUE) {
		queue_maze(x, y + 1);
	    }
	    if (y < ROW_COUNT - 1 && get_maze(x, y + 1) != WALL_VALUE) {
		queue_maze(x, y - 1);
	    }

	    return true;
	}
    }

    return false;
}
