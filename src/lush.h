#ifndef _LUSH_H_INCLUDED
#define _LUSH_H_INCLUDED

#include <stdint.h>

const int ROW_COUNT = 8;
const int COLUMN_COUNT = 8;
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

extern const float GAIN_INTERCEPTS[MAGNITUDE_COUNT];
extern const float GAIN_SLOPES[MAGNITUDE_COUNT];

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
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
};

inline int get_led(int x, int y, int columns = COLUMN_COUNT)
{
    return y * columns + x;
}

inline int flip_x(int x, int columns = COLUMN_COUNT)
{
    return (columns - 1) - x;
}

inline int flip_y(int y, int rows = ROW_COUNT)
{
    return (rows - 1) - y;
}

inline void get_xy(int led, int &x, int &y, int columns = COLUMN_COUNT)
{
    x = led % columns;
    y = led / columns;
}

inline void make_neighbour(Direction dir, int &x, int &y)
{
    switch (dir) {
	case DIR_LEFT:
	    --x;
	    break;
	case DIR_RIGHT:
	    ++x;
	    break;
	case DIR_UP:
	    --y;
	    break;
	case DIR_DOWN:
	    ++y;
	    break;
    }
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

void draw_mask(int y, unsigned mask, Colour fg, const Colour *bg);
void draw_char(int x, int y, char c, Colour fg, const Colour *bg);

inline void show_pixels()
{
    g_octo.show();
}

void program_gain();
void update_pattern();

void set_target_fps(unsigned fps);

void ui_advance_mode();
void ui_loop();
void turn_on();
void turn_off();

void sampler_loop();
void display_loop();

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

    virtual void activate(void *arg)
    {
    }

    virtual bool display() = 0;
};

class Maze
{
  public:
    const uint8_t WALL_VALUE = 0;

    Maze();

    void reset();

    void complete();
    // Attempts to expand the maze.  Returns true if it was successfully,
    // expanded.
    bool expand();

    uint8_t get_order(int index)
    {
	return m_maze[index];
    }

    bool in_bounds(int x, int y)
    {
	return x >= 0 && x < COLUMN_COUNT && y >= 0 && y < ROW_COUNT;
    }

    void add_maze(int x, int y, bool add_to_list);
    void queue_maze(int x, int y);

    void add_wall_list(int x, int y);
    void remove_wall_list(int choice);
    bool in_wall_list(int index);

    uint8_t get_maze(int x, int y)
    {
	return in_bounds(x, y) ? m_maze[get_led(x, y)] : WALL_VALUE;
    }

    uint8_t get_maze(Direction dir, int x, int y)
    {
	make_neighbour(dir, x, y);
	return get_maze(x, y);
    }

    int m_update_ms;
    int m_finished_ms;

    // TODO: Shrink down representation
    // TODO: Use shared scratch space
    uint8_t m_maze[LED_COUNT];
    unsigned m_count;
    uint8_t m_wall_list[LED_COUNT];
    int m_wall_list_count;
};

class Fader
{
  public:
    virtual int get_order(int led) = 0;
    virtual int fade_start(int order) = 0;
    virtual int fade_duration(int order) = 0;
    virtual int fade_end(int order) = 0;

    virtual Colour get_initial(int led) = 0;
    virtual Colour get_final(int led) = 0;

    // If done is non-NULL, it is set to indicate if the current led
    // is done.
    virtual Colour generate(int led, int t, bool *done);

    virtual int determine_duration();

    virtual inline Colour fade(Colour initial, Colour final, int t, int total)
    {
	return linear_fade(initial, final, t, total);
    }

    static Colour linear_fade(Colour initial, Colour final, int t, int total);
};

typedef int Fade_pattern;

