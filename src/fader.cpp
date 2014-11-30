#include "fader.h"
#include "blocks.h"
#include "lush.h"

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
    int scale = 1;
    int columns = COLUMN_COUNT;
    int rows = ROW_COUNT;
    switch (fade_pattern % 14) {
	case 0:
	default:
	    set_shuffled();
	    break;
	case 2:
	    set_back_forth();
	    break;
	case 3:
	    set_alternate_back_forth();
	    break;
	case 4:
	    set_top_and_bottom_reversed();
	    set_staggered_pixels(stagger, duration, 2);
	    break;
	case 5:
	    set_spiral();
	    break;
	case 6:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_shuffled(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale * scale);
	    break;
	case 7:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_inorder(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale * scale);
	    break;
	case 8:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_back_forth();

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale * scale);
	    break;
	case 9:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_alternate_back_forth(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale * scale);
	    break;
	case 10:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_top_and_bottom_reversed(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, 2);
	    break;
	case 11:
	    scale = 2;
	    set_scale(scale, columns, rows);

	    set_spiral(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale * scale);
	    break;

	case 12:
	    scale = 4;
	    set_scale(scale, columns, rows);

	    set_shuffled(columns, rows);

	    expand_array_2d(m_order, columns, rows, scale);
	    set_staggered_pixels(stagger, duration, scale);
	    break;

	case 13:
	    set_inorder();

	    set_staggered_pixels(stagger, duration, COLUMN_COUNT);
	    break;
#if 0
// Disable not working ones.
	case 7:
	    set_inner_spiral();
	    break;
#endif
    }
}

void Fader_fixed::set_shuffled(int columns, int rows)
{
    make_shuffled_array(m_order, columns * rows);
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
void Fader_fixed::set_inorder(int columns, int rows)
{
    // This fills an array of columns*rows.

    Simple_counter y(rows, true);
    Simple_counter x(columns, true);
    For_each_led g(y, x, columns);

    g.fill_array(m_order, rows * columns);
}

// looks bad
// 0123
// 89ab
// cdef
// 4567
void Fader_fixed::set_alternate_ends(int columns, int rows)
{
    Simple_counter y1(rows / 2, true);
    Counter y2(rows - 1, rows / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x(columns, true);
    For_each_led g(y, x, columns);

    g.fill_array(m_order, rows * columns);
}

// 0123
// 7654
// 89ab
// fedc
void Fader_fixed::set_back_forth(int columns, int rows)
{
    Simple_counter y(rows, true);
    Simple_counter x1(columns, true);
    Simple_counter x2(columns, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x, columns);

    g.fill_array(m_order, rows * columns);
}

// 0123
// 89ab
// fedc
// 7654
void Fader_fixed::set_alternate_back_forth(int columns, int rows)
{
    Simple_counter y1(rows / 2, true);
    Counter y2(rows - 1, rows / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x1(columns, true);
    Simple_counter x2(columns, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x, columns);

    g.fill_array(m_order, rows * columns);
}

// 0246
// 8ace
// 9bdf
// 1357
void Fader_fixed::set_top_and_bottom(int columns, int rows)
{
    Simple_counter y1(rows / 2, true);
    Simple_counter x1(columns, true);
    For_each_led g1(y1, x1, columns);

    Counter y2(rows - 1, rows / 2, -1);
    Simple_counter x2(columns, true);
    For_each_led g2(y2, x2, columns);

    Alternator g(g1, g2);

    g.fill_array(m_order, rows * columns);
}

// 0246
// 8ace
// fdb9
// 7531
void Fader_fixed::set_top_and_bottom_reversed(int columns, int rows)
{
    Simple_counter y1(rows / 2, true);
    Simple_counter x1(columns, true);
    For_each_led g1(y1, x1, columns);

    Counter y2(rows - 1, rows / 2, -1);
    Simple_counter x2(columns, false);
    For_each_led g2(y2, x2, columns);

    Alternator g(g1, g2);

    g.fill_array(m_order, rows * columns);
}

// 0123
// bcd4
// afe5
// 9876
void Fader_fixed::set_spiral(int columns, int rows)
{
    int x_pos = 0;
    int y_pos = 0;

    int x_delta = 1;
    int y_delta = 0;

    int x_end = columns - 1;
    int y_end = rows - 1;
    for (int order = 0; order < rows * columns; ++order) {
	m_order[get_led(x_pos, y_pos, columns)] = order;
	x_pos += x_delta;
	y_pos += y_delta;
	if (x_delta && x_pos == x_end) {
	    y_delta = x_delta;
	    x_delta = 0;
	    x_end = flip_x(x_pos, columns);
	    if (x_end > columns / 2) {
		--x_end;
	    }
	} else if (y_delta && y_pos == y_end) {
	    x_delta = -y_delta;
	    y_delta = 0;
	    y_end = flip_y(y_end, rows);
	    if (y_end < rows / 2) {
		++y_end;
	    }
	}
    }
}

#if 0
// fdec
// 432b
// 501a
// 6789
void Fader_fixed::set_inner_spiral()
{
    set_spiral();
    reverse_array(m_order, LED_COUNT);
}
#endif

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
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    m_initial[get_led(x, y)] = get_pixel(x, y);
	}
    }
}

void Fader_static::set_initial_from_final()
{
    for (int led = 0; led < LED_COUNT; ++led) {
	m_initial[led] = m_final[led];
    }
}
