#include <OctoWS2811.h>
#include <Audio.h>
#include "lush.h"

extern AudioPeak g_peak;

void Pattern_peak::activate()
{
}

int last_display = 0;
int REFRESH_HZ = 30;
int REFRESH_MS = 1000 / REFRESH_HZ;

#define OCTOWS2811_HACK
#ifdef OCTOWS2811_HACK
int updates_to_ignore = 0;
#endif

// Maps from NOISE_FLOOR..MAX_VALUE -> 0..mapped_max
static int map_value(int value, int mapped_max)
{
    const int MAX_VALUE = 65535;
    const int NOISE_FLOOR = 19000;

    return (value - NOISE_FLOOR) * (mapped_max + 1) / (MAX_VALUE - NOISE_FLOOR);
}

bool Pattern_peak::display()
{
    int now = millis();

#ifdef OCTOWS2811_HACK
    // OctoWS2811 seems to mess up audio acquisition.  Skip the next two
    // audio updates to work around it.
    if (g_octo.busy()) {
	updates_to_ignore = 2;
	return false;
    }
    if (updates_to_ignore > 0 && g_peak.update_completed_at) {
	--updates_to_ignore;
	g_peak.begin();
    }
#endif

    // Maintain a reasonable refresh rate.
    if (now < last_display + REFRESH_MS) {
	// Nothing to do.
	return false;
    }
    last_display = now;

    uint16_t peak = g_peak.Dpp();
    g_peak.begin();

#define LOG_PEAKS
#ifdef LOG_PEAKS
    Serial.print("peak ");
    Serial.print(peak);
    const int max_dots = 64;
    char dots[max_dots + 2];
    memset(dots, ' ', sizeof(dots));
    dots[max_dots] = '|';
    dots[max_dots + 1] = '\0';
    memset(dots, '.', map_value(peak, max_dots));
    Serial.print(" ");
    Serial.println(dots);
#endif

    // Clean slate.
    draw_pixels(COLOUR_BLACK);

    int leds = map_value(peak, LED_COUNT);
    Colour c = make_hue(g_hue.get());

#if 0
    leds = map_value(peak, (COLUMN_COUNT + 1) * ROW_COUNT / 2);
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
#endif
#if 1
    leds = map_value(peak, LED_COUNT / 2);

    int start_x = COLUMN_COUNT / 2;
    int start_y = ROW_COUNT - 1;
    int x = start_x;
    int y = start_y;
    for (int led = 0; led < leds; ++led) {
	draw_pixel(x, y, c);
	draw_pixel(flip_x(x), y, c);
	++x;
	++y;
	if (x >= COLUMN_COUNT || y >= ROW_COUNT) {
	    --start_y;
	    if (start_y < 0) {
		start_y = 0;
		++start_x;
	    }
	    x = start_x;
	    y = start_y;
	}
    }
#endif

    return true;
}

