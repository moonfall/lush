#include "lush.h"
#include "patterns.h"

void Pattern_off::activate(void *arg)
{
    draw_pixels(COLOUR_BLACK);
}

bool Pattern_off::display()
{
    return false;
}