class Fader_fixed
    : public Fader
{
  public:
    // The following sets both the order and the times.
    void set_fade_pattern(Fade_pattern fade_pattern,
			  int stagger, int duration);

    // The following determine the order that each pixel will be faded.
    void set_scale(int scale, int &columns, int &rows) {
	columns = COLUMN_COUNT / scale;
	rows = ROW_COUNT / scale;
    }
    void set_shuffled(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
    // Fill in scale * scale squares
    void set_inorder(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
    void set_alternate_ends(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
    void set_back_forth(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
    void set_alternate_back_forth(int columns = COLUMN_COUNT,
				  int rows = ROW_COUNT);
    void set_top_and_bottom(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
    void set_top_and_bottom_reversed(int columns = COLUMN_COUNT,
				     int rows = ROW_COUNT);
    void set_spiral(int columns = COLUMN_COUNT, int rows = ROW_COUNT);
#if 0
    void set_inner_spiral();
#endif

    // The following determines when to start each fade and how long
    // based on their order.  count specifies the number of consecutive
    // fades to set to the same times.
    // scale_times scales them by count if set.
    void set_staggered_pixels(int stagger, int duration, int count = 1,
			      bool scale_times = true);

    // Calulcate stagger or duration based on total duration.
    // returns stagger
    static int fixed_duration_total_duration(int duration,
					     int count, int total_duration);
    // returns duration
    static int fixed_stagger_total_duration(int stagger,
					    int count, int total_duration);
    // proportion is ratio of duration to stagger
    // returns stagger
    // duration == stagger * proportion
    static int proportional_total_duration(float proportion,
					   int count, int total_duration);

    virtual int get_order(int led);
    virtual int fade_start(int order);
    virtual int fade_duration(int order);
    virtual int fade_end(int order);

    // TODO: remove this
    int m_order[LED_COUNT];

    // indexed by order, both in ms
    int m_fade_start[LED_COUNT];
    int m_fade_duration[LED_COUNT];
};

#if 0
class Fader_filter
    : public Fader_fixed
{
};
#endif

class Fader_static
    : public Fader_fixed
{
  public:
    Fader_static();

    void set_colour(bool initial, int led, Colour c)
    {
	if (initial) {
	    m_initial[led] = c;
	} else {
	    m_final[led] = c;
	}
    }
    void set_colours(bool initial, Colour c);
    void set_initial_from_current();
    void set_initial_from_final();

    virtual Colour get_initial(int led)
    {
	return m_initial[led];
    }

    virtual Colour get_final(int led)
    {
	return m_final[led];
    }

    Colour m_initial[LED_COUNT];
    Colour m_final[LED_COUNT];
};

class Pattern_random_fader
    : public Pattern
{
  public:
    Pattern_random_fader(Fader_static &fader);

    virtual void activate(void *arg);
    virtual bool display();

    void randomize();
    void randomize_image(bool initial);
    void randomize_fade();

    Fader_static &m_fader;
    int m_fade_pattern;
    int m_start_time;
    // fade_out: initial -> black; !fade_out: black -> initial
    bool m_fade_out;
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

class Pattern_dropper
    : public Pattern
{
  public:
    Pattern_dropper();

    virtual void activate(void *arg);
    virtual bool display();

    class Drop
    {
      public:
	Drop();

	void reset();

	Colour m_c;
	Value m_x;
	Value m_y;
    };

    void *m_activate_arg;
    Colour m_field[LED_COUNT];
    Drop m_drop;
    uint32_t m_reset_ms;
    bool m_needs_new_drop;
};

class Pattern_heart
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();

    uint32_t m_activate_ms;
    Colour m_last_colour;
};

class Pattern_huey
    : public Pattern
{
  public:
    virtual bool display();
};

class Pattern_maze
    : public Pattern
{
  public:
    Pattern_maze(Fader_static &fader);

    virtual void activate(void *arg);
    virtual bool display();

    virtual void regenerate();
    virtual void randomize_maze();
    virtual void randomize_image(bool initial = false);
    virtual void ordered_image(bool initial = false);

    Fader_static &m_fader;

    Maze m_maze;

    int m_start_time;
};

class Pattern_peak
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();
};

class Pattern_plasma
    : public Pattern
{
  public:
    Pattern_plasma();

    virtual void activate(void *arg);
    virtual bool display();

    void reset_plasma();
    void layer(double value);
    double plasma();

    Value m_time;
    int m_palette;
    double m_plasma_sum;
    double m_plasma_count;
};

class Pattern_pulse
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();

  private:
    uint32_t m_activate_ms;
    int m_was_dark;
    int m_wheel;
    Colour m_last_colour;
};

class Pattern_rain
    : public Pattern
{
  public:
    enum Mode
    {
	RAIN_RANDOM,
	RAIN_RANDOM_COLOUR,
	RAIN_CURRENT_HUE,
	RAIN_SINGLE_RANDOM_COLOUR,
	RAIN_PURE_WHITE,

	// The follow should be last.
	RAIN_NUM_MODES,
    };

    Pattern_rain();

    // arg is casted to Mode.
    virtual void activate(void *arg);
    virtual bool display();
    Colour get_colour() const;

    class Drop
    {
      public:
	Drop(Colour c = COLOUR_BLACK);

	void reset(Colour c);

	Colour m_c;
	Value m_x;
	Value m_y;
    };

    Mode m_mode;
    int m_wheel;
    Drop m_drop;
};

class Pattern_spectrum_bars
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();
};

class Pattern_spectrum_field
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();
};

class Pattern_spectrum_timeline
    : public Pattern
{
  public:
    virtual void activate(void *arg);
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

class Pattern_wheel
    : public Pattern
{
  public:
    virtual bool display();
};

struct Mode
{
    Pattern *m_pattern;
    void *m_arg;
};

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
