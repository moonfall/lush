#ifndef _PATTERNS_H_INCLUDED
#define _PATTERNS_H_INCLUDED

#include "displaylist.h"
#include "fader.h"
#include "lush.h"
#include "maze.h"

class Pattern
{
  public:
    virtual ~Pattern()
    {
    }

    virtual void setup()
    {
    }

    virtual void ui_callback(Element_id id, Element const &element)
    {
    }

    virtual void ui_hook()
    {
    }

    virtual void activate(void *arg)
    {
    }

    // full screen will handle clearing background if necessary, otherwise
    // Pattern expects a reasonable background
    virtual bool is_full_screen()
    {
	return false;
    }

    virtual bool display() = 0;
};

class Pattern_audio
    : public Pattern
{
  public:
    virtual void ui_callback(Element_id id, Element const &element);
};

class Pattern_random_fader
    : public Pattern
{
  public:
    Pattern_random_fader(Fader_static &fader);

    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

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

class Pattern_alphabet
    : public Pattern
{
  public:
    Pattern_alphabet();

    virtual void setup();

    virtual void ui_callback(Element_id id, Element const &element);

    virtual bool display();

    Value m_counter;
};

class Pattern_border
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();

  private:
    void display_current();

    int m_origin;
    bool m_clockwise;
    int m_x;
    int m_y;
    uint32_t m_last_move_ms;
};

class Pattern_counter
    : public Pattern
{
  public:
    Pattern_counter();

    virtual void setup();
    virtual void ui_callback(Element_id id, Element const &element);
    virtual bool display();

    Value m_counter;
};

class Pattern_dropper
    : public Pattern
{
  public:
    Pattern_dropper();

    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

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
    Pattern_huey();

    virtual void ui_callback(Element_id id, Element const &element);

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();

  private:
    Value m_hue_offset;
};

class Pattern_line
    : public Pattern
{
  public:
    virtual void activate(void *arg);
    virtual bool display();
  private:
    void display_current();

    uint32_t m_last_move_ms;
};

class Pattern_marquee
    : public Pattern
{
  public:
    Pattern_marquee();

    virtual void setup();

    virtual void ui_callback(Element_id id, Element const &element);

    virtual bool display();

    Value m_pos;
    char m_text[64];
};

class Pattern_maze
    : public Pattern
{
  public:
    Pattern_maze(Fader_static &fader);

    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();

    virtual void regenerate();
    virtual void randomize_maze();
    virtual void randomize_image(bool initial = false);
    virtual void ordered_image(bool initial = false);

    Fader_static &m_fader;

    Maze m_maze;

    int m_start_time;
};

class Pattern_off
    : public Pattern
{
  public:
    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();
};

class Pattern_peak_diagonal
    : public Pattern_audio
{
  public:
    virtual bool display();
};

class Pattern_peak_ordered
    : public Pattern_audio
{
  public:
    Pattern_peak_ordered(Fader_static &fader);

    virtual void activate(void *arg);
    virtual bool display();

    Fader_static &m_fader;
};

class Pattern_peak_noise
    : public Pattern_peak_ordered
{
  public:
    Pattern_peak_noise(Fader_static &fader);

    virtual void activate(void *arg);
    virtual bool display();
};

class Pattern_peak_spike
    : public Pattern_audio
{
  public:
    virtual bool display();
};

class Pattern_plasma
    : public Pattern
{
  public:
    Pattern_plasma();

    virtual void ui_callback(Element_id id, Element const &element);

    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

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

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();

  private:
    uint32_t m_activate_ms;
    int m_was_dark;
    int m_wheel;
    Colour m_last_colour;
};

class Pattern_race
    : public Pattern
{
  public:
    Pattern_race();

    virtual void setup();
    virtual bool display();

    Value m_counter;
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

	// The following should be last.
	RAIN_NUM_MODES,
    };

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

class Pattern_snake
    : public Pattern
{
  public:
    Pattern_snake();

    virtual void setup();
    virtual void ui_callback(Element_id id, Element const &element);
    virtual bool display();

    bool is_valid(int x, int y) const;
    bool is_collision(int x, int y) const;
    void advance();

    static const unsigned int MAX_LENGTH = COLUMN_COUNT * ROW_COUNT / 2;
    Value m_max_length;
    Pixel m_segments[MAX_LENGTH];
    unsigned m_length;
    int m_x;
    int m_y;
};

