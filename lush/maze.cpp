#include <OctoWS2811.h>
#include "lush.h"

Maze::Maze()
{
    reset();
}

void Maze::complete()
{
    // Expand until done.
    while (expand()) {
    }
}

bool Maze::expand()
{
    while (m_wall_list_count != 0) {
	int choice = int(random(m_wall_list_count));
	int index = m_wall_list[choice];
	remove_wall_list(choice);

	int x = 0;
	int y = 0;
	get_xy(index, x, y);

	int neighbours = 0;
	if (get_maze(DIR_LEFT, x, y) != WALL_VALUE) {
	    ++neighbours;
	}
	if (get_maze(DIR_RIGHT, x, y) != WALL_VALUE) {
	    ++neighbours;
	}
	if (get_maze(DIR_UP, x, y) != WALL_VALUE) {
	    ++neighbours;
	}
	if (get_maze(DIR_DOWN, x, y) != WALL_VALUE) {
	    ++neighbours;
	}

	if (neighbours <= 1) {
	    add_maze(x, y, false);

	    if (get_maze(DIR_LEFT, x, y) != WALL_VALUE) {
		add_maze(x + 1, y, true);
	    }
	    if (get_maze(DIR_RIGHT, x, y) != WALL_VALUE) {
		add_maze(x - 1, y, true);
	    }
	    if (get_maze(DIR_UP, x, y) != WALL_VALUE) {
		add_maze(x, y + 1, true);
	    }
	    if (get_maze(DIR_DOWN, x, y) != WALL_VALUE) {
		add_maze(x, y - 1, true);
	    }

	    return true;
	}
    }

    return false;
}

void Maze::reset()
{
    memset(m_maze, 0, sizeof(m_maze));
    m_count = 0;

    m_wall_list_count = 0;

    add_maze(random(COLUMN_COUNT), random(ROW_COUNT), true);
}

void Maze::add_maze(int x, int y, bool add_to_list)
{
    if (!in_bounds(x, y)) {
	return;
    }

    int index = get_led(x, y);
    ++m_count;
    m_maze[index] = m_count;

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

void Maze::add_wall_list(int x, int y)
{
    int index = get_led(x, y);
    if (m_maze[index] != WALL_VALUE) {
	return;
    }

    m_wall_list[m_wall_list_count] = index;
    ++m_wall_list_count;
}

void Maze::remove_wall_list(int choice)
{
    if (m_wall_list_count > 0) {
	--m_wall_list_count;
	m_wall_list[choice] = m_wall_list[m_wall_list_count];
    }
}

bool Maze::in_wall_list(int index)
{
    for (int i = 0; i < m_wall_list_count; ++i) {
	if (m_wall_list[i] == index) {
	    return true;
	}
    }

    return false;
}

