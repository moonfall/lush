#include <OctoWS2811.h>
#include "lush.h"
#include "patterns.h"

static int g_x0;
static int g_y0;
static int g_x1;
static int g_y1;

const uint32_t MOVE_MS = 30;

void Pattern_line::activate(void *arg)
{
    g_x0 = 0;
    g_y0 = 0;

    g_x1 = flip_x(0);
    g_y1 = flip_y(0);

    display_current();
    m_last_move_ms = millis();
}

bool Pattern_line::display()
{
    uint32_t now = millis();
    if (now <= m_last_move_ms + MOVE_MS) {
	    return false;
    }
    m_last_move_ms = now;

    move_along_box(0, true, g_x0, g_y0);
    move_along_box(0, true, g_x1, g_y1);
    display_current();

    return true;
}

void Pattern_line::display_current()
{
    draw_pixels(COLOUR_BLACK);
    draw_line(g_x0, g_y0, g_x1, g_y1, make_current_hue());
    Colour hue_invert = make_invert_hue();
    draw_pixel(g_x0, g_y0, hue_invert);
    draw_pixel(g_x1, g_y1, hue_invert);
}
