#include <Audio.h>
#include "lush.h"
#include "patterns.h"

Pattern_peak_ordered::Pattern_peak_ordered(Fader_static &fader)
    : m_fader(fader)
{
}

void Pattern_peak_ordered::activate(void *arg)
{
    m_fader.set_alternate_back_forth();
}

bool Pattern_peak_ordered::display()
{
#undef LOG_PEAKS
#ifdef LOG_PEAKS
    const int max_dots = 64;
    char dots[max_dots + 2];
    memset(dots, ' ', sizeof(dots));
    dots[max_dots] = '|';
    dots[max_dots + 1] = '\0';
    memset(dots, '.', get_mapped_peak(max_dots));
    Serial.printf("peak %5u %s\n", get_peak(), dots);
#endif

    // Clean slate.
    draw_pixels(COLOUR_BLACK);

    Colour c = make_hue(g_hue.get());

    int peak = get_mapped_peak(LED_COUNT);
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    if (m_fader.m_order[get_led(x, y)] < peak) {
		draw_pixel(x, y, c);	
	    }
	}
    }

    return true;
}

