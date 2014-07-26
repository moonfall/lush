#include <OctoWS2811.h>

#include "lush.h"

Colour make_hsv(uint8_t h, uint8_t s, uint8_t v)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    unsigned char region, remainder, p, q, t;

    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
        return make_rgb(r, g, b);
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }

    return make_rgb(r, g, b);
}

Colour make_hsv16(uint16_t h, uint16_t s, uint16_t v)
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t region, remainder, p, q, t;

    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
        return make_rgb(r >> 8, g >> 8, b >> 8);
    }

#if 0
    region = h / 43;
    remainder = (h - (region * 43)) * 6; 
#endif
    // 65535 / 6 = 10923
    region = h / 10923;
    remainder = (h - (region * 10923)) * 6; 

    p = (v * (65536 - s)) >> 16;
    q = (v * (65536 - ((s * remainder) >> 16))) >> 16;
    t = (v * (65536 - ((s * (65536 - remainder)) >> 16))) >> 16;

    switch (region)
    {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
	    break;
        default:
            r = v; g = p; b = q;
            break;
    }

    return make_rgb(r >> 8, g >> 8, b >> 8);
}

template<int REGION_SIZE>
void set_wheel_region(uint8_t position, uint8_t &a, uint8_t &b, uint8_t &c) 
{
    a = (REGION_SIZE - 1) - position;
    b = position;
    c = 0;
}

template<int REGION_SIZE>
void set_wheel_colour(uint8_t region, uint8_t position,
	       uint8_t &r, uint8_t &g, uint8_t &b) 
{
    switch (region) {
	// R->G
	case 0:
	    set_wheel_region<REGION_SIZE>(position, r, g, b);
	    break;
	// G->B
	case 1:
	    set_wheel_region<REGION_SIZE>(position, g, b, r);
	    break;
	// B->R
	case 2:
	    set_wheel_region<REGION_SIZE>(position, b, r, g);
	    break;
	// R->B
	case 3:
	    set_wheel_region<REGION_SIZE>(position, r, b, g);
	    break;
	// B->G
	case 4:
	    set_wheel_region<REGION_SIZE>(position, b, g, r);
	    break;
	// G->R
	case 5:
	    set_wheel_region<REGION_SIZE>(position, g, r, b);
	    break;
    }
}

template<int REGION_SIZE>
Colour make_wheel(uint16_t wheel, uint8_t brightness)
{
    int region = (wheel / REGION_SIZE) % 6;
    int position = wheel % REGION_SIZE;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    set_wheel_colour<REGION_SIZE>(region, position, r, g, b);

    // If not full scale wheel, scale up each component.
    const int SCALE = 256 / REGION_SIZE;
    return make_rgb(r * SCALE, g * SCALE, b * SCALE, brightness);
}

template<int REGION_SIZE>
Colour make_reverse_wheel(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / REGION_SIZE;
    switch (region) {
	case 0:
	    region = 2;
	    break;
	case 2:
	    region = 0;
	    break;
	case 3:
	    region = 5;
	    break;
	case 5:
	    region = 3;
	    break;
    }
    int position = wheel % REGION_SIZE;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    set_wheel_colour<REGION_SIZE>(region, position, r, g, b);

    // If not full scale wheel, scale up each component.
    const int SCALE = 256 / REGION_SIZE;
    return make_rgb(r * SCALE, g * SCALE, b * SCALE, brightness);
}

Colour make_wheel(uint16_t wheel, uint8_t brightness)
{
    return make_wheel<256>(wheel, brightness);
}

Colour make_reverse_wheel(uint16_t wheel, uint8_t brightness)
{
    return make_reverse_wheel<256>(wheel, brightness);
}

Colour make_wheel7(uint16_t wheel, uint8_t brightness)
{
    return make_wheel<128>(wheel, brightness);
}

