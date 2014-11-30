#include "lush.h"
#include "patterns.h"

const int HUE_OFFSET = 50;

Pattern_race::Pattern_race()
    : m_counter(0)
{
}

void Pattern_race::setup()
{
    m_counter.set_velocity(1, 50);
}

bool Pattern_race::display()
{
    draw_pixels(COLOUR_BLACK);

    int counter = m_counter.get() % LED_COUNT;
    Colour c = make_current_hue();

#if 0
    // Pixel order.
    draw_pixel(counter, c);
#else
    // Logical order.
    int x = counter % COLUMN_COUNT;
    int y = counter / ROW_COUNT;
    draw_pixel(x, y, c);
#endif

    return true;
}

