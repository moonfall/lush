#ifndef _MAZE_H_INCLUDED
#define _MAZE_H_INCLUDED

class Maze
{
  public:
    const uint8_t WALL_VALUE = 0;

    Maze();

    void reset();

    void complete();
    // Attempts to expand the maze.  Returns true if it was successfully,
    // expanded.
    bool expand();

    uint8_t get_order(int index)
    {
	return m_maze[index];
    }

    bool in_bounds(int x, int y)
    {
	return x >= 0 && x < COLUMN_COUNT && y >= 0 && y < ROW_COUNT;
    }

    void add_maze(int x, int y, bool add_to_list);
    void queue_maze(int x, int y);

    void add_wall_list(int x, int y);
    void remove_wall_list(int choice);
    bool in_wall_list(int index);

    uint8_t get_maze(int x, int y)
    {
	return in_bounds(x, y) ? m_maze[get_led(x, y)] : WALL_VALUE;
    }

    uint8_t get_maze(Direction dir, int x, int y)
    {
	make_neighbour(dir, x, y);
	return get_maze(x, y);
    }

    int m_update_ms;
    int m_finished_ms;

    // TODO: Shrink down representation
    // TODO: Use shared scratch space
    uint8_t m_maze[LED_COUNT];
    unsigned m_count;
    uint8_t m_wall_list[LED_COUNT];
    int m_wall_list_count;
};

#endif // !_MAZE_H_INCLUDED