Colour make_reverse_wheel7(uint16_t wheel, uint8_t brightness)
{
    return make_reverse_wheel<128>(wheel, brightness);
}

#if 0
void set_wheel384(uint8_t position, uint8_t &a, uint8_t &b, uint8_t &c)
{
    a = 127 - position;
    b = position;
    c = 0;
}

Colour make_wheel384(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 128;
    int position = wheel % 128;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
	    set_wheel384(position, r, g, b);
	    break;
	case 1:
	    set_wheel384(position, g, b, r);
	    break;
	case 2:
	    set_wheel384(position, b, r, g);
	    break;
    }

    return make_rgb(r * 2, g * 2, b * 2, brightness);
}

void set_wheel768(uint8_t position, uint8_t &a, uint8_t &b, uint8_t &c)
{
    a = 255 - position;
    b = position;
    c = 0;
}

Colour make_wheel768(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 256;
    int position = wheel % 256;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
	    set_wheel768(position, r, g, b);
	    break;
	case 1:
	    set_wheel768(position, g, b, r);
	    break;
	case 2:
	    set_wheel768(position, b, r, g);
	    break;
    }

    return make_rgb(r, g, b, brightness);
}

Colour make_inverse_wheel384(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 128;
    int position = wheel % 128;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
	    set_wheel384(position, b, r, g);
	    break;
	case 1:
	    set_wheel384(position, g, b, r);
	    break;
	case 2:
	    set_wheel384(position, r, g, b);
	    break;
    }

    return make_rgb(r * 2, g * 2, b * 2, brightness);
}

Colour make_inverse_wheel768(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 256;
    int position = wheel % 256;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
	    set_wheel768(position, b, r, g);
	    break;
	case 1:
	    set_wheel768(position, g, b, r);
	    break;
	case 2:
	    set_wheel768(position, r, g, b);
	    break;
    }

    return make_rgb(r, g, b, brightness);
}
#endif

#if 0
// This doesn't quite make sense...
Colour make_bg_wheel384(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 128;
    int position = wheel % 128;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
	    r = 0;
	    g = position;
	    b = 0;
	    break;
	case 1:
#if 1
	    set_wheel384(position, g, b, r);
#else
	    g = 127 - position;
	    b = position;
	    r = 0;
#endif
	    break;
	case 2:
#if 1
	    set_wheel384(position, b, g, r);
#else
	    b = 127 - position;
	    r = 0;
	    g = position;
#endif
	    break;
    }

    return make_rgb(r * 2, g * 2, b * 2, brightness);
}
#endif

Colour make_palette(uint16_t palette, uint16_t index, uint8_t brightness)
{
    const int PALETTE_COUNT = 
#ifdef SOLID_COLOURS
	3 +
#endif
	6 + 6;
    palette %= PALETTE_COUNT;

#ifdef SOLID_COLOURS
    // Solid colour of varying brightness.
    if (palette < 3) {
	case 0:
	    return make_rgb(index, 0, 0, brightness);
	case 1:
	    return make_rgb(0, index, 0, brightness);
	case 2:
	    return make_rgb(0, 0, index, brightness);
    }
    palette -= 3;
#endif

    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    if (palette < 6) {
	a = uint8_t(127.5 + 127.5 * sin(M_PI * index / 128.0));
	b = uint8_t(127.5 + 127.5 * sin(M_PI * index / 16.0));
	c = 0;
    } else if (palette < 12) {
	set_wheel_region<256>(index, a, b, c);
    }

    switch (palette % 6) {
	case 0:
	    return make_rgb(a, b, c, brightness);
	case 1:
	    return make_rgb(a, c, b, brightness);
	case 2:
	    return make_rgb(b, a, c, brightness);
	case 3:
	    return make_rgb(b, c, a, brightness);
	case 4:
	    return make_rgb(c, a, b, brightness);
	case 5:
	    return make_rgb(c, b, a, brightness);
    }

    return COLOUR_BLACK;
}
