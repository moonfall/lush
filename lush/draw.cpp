#include <OctoWS2811.h>
#include "lush.h"

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
