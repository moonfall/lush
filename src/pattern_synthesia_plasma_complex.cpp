#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

#define dist(a, b, c, d) sqrt(double((a - c) * (a - c) + (b - d) * (b - d)))

Pattern_synthesia_plasma_complex::Pattern_synthesia_plasma_complex()
	: m_time(0, 0, 50, true)
{
}

void Pattern_synthesia_plasma_complex::setup()
{
}

bool Pattern_synthesia_plasma_complex::display()
{
    m_time.modify(1);

    for (int x = 0; x < COLUMN_COUNT; ++x) {
	for (int y = 0; y < COLUMN_COUNT; ++y) {
	    double value = sin(dist(x + m_time.get(), y, 64.0, 64.0) / 4.0) +
			   sin(dist(x, y, 32.0, 32.0) / 4.0) +
			   sin(dist(x, y + m_time.get() / 7, 95.0, 32) / 3.5) +
			   sin(dist(x, y, 95.0, 50.0) / 4.0);

	    int color = int((4 + value) * 768) % 768;
	    draw_pixel(x, y, make_wheel(color, g_brightness.get()));
	}
    }

    return true;
}

