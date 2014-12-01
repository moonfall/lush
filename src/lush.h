#ifndef _LUSH_H_INCLUDED
#define _LUSH_H_INCLUDED

#include <OctoWS2811.h>
#include <stdint.h>


// Enable the following for the 16x16 pixel grid
#undef FLIPPED_X_COORDS
#undef LARGE_MATRIX

#ifdef LARGE_MATRIX
const int ROW_COUNT = 16;
const int COLUMN_COUNT = 16;
#else
const int ROW_COUNT = 8;
const int COLUMN_COUNT = 8;
#endif
const int LED_COUNT = ROW_COUNT * COLUMN_COUNT;
const int LEDS_PER_STRIP = LED_COUNT;
#undef FFT1024
#ifdef FFT1024
const int FFT_SIZE = 1024;
#else
const int FFT_SIZE = 256;
#endif
const int MAGNITUDE_COUNT = FFT_SIZE / 2;
const int MAX_BIN_COUNT = 64;
extern OctoWS2811 g_octo;

const int MAX_BRIGHTNESS = 255;
const int MAX_SATURATION = 255;

const int FONT_WIDTH = 4;
const int FONT_HEIGHT = 5;
extern const char FONT_START;
extern const int FONT_COUNT;
extern const char FONT_END;

typedef int16_t Sample_type;
typedef int32_t Bin_type;
extern Sample_type g_magnitudes[MAGNITUDE_COUNT];
extern Bin_type g_bins[MAX_BIN_COUNT];

// Drawing commands
typedef int Colour;
const Colour COLOUR_BLACK = 0;
inline Colour make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

inline Colour make_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}

inline Colour make_rgb(int r, int g, int b, uint8_t brightness)
{
    return make_rgb(r * brightness / MAX_BRIGHTNESS,
		    g * brightness / MAX_BRIGHTNESS,
		    b * brightness / MAX_BRIGHTNESS);
}

inline void split_rgb(Colour c, uint8_t &r, uint8_t &g, uint8_t &b)
{
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
}

Colour make_hsv(uint8_t h, uint8_t s, uint8_t v);
Colour make_hsv16(uint8_t h, uint8_t s, uint8_t v);
// wheel == 0 .. 1535 (6 * 256 - 1)
const int MAX_WHEEL = 256 * 6;
Colour make_wheel(uint16_t wheel, uint8_t brightness);
Colour make_reverse_wheel(uint16_t wheel, uint8_t brightness);
Colour make_palette(uint16_t palette, uint16_t index, uint8_t brightness);

// make_wheel384 -> make_wheel7
// wheel == 0 .. 768
Colour make_wheel7(uint16_t wheel, uint8_t brightness);
Colour make_reverse_wheel7(uint16_t wheel, uint8_t brightness);

enum Direction
{
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    NUM_DIRECTIONS,
};

inline int flip_x(int x, int columns = COLUMN_COUNT)
{
    return (columns - 1) - x;
}

inline int flip_y(int y, int rows = ROW_COUNT)
{
    return (rows - 1) - y;
}

extern Direction g_up_direction;

int get_led_dir(Direction dir, int x, int y, int columns = COLUMN_COUNT);
inline int get_led(int x, int y, int columns = COLUMN_COUNT)
{
    return get_led_dir(DIR_UP, x, y, columns);
}

void get_xy_dir(Direction dir, int led, int &x, int &y,
		int columns = COLUMN_COUNT);
inline void get_xy(int led, int &x, int &y, int columns = COLUMN_COUNT)
{
    get_xy_dir(DIR_UP, led, x, y, columns);
}

inline void make_neighbour(Direction dir, int &x, int &y)
{
    switch (dir) {
	case DIR_UP:
	    --y;
	    break;
	case DIR_RIGHT:
	    ++x;
	    break;
	case DIR_DOWN:
	    ++y;
	    break;
	case DIR_LEFT:
	    --x;
	    break;
	case NUM_DIRECTIONS:
	    break;
    }
}

// Move along the box that contains (origin,origin)
void move_along_box(int origin, bool clockwise, int &x, int &y);

inline Colour get_led(int led)
{
    return g_octo.getPixel(led);
}

inline Colour get_pixel(int x, int y)
{
    return get_led(get_led_dir(g_up_direction, x, y));
}

void blend_led(int led, Colour c);
inline void draw_led(int led, Colour c)
{
    if (c & 0xff000000) {
	// Blend pixel
	blend_led(led, c);
    } else {
	// Just write pixel.
	g_octo.setPixel(led, c);
    }
}

inline void draw_pixel(int x, int y, Colour c)
{
    draw_led(get_led_dir(g_up_direction, x, y), c);
}

inline void draw_pixels(Colour c)
{
    for (int led = 0; led < LED_COUNT; ++led) {
	draw_led(led, c);
    }
}

void draw_mask8(int x, int y, unsigned mask, Colour fg, const Colour *bg);
void draw_char(int x, int y, char c, Colour fg, const Colour *bg);
void draw_string(int x, int y, const char *s, Colour fg, const Colour *bg,
		 Direction dir = DIR_RIGHT, unsigned spacing = 0);
void draw_centered_string(int x, int y, int width, int height,
		          const char *s, Colour fg, const Colour *bg,
			  Direction dir = DIR_RIGHT, unsigned spacing = 0);
void draw_line(int x0, int y0, int x1, int y1, Colour c);

inline void show_pixels()
{
    g_octo.show();
}

void set_up_direction(int direction);

void set_gain(int gain);
void set_gain(int gain1, int gain2);
void adjust_gain(int adjustment);
void adjust_gain(int adjustment1, int adjustment2);
void program_gain();
void update_pattern();

void adjust_fft_max_power(int adjustment);

void set_target_fps(unsigned fps);

void ui_advance_mode();
void ui_loop();
void turn_on();
void turn_off();

void sampler_loop();
void display_loop();

void set_fft_bin_count(unsigned bin_count);
void set_fft_scale_factor(float scale_factor);
void fft_reduce();

void reset_peak();
unsigned get_peak();
// Maps peak from 0..max_value, accounting for noise floor.
unsigned get_mapped_peak(unsigned max_value);

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

    void set_callback(void (*callback)(int value));

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

    void (*m_callback)(int value);
};

extern Value g_brightness;
extern Value g_hue;

inline Colour make_hue(uint8_t h)
{
    return make_hsv(h, MAX_SATURATION, g_brightness.get());
}

inline Colour make_current_hue()
{
    return make_hue(g_hue.get());
}

inline Colour make_invert_hue()
{
    return make_hue(g_hue.get() + 128);
}

enum Element_id
{
    UI_KNOB1_ENCODER,
    UI_KNOB1_BUTTON,
    UI_KNOB2_ENCODER,
    UI_KNOB2_BUTTON,
};
class Element;

class Element_state
{
  public:
    Element_state(int value = 0)
	: m_value(value), m_micros(micros())
    {
    }
    int			m_value;
    int			m_micros;
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

    uint32_t get_current_millis() const
    {
	return (micros() - get_current().m_micros) / 1000;
    }

    uint32_t get_previous_millis() const
    {
	return (get_current().m_micros - get_previous().m_micros) / 1000;
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

extern Value g_brightness;
extern Value g_gain0;
extern Value g_gain1;
extern Value g_number;
extern Value g_up;
#endif // !_LUSH_H_INCLUDED
