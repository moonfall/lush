#include <OctoWS2811.h>
#include "lush.h"
#include "blocks.h"

Colour Fader::generate(int led, int t, bool *done)
{
    // Determine fade state.
    Colour initial = get_initial(led);
    Colour final = get_final(led);
    int order = get_order(led);
    int start = fade_start(order);
    int end = fade_end(order);

    // Calculate colour.
    Colour c;
    if (t < start) {
	c = initial;
	if (done) {
	    *done = false;
	}
    } else if (t >= end) {
	c = final;
	if (done) {
	    *done = true;
	}
    } else {
	c = fade(initial, final, t - start, end - start);
	if (done) {
	    *done = false;
	}
    }

    return c;
}

int Fader::determine_duration()
{
    int max_end = 0;
    for (int order = 0; order < LED_COUNT; ++order) {
	max_end = max(max_end, fade_end(order));
    }

    return max_end;
}

Colour Fader::linear_fade(Colour start, Colour end, int t, int duration)
{
    uint8_t start_r;
    uint8_t start_g;
    uint8_t start_b;
    split_rgb(start, start_r, start_g, start_b);

    uint8_t end_r;
    uint8_t end_g;
    uint8_t end_b;
    split_rgb(end, end_r, end_g, end_b);

#if 0
    float alpha = (float) t / (float) duration;
    uint8_t r = (1.0 - alpha) * start_r + alpha * end_r;
    uint8_t g = (1.0 - alpha) * start_g + alpha * end_g;
    uint8_t b = (1.0 - alpha) * start_b + alpha * end_b;
#else
    uint8_t r = (duration - t) * start_r / duration + t * end_r / duration;
    uint8_t g = (duration - t) * start_g / duration + t * end_g / duration;
    uint8_t b = (duration - t) * start_b / duration + t * end_b / duration;
#endif

    return make_rgb(r, g, b);
}

void Fader_fixed::set_fade_pattern(Fade_pattern fade_pattern,
				   int stagger, int duration)
{
    set_staggered_pixels(stagger, duration);
    switch (fade_pattern % 10) {
	case 0:
	default:
	    set_shuffled();
	    break;
	case 1:
	    set_inorder();
	    break;
	case 2:
	    set_back_forth_squares();
	    break;
	case 3:
	    set_alternate_back_forth_squares();
	    break;
	case 4:
	    set_top_and_bottom_reversed();
	    set_staggered_pixels(stagger, duration, 2);
	    break;
	case 5:
	    set_inorder();
	    set_staggered_pixels(stagger, duration, COLUMN_COUNT);
	    break;
	case 6:
	    set_spiral();
	    break;
	case 7:
	    set_shuffled(2);
	    set_staggered_pixels(stagger, duration, 4);
	    break;
	case 8:
	    set_inorder(2);
	    set_staggered_pixels(stagger, duration, 4);
	    break;
	case 9:
	    set_shuffled(4);
	    set_staggered_pixels(stagger, duration, 4);
	    break;
#if 0
// Disable not working ones.
	case 7:
	    set_inner_spiral();
	    break;
#endif
    }
}

void Fader_fixed::set_shuffled(int scale)
{
    make_shuffled_array(m_order, LED_COUNT / (scale * scale));
    if (scale > 1) {
	expand_array_2d(m_order, COLUMN_COUNT / scale, ROW_COUNT / scale,
		        scale);
    }
}

// scale = 1
// 0123
// 4567
// 89ab
// cdef
// scale = 2
// 0145
// 2367
// 89cd
// abef
void Fader_fixed::set_inorder(int scale)
{
    // This fills an array of enough scale*scale squares to fit the target.
    int unscaled_rows = ROW_COUNT / scale;
    int unscaled_cols = COLUMN_COUNT / scale;

    Simple_counter y(unscaled_rows, true);
    Simple_counter x(unscaled_cols, true);
    For_each_led g(y, x, unscaled_cols);

    g.fill_array(m_order, unscaled_rows * unscaled_cols);
    if (scale > 1) {
	expand_array_2d(m_order, unscaled_cols, unscaled_rows, scale);
    }
}

// looks bad
// 0123
// 89ab
// cdef
// 4567
void Fader_fixed::set_alternate_ends_squares()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x(COLUMN_COUNT, true);
    For_each_led g(y, x);

    g.fill_array(m_order, LED_COUNT);
}

// 0123
// 7654
// 89ab
// fedc
void Fader_fixed::set_back_forth_squares()
{
    Simple_counter y(ROW_COUNT, true);
    Simple_counter x1(COLUMN_COUNT, true);
    Simple_counter x2(COLUMN_COUNT, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x);

    g.fill_array(m_order, LED_COUNT);
}

// 0123
// 89ab
// fedc
// 7654
void Fader_fixed::set_alternate_back_forth_squares()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x1(COLUMN_COUNT, true);
    Simple_counter x2(COLUMN_COUNT, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x);

    g.fill_array(m_order, LED_COUNT);
}

// 0246
// 8ace
// 9bdf
// 1357
void Fader_fixed::set_top_and_bottom()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Simple_counter x1(COLUMN_COUNT, true);
    For_each_led g1(y1, x1);

    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Simple_counter x2(COLUMN_COUNT, true);
    For_each_led g2(y2, x2);

    Alternator g(g1, g2);

    g.fill_array(m_order, LED_COUNT);
}

// 0246
// 8ace
// fdb9
// 7531
void Fader_fixed::set_top_and_bottom_reversed()
{
    int x_pos = 0;
    int y_pos = 0;

    int x_delta = 1;
    for (int order = 0; order < LED_COUNT; order += 2) {
	m_order[get_led(x_pos, y_pos)] = order;
	m_order[get_led(flip_x(x_pos), flip_y(y_pos))] = order + 1;
	x_pos += x_delta;
	if (x_pos == COLUMN_COUNT) {
	    ++y_pos;
	    x_pos = 0;
	}
    }
}

