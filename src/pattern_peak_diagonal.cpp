#include <Audio.h>
#include "lush.h"
#include "patterns.h"

bool Pattern_peak_diagonal::display()
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

    int leds = get_mapped_peak((COLUMN_COUNT + 1) * ROW_COUNT / 2);
    int start_x = 0;
    int x = start_x;
    int y = ROW_COUNT - 1;
    for (int led = 0; led < leds; ++led) {
	draw_pixel(x, y, c);	
	--x;
	--y;
	if (x < 0) {
	    ++start_x;
	    x = start_x;
	    y = ROW_COUNT - 1;
	}
	if (y < 0) {
	    break;
	}
    }

    return true;
}