class Pattern_spectrum_bars
    : public Pattern_audio
{
  public:
    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();
};

class Pattern_spectrum_field
    : public Pattern_audio
{
  public:
    virtual void activate(void *arg);

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();
};

class Pattern_spectrum_timeline
    : public Pattern_audio
{
  public:
    virtual bool is_full_screen()
    {
	return true;
    }

    virtual void activate(void *arg);
    virtual bool display();
};

class Pattern_synthesia_fire
    : public Pattern
{
  public:
    Pattern_synthesia_fire();

    virtual void setup();

    virtual bool is_full_screen()
    {
	return true;
    }

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

    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();

    Value m_time;
};

class Pattern_wheel
    : public Pattern
{
  public:
    virtual bool is_full_screen()
    {
	return true;
    }

    virtual bool display();
};

struct Mode
{
    Mode(Pattern *pattern = NULL, void *arg = NULL, const char *id = NULL)
	: m_pattern(pattern), m_arg(arg), m_id(id)
    {
    }

    Pattern *m_pattern;
    void *m_arg;
    const char *m_id;
};

class Pattern_set
    : public Pattern
{
  public:
    Pattern_set(const Mode *modes, unsigned num_modes);

    virtual void ui_callback(Element_id id, Element const &element);
    virtual void ui_hook();

    virtual void activate(void *arg);
    virtual bool is_full_screen();
    virtual bool display();

  protected:
    Pattern *get_child();
    virtual void activate_child();
    
    void display_overlay_until(uint32_t ms)
    {
	m_overlay_end_ms = ms;
    }

    void display_overlay_for(uint32_t ms)
    {
	display_overlay_until(millis() + ms);
    }

    virtual void display_overlay();
    virtual void display_status_string(const char *s);

    const Mode *m_modes;
    unsigned m_num_modes;
    Value m_current_mode;

    bool m_force_update;
    uint32_t m_overlay_end_ms;
};

class Pattern_composite
    : public Pattern
{
  public:
    Pattern_composite(const Mode *modes, unsigned num_modes);

    virtual void ui_callback(Element_id id, Element const &element);
    virtual void ui_hook();

    virtual void activate(void *arg);
    virtual bool is_full_screen();
    virtual bool display();

  protected:
    Pattern *get_top();

    const Mode *m_modes;
    unsigned m_num_modes;

    bool m_force_update;
};

class Pattern_main_menu
    : public Pattern_set
{
  public:
    Pattern_main_menu(const Mode *modes, unsigned num_modes);

    virtual void ui_callback(Element_id id, Element const &element);
    virtual void ui_hook();

    virtual void activate(void *arg);

  private:
    void display_overlay();

    uint32_t m_unhandled_button_press_ms;
};

class Pattern_selector
    : public Pattern_set
{
  public:
    Pattern_selector(const Mode *modes, unsigned num_modes);

    virtual void ui_callback(Element_id id, Element const &element);
    virtual void ui_hook();

    virtual void activate(void *arg);

  protected:
    void display_overlay();

    bool m_locked;
    uint32_t m_unhandled_button_press_ms;
};

class Pattern_random
    : public Pattern_selector
{
  public:
    Pattern_random(const Mode *modes, unsigned num_modes);

    virtual void ui_hook();

    virtual void activate(void *arg);

  private:
    virtual void activate_child();
    void select_next();
    void display_overlay();

    uint32_t m_duration_s;

    uint32_t m_last_select_ms;
};

class Pattern_config
    : public Pattern_set
{
  public:
    Pattern_config(const Mode *modes, unsigned num_modes);

    virtual void ui_callback(Element_id id, Element const &element);
};

class Pattern_option
    : public Pattern
{
  public:
    Pattern_option(char const *name, Value &value, bool show_value);

    virtual void ui_callback(Element_id id, Element const &element);

  private:
    bool display();

    char const *m_name;
    Value &m_value;
    bool m_show_value;
};

#endif // !_PATTERNS_H_INCLUDED
