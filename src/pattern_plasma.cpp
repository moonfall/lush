#include <OctoWS2811.h>
#include "lush.h"

#define dist(a, b, c, d) sqrt(double((a - c) * (a - c) + (b - d) * (b - d)))

const int MAX_LAYERS = 16;
double g_layer_alpha[MAX_LAYERS];

Pattern_plasma::Pattern_plasma()
    : m_time(0), m_palette(0)
{
    for (int i = 0; i < MAX_LAYERS; ++i) {
	g_layer_alpha[i] = 1.0;
    }
}

void Pattern_plasma::activate(void *arg)
{
    ++m_palette;
}

bool Pattern_plasma::display()
{
    m_time.modify(1);

    int time = m_time.get();

    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    reset_plasma();

#if 0
	    layer(sin(x / 16.0));
	    layer(sin(y / 32.0));
	    layer(sin(dist(x, y, COLUMN_COUNT / 2.0, ROW_COUNT / 2.0) / 8.0));
	    layer(sin(dist(x, y, 0, 0) / 8.0));
#endif

#if 0
	    layer(sin(dist(x + time, y, 128.0, 128.0) / 8.0));
	    layer(sin(dist(x, y, 64.0, 64.0) / 8.0));
	    layer(sin(dist(x, y + time / 7, 192.0, 64) / 7.0));
	    layer(sin(dist(x, y, 192.0, 100.0) / 8.0));
#endif

#if 1
	    layer(sin(dist(x + time, y, 64.0, 64.0) / 4.0));
	    layer(sin(dist(x, y, 32.0, 32.0) / 4.0));
	    layer(sin(dist(x, y + time / 7, 96.0, 32) / 3.5));
	    layer(sin(dist(x, y, 96.0, 50.0) / 4.0));
#endif

	    double value = plasma();

#if 0
	    // int colour = (value + 1.0) * 768.0 + time;
	    int colour = (value + 1.0) * 768.0;
	    draw_pixel(x, y, make_wheel(colour, g_brightness.get()));
#endif
	    // int colour = (value + 1.0) * 768.0 + time;
	    int colour = (value + 1.0) * 128.0;
	    draw_pixel(x, y, make_palette(m_palette, colour,
					  g_brightness.get()));
	}
    }

    return true;
}

void Pattern_plasma::reset_plasma()
{
    m_plasma_sum = 0.0;
    m_plasma_count = 0.0;
} 

void Pattern_plasma::layer(double value)
{
    m_plasma_sum += value;
    m_plasma_count += 1.0;
}

double Pattern_plasma::plasma()
{
    return m_plasma_sum / m_plasma_count;
} 
