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

void draw_char(int x, int y, char c, Colour fg, const Colour *bg)
{
  // Each character is 4x5 stored as a word, each nibble represents
  // a single row with a set bit meaning the character has that pixel set.
  int bits;
  switch (c) {
    case '0':
      bits = 0x69996000;
      break;
    case '1':
      bits = 0x62222000;
      break;
    case '2':
      bits = 0xe124f000;
      break;
    case '3':
      bits = 0xe161e000;
      break;
    case '4':
      bits = 0x99f11000;
      break;
    case '5':
      bits = 0xf8e1e000;
      break;
    case '6':
      bits = 0x68e96000;
      break;
    case '7':
      bits = 0xf1248000;
      break;
    case '8':
      bits = 0x69696000;
      break;
    case '9':
      bits = 0x69716000;
      break;
//  XX  6
// X  X 9
// XXXX f
// X  X 9
// X  X 9
    case 'A':
      bits = 0x69f99000;
      break;
// XXX  e
// X  X 9
// XXX  e
// X  X 9
// XXX  e
    case 'B':
      bits = 0xe9e9e000;
      break;
//
//  XX  6
// X  X 9
// X    8
// X  X 9
//  XX  6
    case 'C':
      bits = 0x69896000;
      break;
//
// XXX  e
// X  X 9
// X  X 9
// X  X 9
// XXX  e
    case 'D':
      bits = 0xe999e000;
      break;
//
// XXXX f
// X    8
// XXX  e
// X    8
// XXXX f
    case 'E':
      bits = 0xf8e8f000;
      break;
// 
// XXXX f
// X    8
// XXX  e
// X    8
// X    8
    case 'F':
      bits = 0xf8e88000;
      break;
//
//  XX  6
// X    8
// X XX b
// X  X 9
//  XX  6
    case 'G':
      bits = 0x68b96000;
      break;
//
// X  X 9
// X  X 9
// XXXX f
// X  X 9
// X  X 9
    case 'H':
      bits = 0x99f99000;
      break;
//
// XXXX f
//  XX  6
//  XX  6
//  XX  6
// XXXX f
    case 'I':
      bits = 0xf666f000;
      break;
//
//    X 1
//    X 1
//    X 1
// X  X 9
//  XX  6
    case 'J':
      bits = 0x11196000;
      break;
//
// X  X 9
// X X  a
// XX   c
// X X  a
// X  X 9
    case 'K':
      bits = 0x9aca9000;
      break;
//
// X    8
// X    8
// X    8
// X    8
// XXXX f
    case 'L':
      bits = 0x8888f000;
      break;
//
// X  X 9
// XXXX f
// XXXX f
// X  X 9
// X  X 9
    case 'M':
      bits = 0x9ff99000;
      break;
//
// X  X 9
// XX X d
// XXXX f
// X XX b
// X  X 9
    case 'N':
      bits = 0x9dfb9000;
      break;
//
//  XX  6
// X  X 9
// X  X 9
// X  X 9
//  XX  6
    case 'O':
      bits = 0x69996000;
      break;
// 
// XXX  e
// X  X 9
// XXX  e
// X    8
// X    8
    case 'P':
      bits = 0xe9e88000;
      break;
//
//  XX  6
// X  X 9
// X  X 9
// X XX b
//  XXX 7
    case 'Q':
      bits = 0x699b7000;
      break;
//
// XXX  e
// X  X 9
// XXX  e
// X  X 9
// X  X 9
    case 'R':
      bits = 0xe9e99000;
      break;
//
//  XXX 7
// X    8
//  XX  6
//    X 1
// XXX  e
    case 'S':
      bits = 0x7861e000;
      break;
//
// XXXX f
//  XX  6
//  XX  6
//  XX  6
//  XX  6
    case 'T':
      bits = 0xf6666000;
      break;
//
// X  X 9
// X  X 9
// X  X 9
// X  X 9
// XXXX f
    case 'U':
      bits = 0x9999f000;
      break;
//
// X  X 9
// X  X 9
// X  X 9
// X  X 9
//  XX  6
    case 'V':
      bits = 0x99996000;
      break;
//
// X  X 9
// X  X 9
// XXXX f
// XXXX f
// X  X 9
    case 'W':
      bits = 0x99ff9000;
      break;
//
// X  X 9
// X  X 9
//  XX  6
// X  X 9
// X  X 9
    case 'X':
      bits = 0x99699000;
      break;
//
// X  X 9
// X  X 9
// X  X 9
//  XX  6
//  XX  6
    case 'Y':
      bits = 0x99966000;
      break;
//
// XXXX f
//   XX 3
//  XX  6
// XX   c
// XXXX f
    case 'Z':
      bits = 0xf36cf000;
      break;
    default:
      bits = 0x0;
  }

  for (int y_pos = y; y_pos < y + FONT_HEIGHT && y_pos < ROW_COUNT; ++y_pos) {
    // Find the correct line.
    int line = (bits >> ((32 - FONT_WIDTH) - FONT_WIDTH * (y_pos - y))) & 0xf;
    int mask = 0x8;
    for (int x_pos = x; x_pos < x + FONT_WIDTH && x_pos < COLUMN_COUNT;
	 ++x_pos, mask >>= 1) {
	if (line & mask) {
	    draw_pixel(x_pos, y_pos, fg);
	} else if (bg) {
	    draw_pixel(x_pos, y_pos, *bg);
	}
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
