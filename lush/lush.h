#ifndef _LUSH_H_INCLUDED
#define _LUSH_H_INCLUDED

#include <stdint.h>

extern const int ROW_COUNT;
extern const int COLUMN_COUNT;
extern const int LED_COUNT;
extern OctoWS2811 g_octo;

const int MAX_BRIGHTNESS = 255;
const int MAX_SATURATION = 255;

class Pattern
{
  public:
    virtual ~Pattern()
    {
    }

    virtual void setup() = 0;
    virtual bool display() = 0;
};

class Pattern_huey
    : public Pattern
{
  public:
    virtual void setup();
    virtual bool display();
};

// Drawing commands
typedef int Colour;
const Colour COLOUR_BLACK = 0;
inline Colour make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

Colour make_hsv(uint8_t h, uint8_t s, uint8_t v);

inline int get_led(int x, int y)
{
    return y * COLUMN_COUNT + x;
}

inline void get_xy(int led, int &x, int &y)
{
    x = led % COLUMN_COUNT;
    y = led / COLUMN_COUNT;
}

inline Colour get_pixel(int led)
{
    return g_octo.getPixel(led);
}

inline Colour get_pixel_xy(int x, int y)
{
    return get_pixel(get_led(x, y));
}

inline void draw_pixel(int led, Colour c)
{
    g_octo.setPixel(led, c);
}

inline void draw_pixel_xy(int x, int y, Colour c)
{
    draw_pixel(get_led(x, y), c);
}

inline void draw_pixels(Colour c)
{
    for (int led = 0; led < LED_COUNT; ++led) {
	draw_pixel(led, c);
    }
}

inline void show_pixels()
{
    g_octo.show();
}

void update_hue();

// Utility functions
int clamp(int x, int min, int max);

class Value
{
  public:
    Value(int initial);
    Value(int initial, int min, int max, bool wraps = false);

    void set_min(int min);
    void set_max(int max);
    void set_wrap(bool wraps);

    int get_min();
    int get_max();
    bool get_wrap();
    int get_range();

    void set(int value);
    void modify(int delta);

    void set_velocity(int delta, int delta_ms);
    void set_periodic(int amplitude, int cycle_ms);

    int get();

  private:
    int now();
    int bound(int value);

    int m_min;
    int m_max;
    int m_wraps;

    int m_value;
    int m_value_time;

    int m_velocity;
    int m_velocity_ms;

    int m_periodic;
    int m_periodic_ms;
};

extern Value g_brightness;
extern Value g_hue;

// lock
// selected mode
// display pattern
// brightness
// hue change speed
// max refresh rate
// sample rate
// sound sensitivity
//   gain
//   min
//   max

#endif // !_LUSH_H_INCLUDED
