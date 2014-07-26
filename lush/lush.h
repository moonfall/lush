#ifndef _LUSH_H_INCLUDED
#define _LUSH_H_INCLUDED

#include <stdint.h>

extern const int ROW_COUNT;
extern const int COLUMN_COUNT;
extern const int LED_COUNT;
const int FFT_SIZE = 256;
const int MAGNITUDE_COUNT = FFT_SIZE / 2;
const int BIN_COUNT = 8;
extern OctoWS2811 g_octo;

const int MAX_BRIGHTNESS = 255;
const int MAX_SATURATION = 255;

const int FONT_WIDTH = 4;
const int FONT_HEIGHT = 5;

extern int16_t g_fft_samples[FFT_SIZE * 2];
extern int g_fft_sample_generation;
extern int16_t g_magnitudes[MAGNITUDE_COUNT];
extern int g_fft_generation;
extern int16_t g_bins[BIN_COUNT];
extern int g_bin_generation;

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

inline Colour get_pixel(int x, int y)
{
    return get_pixel(get_led(x, y));
}

inline void draw_pixel(int led, Colour c)
{
    g_octo.setPixel(led, c);
}

inline void draw_pixel(int x, int y, Colour c)
{
    draw_pixel(get_led(x, y), c);
}

inline void draw_pixels(Colour c)
{
    for (int led = 0; led < LED_COUNT; ++led) {
	draw_pixel(led, c);
    }
}

void draw_char(int x, int y, char c, Colour fg, const Colour *bg);

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
    Value(int initial = 0);
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

    int get_unbounded();
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

inline Colour make_hue(uint8_t h)
{
    return make_hsv(h, MAX_SATURATION, g_brightness.get());
}

inline Colour make_current_hue()
{
    return make_hsv(g_hue.get(), MAX_SATURATION, g_brightness.get());
}

class Pattern
{
  public:
    virtual ~Pattern()
    {
    }

    virtual void setup() = 0;
    virtual bool display() = 0;
};

class Pattern_counter
    : public Pattern
{
  public:
    Pattern_counter();

    virtual void setup();
    virtual bool display();

    Value m_counter;
};

class Pattern_huey
    : public Pattern
{
  public:
    virtual void setup();
    virtual bool display();
  
    Value m_hue_offset;
};

class Pattern_spectrum
    : public Pattern
{
  public:
    virtual void setup();
    virtual bool display();
};

struct Mode
{
    Pattern *m_pattern;
};

class Element_state
{
  public:
    Element_state(int value = 0)
	: m_value(value), m_ms(millis())
    {
    }
    int			m_value;
    int			m_ms;
};

class Element
{
  public:
    Element_state	m_data[2];

    Element_state get_current() const
    {
	return m_data[0];
    }

    Element_state get_previous() const
    {
	return m_data[1];
    }

    virtual int get_change(Element_state const &current,
	     Element_state const &previous) const
    {
	return current.m_value - previous.m_value;
    }

    virtual int get_current_change() const
    {
	return get_change(get_current(), get_previous());
    }

    int get_current_millis() const
    {
	return millis() - get_current().m_ms;
    }

    int get_previous_millis() const
    {
	return get_current().m_ms - get_previous().m_ms;
    }

    void push(Element_state const &current)
    {
	m_data[1] = m_data[0];
	m_data[0] = current;
    }
};

class Encoder_element
    : public Element
{
    virtual int get_change(Element_state const &current,
	    Element_state const &previous) const
    {
	return (previous.m_value - current.m_value) / 4;
    }
};

enum Element_id
{
    UI_KNOB1_ENCODER,
    UI_KNOB1_BUTTON,
    UI_KNOB2_ENCODER,
    UI_KNOB2_BUTTON,
};

class UI_state
{
  public:
    Encoder_element	m_knob1_encoder;
    Element		m_knob1_button;
    Encoder_element	m_knob2_encoder;
    Element		m_knob2_button;
};

extern UI_state g_ui;

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
