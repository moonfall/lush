#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

#define dist(a, b, c, d) sqrt(double((a - c) * (a - c) + (b - d) * (b - d)))

Pattern_synthesia_fire::Pattern_synthesia_fire()
{
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	for (int y = 0; y < ROW_COUNT; ++y) {
	    m_matrix_layer1[x][y] = 0;
	    m_matrix_layer2[x][y] = 0;
	}
    }

}

void Pattern_synthesia_fire::setup()
{
}

bool Pattern_synthesia_fire::display()
{
#if 1
    for (int x = 0; x < COLUMN_COUNT; ++x) {
	m_matrix_layer1[x][0] = random(-20, 127);
	m_matrix_layer1[x][1] = random(-50, 127);
    }
#endif

#if 1
    for (int y = 0; y < ROW_COUNT - 1; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int left = x > 0 ? m_matrix_layer1[x - 1][y] : 0;
	    int right = x < COLUMN_COUNT - 1 ? m_matrix_layer1[x + 1][y] : 0;
	    int down = y > 0 ? m_matrix_layer1[x][y - 1] : 0;
	    int up = y < ROW_COUNT - 1 ? m_matrix_layer1[x][y + 1] : 0;
	    int newPoint = (left + right + down + up) / 4 - 10;
	    if (newPoint > 255) {
		newPoint = 255;
	    }
	    m_matrix_layer2[x][y + 1] = newPoint;
	}
    }
#endif

    for (int x = 0; x < COLUMN_COUNT; ++x) {
	for (int y = 0; y < ROW_COUNT; ++y) {
	    int value = m_matrix_layer1[x][y];
	    if (value < 0) {
		value = 0;
	    }
	    value = (unsigned) gamma(value);
	    value /= 4;
	    Serial.print(value);
	    Serial.print(" ");
	    draw_pixel(x, y, make_rgb(value, 0, 0));
	    m_matrix_layer1[x][y] = m_matrix_layer2[x][y];
	}
	Serial.println();
    }

    return true;
}

const unsigned char GAMMA_TABLE[]  = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
   11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
   16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
   23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
   30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
   40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
   50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
   62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
   76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
   92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};

unsigned char gamma(unsigned char x) {
    return GAMMA_TABLE[x];
}
