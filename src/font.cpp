#include "lush.h"

#define FONT_4x5
#ifdef FONT_3x5
const char FONT_START = ' ';
const uint32_t FONT_DATA[] = {
    0x00000000, 0x44404000, // <space> !
    0xaa000000, 0xaeaea000, // " #
    0x6ce6c000, 0xa248a000, // $ %
    0x4ecae000, 0x44000000, // & '
    0x24442000, 0x42224000, // ( )
    0x0a4a0000, 0x04e40000, // * +
    0x00048000, 0x00e00000, // , -
    0x00008000, 0x22488000, // . /
    0x4aaa4000, 0xc444e000, // 0 1
    0xc248e000, 0xe242e000, // 2 3
    0xaae22000, 0xe8c2c000, // 4 5
    0x68eae000, 0xe2444000, // 6 7
    0xea4ae000, 0xeae2c000, // 8 9
    0x04040000, 0x04048000, // : ;
    0x24842000, 0x0e0e0000, // < =
    0x84248000, 0xc2404000, // > ?
    0x4ae86000, 0x4aeaa000, // @ A
    0xeacae000, 0xe888e000, // B C
    0xcaaac000, 0xe8c8e000, // D E
    0xe8c88000, 0x68aae000, // F G
    0xaaeaa000, 0xe444e000, // H I
    0x222ae000, 0xaacaa000, // J K
    0x8888e000, 0xaeeaa000, // L M
    0xcaaaa000, 0xeaaae000, // N O
    0xeae88000, 0xeaaee000, // P Q
    0xcacaa000, 0xe8e2e000, // R S
    0xe4444000, 0xaaaae000, // T U
    0xaaaa4000, 0xaaeea000, // V W
    0xaa4aa000, 0xaae44000, // X Y
    0xe248e000, 0x64446000, // Z [
    0x88422000, 0x62226000, // \ ]
    0x4a000000, 0x0000e000, // ^ _
    0x84000000, 0x00ea6000, // ` a
    0x08cac000, 0x00686000, // b c
    0x026a6000, 0x04ac6000, // d e
    0x02464000, 0x04a6c000, // f g
    0x08caa000, 0x04044000, // h i
    0x40448000, 0x08aca000, // j k
    0x04442000, 0x00eea000, // l m
    0x00caa000, 0x00eae000, // n o
    0x04ac8000, 0x04a62000, // p q
    0x00244000, 0x04848000, // r s
    0x04642000, 0x00aa6000, // t u
    0x00aa4000, 0x00aee000, // v w
    0x00a4a000, 0x00a6c000, // x y
    0x08484000, 0x64846000, // z {
    0x04444000, 0xc424c000, // | }
    0x02e80000, 0xeeeee000, // ~ <block>
};
#endif
#ifdef FONT_4x5
const char FONT_START = '0';
const uint32_t FONT_DATA[] = {
//  XX  6
// X  X 9
// X  X 9
// X  X 9
//  XX  6
    0x69996000,
//  XX  6
//   X  2
//   X  2
//   X  2
//   X  2
    0x62222000,
// XXX  e
//    X 1
//   X  2
//  X   4
// XXXX f
    0xe124f000,
// XXX  e
//    X 1
//  XX  6
//    X 1
// XXX  e
    0xe161e000,
// X  X 9
// X  X 9
// XXXX f
//    X 1
//    X 1
    0x99f11000,
// XXXX f
// X    8
// XXX  e
//    X 1
// XXX  e
    0xf8e1e000,
//  XX  6
// X    8
// XXX  e
// X  X 9
//  XX  6
    0x68e96000,
// XXXX f
//    X 1
//   X  2
//  X   4
// X    8
    0xf1248000,
//  XX  6
// X  X 9
//  XX  6
// X  X 9
//  XX  6
    0x69696000,
//  XX  6
// X  X 9
//  XXX 7
//    X 1
//  XX  6
    0x69716000,

    0x04040000, 0x04048000, // : ,
    0x24842000, 0x0e0e0000, // < =
    0x84248000, 0xc2404000, // > ?
    0x4ae86000, // @
//  XX  6
// X  X 9
// XXXX f
// X  X 9
// X  X 9
    0x69f99000,
// XXX  e
// X  X 9
// XXX  e
// X  X 9
// XXX  e
    0xe9e9e000,
//  XX  6
// X  X 9
// X    8
// X  X 9
//  XX  6
    0x69896000,
// XXX  e
// X  X 9
// X  X 9
// X  X 9
// XXX  e
    0xe999e000,
// XXXX f
// X    8
// XXX  e
// X    8
// XXXX f
    0xf8e8f000,
// XXXX f
// X    8
// XXX  e
// X    8
// X    8
    0xf8e88000,
//  XX  6
// X    8
// X XX b
// X  X 9
//  XX  6
    0x68b96000,
// X  X 9
// X  X 9
// XXXX f
// X  X 9
// X  X 9
    0x99f99000,
// XXXX f
//  XX  6
//  XX  6
//  XX  6
// XXXX f
    0xf666f000,
//    X 1
//    X 1
//    X 1
// X  X 9
//  XX  6
    0x11196000,
// X  X 9
// X X  a
// XX   c
// X X  a
// X  X 9
    0x9aca9000,
// X    8
// X    8
// X    8
// X    8
// XXXX f
    0x8888f000,
// X  X 9
// XXXX f
// XXXX f
// X  X 9
// X  X 9
    0x9ff99000,
// X  X 9
// XX X d
// XXXX f
// X XX b
// X  X 9
    0x9dfb9000,
//  XX  6
// X  X 9
// X  X 9
// X  X 9
//  XX  6
    0x69996000,
// XXX  e
// X  X 9
// XXX  e
// X    8
// X    8
    0xe9e88000,
//  XX  6
// X  X 9
// X  X 9
// X XX b
//  XXX 7
    0x699b7000,
// XXX  e
// X  X 9
// XXX  e
// X  X 9
// X  X 9
    0xe9e99000,
//  XXX 7
// X    8
//  XX  6
//    X 1
// XXX  e
    0x7861e000,
// XXXX f
//  XX  6
//  XX  6
//  XX  6
//  XX  6
    0xf6666000,
// X  X 9
// X  X 9
// X  X 9
// X  X 9
// XXXX f
    0x9999f000,
// X  X 9
// X  X 9
// X  X 9
// X  X 9
//  XX  6
    0x99996000,
// X  X 9
// X  X 9
// XXXX f
// XXXX f
// X  X 9
    0x99ff9000,
// X  X 9
// X  X 9
//  XX  6
// X  X 9
// X  X 9
    0x99699000,
// X  X 9
// X  X 9
// X  X 9
//  XX  6
//  XX  6
    0x99966000,
// XXXX f
//   XX 3
//  XX  6
// XX   c
// XXXX f
    0xf36cf000,
};
#endif

