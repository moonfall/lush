#ifndef _FADER_H_INCLUDED
#define _FADER_H_INCLUDED

#include "lush.h"

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

#endif // !_FADER_H_INCLUDED
