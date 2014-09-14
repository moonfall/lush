#include "lush.h"

Direction g_up_direction = DIR_UP;

// top == UP
// 0,0 -> 0,0

// top == RIGHT
// 0,0 -> 7,0
// 1,0 -> 7,1

// top == LEFT
// 0,0 -> 0,7
// 1,0 -> 0,6

// top == BOTTOM
// 0,0 -> 7,7
// 1,0 -> 6,7

void set_up_direction(int up) 
{
    g_up_direction = (Direction) up;
}

int get_led_dir(Direction dir, int x, int y, int columns)
{
#if 1
    if (x < 0 || y < 0 || x > COLUMN_COUNT || y > ROW_COUNT) {
	Serial.printf("bad pixel %d,%d\n", x, y);
	return 0;
    }
#endif

    switch (dir) {
	case DIR_UP:
	default:
	    break;
	case DIR_RIGHT: {
	    int tmp = flip_y(y);
	    y = x;
	    x = tmp;
	    break;
	}
	case DIR_DOWN:
	    x = flip_x(x);
	    y = flip_y(y);
	    break;
	case DIR_LEFT: {
	    int tmp = flip_x(x);
	    x = y;
	    y = tmp;
	    break;
	}
    }

#ifdef FLIPPED_X_COORDS
    if (y % 2 == 0) {
	x = flip_x(x);
    }
#endif

    return y * columns + x;
}

void get_xy_dir(Direction dir, int led, int &x, int &y, int columns)
{
    x = led % columns;
    y = led / columns;

#ifdef FLIPPED_X_COORDS
    if (y % 2 == 0) {
	x = flip_x(x);
    }
#endif

    switch (dir) {
	case DIR_UP:
	default:
	    break;
	case DIR_RIGHT: {
	    int tmp = flip_x(x);
	    x = y;
	    y = tmp;
	    break;
	}
	case DIR_DOWN:
	    x = flip_x(x);
	    y = flip_y(y);
	    break;
	case DIR_LEFT: {
	    int tmp = flip_y(y);
	    y = x;
	    x = tmp;
	    break;
	}
    }
}

void blend_led(int led, Colour c)
{
    uint8_t a = (c >> 24);

    uint8_t fg_r;
    uint8_t fg_g;
    uint8_t fg_b;
    split_rgb(c, fg_r, fg_g, fg_b);

    uint8_t bg_r;
    uint8_t bg_g;
    uint8_t bg_b;
    split_rgb(get_led(led), bg_r, bg_g, bg_b);

    uint8_t r = (a * (unsigned) fg_r + (255 - a) * (unsigned) bg_r) >> 8;
    uint8_t g = (a * (unsigned) fg_g + (255 - a) * (unsigned) bg_g) >> 8;
    uint8_t b = (a * (unsigned) fg_b + (255 - a) * (unsigned) bg_b) >> 8;

    draw_led(led, make_rgb(r, g, b));
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
