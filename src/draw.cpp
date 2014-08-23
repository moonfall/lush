#include <OctoWS2811.h>
#include "lush.h"

void move_along_box(int origin, bool clockwise, int &x, int &y)
{
    if (clockwise) {
	if (y == origin && x < flip_x(origin)) {
	    ++x;
	} else if (x == flip_x(origin) && y < flip_y(origin)) {
	    ++y;
	} else if (y == flip_y(origin) && x > origin) {
	    --x;
	} else {
	    --y;
	}
    } else {
	if (x == origin && y < flip_y(origin)) {
	    ++y;
	} else if (y == flip_y(origin) && x < flip_x(origin)) {
	    ++x;
	} else if (x == flip_x(origin) && y > origin) {
	    --y;
	} else {
	    --x;
	}
    }
}

void draw_mask(int y, unsigned mask, Colour fg, const Colour *bg)
{
    for (int x_pos = 0; x_pos < COLUMN_COUNT; ++x_pos) {
	if (mask & (1 << (COLUMN_COUNT - x_pos - 1))) {
	    draw_pixel(x_pos, y, fg);
	} else if (bg) {
	    draw_pixel(x_pos, y, *bg);
	}
    }
}

void draw_line(int x0, int y0, int x1, int y1, Colour c)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    
    draw_pixel(x0, y0, c);
    int x = x0;
    int y = y0;
    draw_pixel(x, y, c);
    while (x != x1 || y != y1) {
	int e2 = err;
	if (e2 > -dx) {
	    err -= dy;
	    x += sx;
	}
	if (e2 < dy) {
	    err += dx;
	    y += sy;
	}
	draw_pixel(x, y, c);
    }
}