const int FONT_COUNT = sizeof(FONT_DATA) / sizeof(FONT_DATA[0]);
const char FONT_END = FONT_START + FONT_COUNT;

void draw_char(int x, int y, char c, Colour fg, const Colour *bg)
{
    // Each character is 4x5 stored as a word, each nibble represents
    // a single row with a set bit meaning the character has that pixel set.
    if (c < FONT_START || c >= FONT_END) {
	c = FONT_END - 1;
    }
    uint32_t bits = FONT_DATA[c - FONT_START];

    if (x + FONT_WIDTH <= 0 || x > COLUMN_COUNT ||
	y + FONT_HEIGHT <= 0 || y > ROW_COUNT) {
	// Skip entire clipped characters.
	return; 
    }

    for (int y_pos = y;
	 y_pos < y + FONT_HEIGHT && y_pos < ROW_COUNT; ++y_pos) {
	if (y_pos < 0) {
	    continue;
	}
	// Find the correct line.
	int line = (bits >> ((32 - FONT_WIDTH) - FONT_WIDTH * (y_pos - y))) &
	           0xf;
	int mask = 0x8;
	for (int x_pos = x; x_pos < x + FONT_WIDTH && x_pos < COLUMN_COUNT;
	     ++x_pos, mask >>= 1) {
	    if (x_pos < 0) {
		continue;
	    }
	    if (line & mask) {
		draw_pixel(x_pos, y_pos, fg);
	    } else if (bg) {
		draw_pixel(x_pos, y_pos, *bg);
	    }
	}
    }
}

void draw_string(int x, int y, const char *s, Colour fg, const Colour *bg, 
		 Direction dir, unsigned spacing)
{
    while (*s) {
	draw_char(x, y, *s, fg, bg);
	switch (dir) {
	    case DIR_LEFT:
		x -= (FONT_WIDTH + spacing);
		break;
	    case DIR_RIGHT:
		x += (FONT_WIDTH + spacing);
		break;
	    case DIR_UP:
		y -= (FONT_HEIGHT + spacing);
		break;
	    case DIR_DOWN:
		y += (FONT_HEIGHT + spacing);
		break;
	}
	++s;
    }
}

void draw_centered_string(int x, int y, int width, int height,
			  const char *s, Colour fg, const Colour *bg,
			  Direction dir, unsigned spacing)
{
    unsigned length = strlen(s);
    bool horiz = (dir == DIR_LEFT) || (dir == DIR_RIGHT);
    int x_length = horiz ? length : 1;
    int y_length = !horiz ? length : 1;
    int x_required = FONT_WIDTH * x_length + spacing * (x_length - 1);
    int y_required = FONT_HEIGHT * y_length + spacing * (y_length - 1);

    int x_offset = (width - x_required) / 2;
    int y_offset = (height - y_required) / 2;

    x += x_offset;
    y += y_offset;
    draw_string(x, y, s, fg, bg, dir, spacing);
}

