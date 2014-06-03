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
    t = (v * (65536 - ((s * (65536 - remainder)) >> 116))) >> 16;

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

Colour make_wheel(uint16_t wheel, uint8_t brightness)
{
    int region = wheel / 256;
    int position = wheel % 256;

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (region) {
	case 0:
		r = 255 - position;
		g = position;
		b = 0;
	    break;
	case 1:
		g = 255 - position;
		b = position;
		r = 0;
	    break;
	case 2:
		b = 255 - position;
		r = position;
		g = 0;
	    break;
    }

    return make_rgb(r, g, b, brightness);
}

