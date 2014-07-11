#include <OctoWS2811.h>
#include "lush.h"

const uint8_t WALL_VALUE = 0;

const int RESET_MS = 2000;
const int UPDATE_MS = 100;
const int HUE_OFFSET = 3;

Pattern_maze::Pattern_maze(Fader_static &fader)
    : m_fader(fader)
{
}

void Pattern_maze::activate()
{
    regenerate();
}

void Pattern_maze::regenerate()
{
    randomize_maze();
    if (random() % 2) {
	randomize_image();
    } else {
	ordered_image();
    }
}

void Pattern_maze::randomize_maze()
{
    m_maze.reset();
    m_maze.complete();
    for (int led = 0; led < LED_COUNT; ++led) {
	int order = m_maze.get_order(led);
	if (order) { 
	    m_fader.m_order[led] = order - 1;
	} else {
#if 0
	    m_fader.m_order[led] = m_maze.m_count;
#else
	    m_fader.m_order[led] = LED_COUNT - 1;
#endif
	}
    }
    for (int order = 0; order < LED_COUNT; ++order) {
	m_fader.m_fade_start[order] = order * 100;
	m_fader.m_fade_duration[order] = 100;
    }

    m_start_time = millis();
}

void Pattern_maze::randomize_image(bool initial)
{
    int start = millis();
    int x_offset = random(1, 10);
    int y_offset = random(10, 100);
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int wheel = start + x * x_offset + y * y_offset;
	    int order = m_maze.get_order(get_led(x, y));
	    m_fader.set_colour(initial, get_led(x, y),
			       order ?
			       make_wheel(wheel, g_brightness.get()) :
			       COLOUR_BLACK);
	}
    }
    m_fader.set_colours(!initial, COLOUR_BLACK);
}

void Pattern_maze::ordered_image(bool initial)
{
    int start = millis();
    for (int led = 0; led < LED_COUNT; ++led) {
	const int offset = random(1, 5);
	int order = m_maze.get_order(led);
	m_fader.set_colour(initial, led,
			   order ?
			   make_wheel(start + offset * order,
				      g_brightness.get()) :
			   COLOUR_BLACK);
    }
    m_fader.set_colours(!initial, COLOUR_BLACK);
}

bool Pattern_maze::display()
{
    int now = millis();
    int t = now - m_start_time;

    bool all_done = true;
    for (int led = 0; led < LED_COUNT; ++led) {
	bool done = false;
	draw_pixel(led, m_fader.generate(led, t, &done));
	if (!done) {
	    all_done = false;
	}
    }

    if (all_done) {
	regenerate(); 
    }

    return true;
#if 0
    int now = millis();
    if (m_finished_ms) {
	if (now > m_finished_ms + RESET_MS) {
	    m_maze.reset();
	    m_update_ms = now;
	    m_finished_ms = 0;
	}
    } else if (now > m_update_ms + UPDATE_MS) {
	// Try to expand and record what happened.
	if (m_maze.expand()) {
	    m_update_ms = now;
	} else {
	    m_finished_ms = now;
	}
    }

    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int index = get_led(x, y);
	    int order = m_maze.get_order(index);
	    int hue = g_hue.get();
	    Colour c = COLOUR_BLACK;
	    if (order) {
		c = make_hue(hue + HUE_OFFSET * order);
	    }
#ifdef SHOW_POSSIBILITES
	    if (m_maze.in_wall_list(index)) {
		c = make_hue(hue + 50);
	    }
#endif
	    draw_pixel(index, c);
	}  
    }

    return true;
#endif
}
