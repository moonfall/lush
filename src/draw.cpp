#include "lush.h"

void blend_pixel(int led, Colour c)
{
    uint8_t a = (c >> 24);

    uint8_t fg_r;
    uint8_t fg_g;
    uint8_t fg_b;
    split_rgb(c, fg_r, fg_g, fg_b);

    uint8_t bg_r;
    uint8_t bg_g;
    uint8_t bg_b;
    split_rgb(get_pixel(led), bg_r, bg_g, bg_b);

    uint8_t r = (a * (unsigned) fg_r + (255 - a) * (unsigned) bg_r) >> 8;
    uint8_t g = (a * (unsigned) fg_g + (255 - a) * (unsigned) bg_g) >> 8;
    uint8_t b = (a * (unsigned) fg_b + (255 - a) * (unsigned) bg_b) >> 8;

    g_octo.setPixel(led, make_rgb(r, g, b));
}

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

void draw_mask8(int x, int y, unsigned mask, Colour fg, const Colour *bg)
{
    for (int x_pos = 0; x_pos < 8; ++x_pos) {
	if (mask & (1 << (8 - x_pos - 1))) {
	    draw_pixel(x + x_pos, y, fg);
	} else if (bg) {
	    draw_pixel(x + x_pos, y, *bg);
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
