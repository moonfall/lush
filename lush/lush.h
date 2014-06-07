#ifndef _LUSH_H_INCLUDED
#define _LUSH_H_INCLUDED

#include <stdint.h>

const int ROW_COUNT = 8;
const int COLUMN_COUNT = 8;
const int LED_COUNT = ROW_COUNT * COLUMN_COUNT;
const int LEDS_PER_STRIP = LED_COUNT;
const int FFT_SIZE = 256;
const int MAGNITUDE_COUNT = FFT_SIZE / 2;
const int MAX_BIN_COUNT = 64;
extern OctoWS2811 g_octo;

const int MAX_BRIGHTNESS = 255;
const int MAX_SATURATION = 255;

const int FONT_WIDTH = 4;
const int FONT_HEIGHT = 5;

extern const float GAIN_INTERCEPTS[MAGNITUDE_COUNT];
extern const float GAIN_SLOPES[MAGNITUDE_COUNT];

#undef Q15
#undef Q31
#define F32

#if defined(Q15)
#define arm_cfft_radix4_instance arm_cfft_radix4_instance_q15
#define arm_cfft_radix4_init arm_cfft_radix4_init_q15
#define arm_cfft_radix4 arm_cfft_radix4_q15
typedef int16_t Sample_type;
#elif defined(Q31)
#define arm_cfft_radix4_instance arm_cfft_radix4_instance_q31
#define arm_cfft_radix4_init arm_cfft_radix4_init_q31
#define arm_cfft_radix4 arm_cfft_radix4_q31
typedef int32_t Sample_type;
#else
#define arm_cfft_radix4_instance arm_cfft_radix4_instance_f32
#define arm_cfft_radix4_init arm_cfft_radix4_init_f32
#define arm_cfft_radix4 arm_cfft_radix4_f32
typedef float Sample_type;
#endif

extern Sample_type g_fft_samples[FFT_SIZE * 2];
extern int g_fft_sample_generation;
extern Sample_type g_magnitudes[FFT_SIZE];
extern int g_fft_generation;
extern Sample_type g_bins[MAX_BIN_COUNT];
extern int g_bin_generation;

// Drawing commands
typedef int Colour;
const Colour COLOUR_BLACK = 0;
inline Colour make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}
inline Colour make_rgb(int r, int g, int b, uint8_t brightness)
{
    return make_rgb(r * brightness / MAX_BRIGHTNESS,
		    g * brightness / MAX_BRIGHTNESS,
		    b * brightness / MAX_BRIGHTNESS);
}

Colour make_hsv(uint8_t h, uint8_t s, uint8_t v);
Colour make_hsv16(uint8_t h, uint8_t s, uint8_t v);
// wheel 0..767
Colour make_wheel(uint16_t wheel, uint8_t brightness);

inline int get_led(int x, int y)
{
    return y * COLUMN_COUNT + x;
}

inline int flip_x(int x)
{
    return (COLUMN_COUNT - 1) - x;
}

inline int flip_y(int y)
{
    return (ROW_COUNT - 1) - y;
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
extern Value g_bin_count;

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

    virtual void setup()
    {
    }

    virtual void activate()
    {
    }

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

class Pattern_spectrum_bars
    : public Pattern
{
  public:
    virtual void activate();
    virtual bool display();
};

class Pattern_spectrum_field
    : public Pattern
{
  public:
    virtual void activate();
    virtual bool display();
};

class Pattern_spectrum_timeline
    : public Pattern
{
  public:
    virtual void activate();
    virtual bool display();
};

class Pattern_synthesia_fire
    : public Pattern
{
  public:
    Pattern_synthesia_fire();

    virtual void setup();
    virtual bool display();

    int m_matrix_layer1[COLUMN_COUNT][ROW_COUNT];
    int m_matrix_layer2[COLUMN_COUNT][ROW_COUNT];
};

class Pattern_synthesia_plasma_complex
    : public Pattern
{
  public:
    Pattern_synthesia_plasma_complex();

    virtual void setup();
    virtual bool display();

    Value m_time;
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

template <class T>
T running_average(T &sum, int count, T sample)
{
    T mean = sum / count;
    sum += sample - mean;
    return mean;
}

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