// 0123
// bcd4
// afe5
// 9876
void Fader_fixed::set_spiral()
{
    int x_pos = 0;
    int y_pos = 0;

    int x_delta = 1;
    int y_delta = 0;

    int x_end = COLUMN_COUNT - 1;
    int y_end = ROW_COUNT - 1;
    for (int order = 0; order < LED_COUNT; ++order) {
	m_order[get_led(x_pos, y_pos)] = order;
	x_pos += x_delta;
	y_pos += y_delta;
	if (x_delta && x_pos == x_end) {
	    y_delta = x_delta;
	    x_delta = 0;
	    x_end = flip_x(x_pos);
	    if (x_end > COLUMN_COUNT / 2) {
		--x_end;
	    }
	} else if (y_delta && y_pos == y_end) {
	    x_delta = -y_delta;
	    y_delta = 0;
	    y_end = flip_y(y_end);
	    if (y_end < ROW_COUNT / 2) {
		++y_end;
	    }
	}
    }
}

// fdec
// 432b
// 501a
// 6789
void Fader_fixed::set_inner_spiral()
{
    set_spiral();
    reverse_array(m_order, LED_COUNT);
}

void Fader_fixed::set_staggered_pixels(int stagger, int duration, int count,
				       bool scale_times)
{
    if (scale_times) {
	stagger *= count;
	duration *= count;
    }

    for (int order = 0, rank = 0; order < LED_COUNT; order += count, ++rank) {
	for (int i = 0; i < count; ++i) {
	    m_fade_start[order + i] = stagger * rank;
	    m_fade_duration[order + i] = duration;
	}
    }
}

// duration + (count - 1) * stagger == total_duration
int Fader_fixed::fixed_duration_total_duration(int duration,
					       int count, int total_duration)
{
    return (total_duration - duration) / (count - 1);
}

int Fader_fixed::fixed_stagger_total_duration(int stagger,
					      int count, int total_duration)
{
    return (total_duration - (count - 1) * stagger);
}

// proportion * stagger + (count - 1) * stagger == total_duration
// (propotion + count - 1) * stagger == total_duration
int Fader_fixed::proportional_total_duration(float proportion,
					     int count, int total_duration)
{
    return (int) ((float) total_duration / (proportion + (float) count - 1));
}

int Fader_fixed::get_order(int led)
{
    return m_order[led];
}

int Fader_fixed::fade_start(int order)
{
    return m_fade_start[order];
}

int Fader_fixed::fade_duration(int order)
{
    return m_fade_duration[order];
}

int Fader_fixed::fade_end(int order)
{
    return fade_start(order) + fade_duration(order);
}

Fader_static::Fader_static()
{
    set_colours(true, COLOUR_BLACK);
    set_colours(false, COLOUR_BLACK);
}

void Fader_static::set_colours(bool initial, Colour c)
{
    for (int led = 0; led < LED_COUNT; ++led) {
	set_colour(initial, led, c);
    }
}

void Fader_static::set_initial_from_current()
{
    for (int led = 0; led < LED_COUNT; ++led) {
	m_initial[led] = get_pixel(led);
    }
}

void Fader_static::set_initial_from_final()
{
    for (int led = 0; led < LED_COUNT; ++led) {
	m_initial[led] = m_final[led];
    }
}

Pattern_random_fader::Pattern_random_fader(Fader_static &fader)
    : m_fader(fader), m_fade_pattern(0), m_start_time(0), m_fade_out(true)
{
}

void Pattern_random_fader::activate()
{
    // Start off with something.
    m_fade_out = true;
    randomize_image(true);
    randomize_fade();
}

void Pattern_random_fader::randomize_image(bool initial)
{
    int start = millis();
    int x_offset = random(1, 10);
    int y_offset = random(10, 100);
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int wheel = start + x * x_offset + y * y_offset;
	    m_fader.set_colour(initial, get_led(x, y),
			       make_wheel(wheel, g_brightness.get()));
	}
    }
    m_fader.set_colours(!initial, COLOUR_BLACK);
}

void Pattern_random_fader::randomize_fade()
{
#ifdef CYCLE_PATTERNS
    ++m_fade_pattern;
#else
    m_fade_pattern = random();
#endif
    const float DURATION_PROPORTION = 2;
    const int TOTAL_DURATION = 2000;
    int stagger = m_fader.proportional_total_duration(DURATION_PROPORTION,
						      LED_COUNT, 
						      TOTAL_DURATION);
    int duration = (int) (DURATION_PROPORTION * stagger);
    m_fader.set_fade_pattern(m_fade_pattern, stagger, duration);
    
    m_start_time = millis();
}

void Pattern_random_fader::randomize()
{
    if (m_fade_out) {
	// Just finished fading out, select something to fade in to.
	randomize_image(false);
    } else {
	// Just finished fading in, fade out from that now.
	m_fader.set_initial_from_final();
	m_fader.set_colours(false, COLOUR_BLACK);
    }
    m_fade_out = !m_fade_out;

    randomize_fade();
}

bool Pattern_random_fader::display()
{
    int now = millis();
    int t = now - m_start_time;

    bool all_done = true;
    for (int led = 0; led < LED_COUNT; ++led) {
	bool done = false;
	draw_pixel(led, m_fader.generate(led, t, &done));
	if (!done) {
	    all_done = false;
	}
    }

    if (all_done) {
	randomize(); 
    }

    return true;
}

