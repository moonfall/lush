#include <OctoWS2811.h>
#include "lush.h"
#include "blocks.h"

static Colour g_initial[LED_COUNT];
static Colour g_final[LED_COUNT];

// TODO: fader order isn't necessary, just determine fade
// g_fade_order[led] == order
static int g_order[LED_COUNT];

const int DEFAULT_STAGGER = 25;
const int DEFAULT_DURATION = 50;

// g_fade_start[order] == ms
// g_fade_duration[order] == ms
static int g_fade_start[LED_COUNT];
static int g_fade_duration[LED_COUNT];

static int g_start_time = 0;

void Fader::activate()
{
#if 0
    set_fade_down();
    set_shuffled_squares();
    set_staggered_pixels(DEFAULT_STAGGER, DEFAULT_DURATION);
    g_start_time = millis();
#if 1
    reset();
#endif
#else
    reset();
#endif
}

#if 1
int sequence = 0;
void Fader::reset()
{
    int start = millis();
    int x_offset = random(1, 10);
    int y_offset = random(10, 100);
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    int wheel = start + x * x_offset + y * y_offset;
	    g_initial[get_led(x, y)] = make_wheel(wheel, g_brightness.get());
	}
    }
#if 1
    set_staggered_pixels(DEFAULT_STAGGER, DEFAULT_DURATION);
    switch (sequence) {
	case 0:
	default:
	    set_shuffled_squares();
	    // Reset to the beginning if reaching this through looping
	    // through everything.
	    sequence = 0;
	    break;
	case 1:
	    set_inorder_squares();
	    break;
	case 2:
	    set_back_forth_squares();
	    break;
	case 3:
	    set_alternate_back_forth_squares();
	    break;
	case 4:
	    set_top_and_bottom();
	    set_staggered_pixels(DEFAULT_STAGGER * 2, DEFAULT_DURATION * 2, 2);
	    break;
	case 5:
	    set_top_and_bottom_reversed();
	    set_staggered_pixels(DEFAULT_STAGGER * 2, DEFAULT_DURATION * 2, 2);
	    break;
	case 6:
	    set_inorder_squares();
	    set_staggered_pixels(DEFAULT_DURATION * COLUMN_COUNT,
				 DEFAULT_DURATION * COLUMN_COUNT, COLUMN_COUNT);
	    break;
	case 7:
	    set_spiral();
	    break;
#if 0
// Disable not working ones.
	case 7:
	    set_inner_spiral();
	    break;
	case 8:
	    set_inorder_2x2();
	    set_staggered_pixels(DEFAULT_STAGGER, DEFAULT_DURATION, 4);
	    break;
#endif
    }
    ++sequence;
#else
	    set_inner_spiral();
	    set_staggered_pixels(DEFAULT_STAGGER, DEFAULT_DURATION);
#endif
    
    g_start_time = millis();
}
#endif

bool Fader::display()
{
    int now = millis();
    int t = now - g_start_time;

#if 0
    Serial.print("t ");
    Serial.print(t);
    Serial.print(" total ");
    Serial.print(exact_end());
    Serial.println();
#endif

    bool done = true;
    for (int led = 0; led < LED_COUNT; ++led) {
	// Determine fade state.
	Colour initial = get_initial(led);
	Colour final = get_final(led);
	int order = get_order(led);
	int start = fade_start(order);
	int end = fade_end(order);

#if 0
	Serial.print("led ");
	Serial.print(led);
	Serial.print(" order ");
	Serial.print(order);
	Serial.print(" start ");
	Serial.print(start);
	Serial.print(" end ");
	Serial.print(end);
	Serial.println();
#endif

	// Calculate colour.
	Colour c;
	if (t < start) {
	    c = initial;
	    done = false;
	} else if (t >= end) {
	    c = final;
	} else {
	    c = linear_fade(initial, final, t - start, end - start);
	    done = false;
	}

	draw_pixel(led, c);
    }

    if (done) {
#if 1
	reset(); 
#else
	for (int led = 0; led < LED_COUNT; ++led) {
	    g_initial[led] = make_rgb(g_brightness.get(), 0, 0);
	}
	g_start_time = now;
#endif
    }

    return true;
}

void Fader::set_fade_down()
{
    for (int led = 0; led < LED_COUNT; ++led) {
	g_initial[led] = get_pixel(led);
	g_final[led] = COLOUR_BLACK;
    }
}

void Fader::set_fade_up()
{
    // TODO
}

void Fader::set_shuffled_squares()
{
    make_shuffled_array(g_order, LED_COUNT);
}

// 0123
// 4567
// 89ab
// cdef
void Fader::set_inorder_squares()
{
    Simple_counter y(COLUMN_COUNT, true);
    Simple_counter x(ROW_COUNT, true);
    For_each_led g(y, x);

    g.fill_array(g_order, LED_COUNT);
}

// looks bad
// 0123
// 89ab
// cdef
// 4567
void Fader::set_alternate_ends_squares()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x(COLUMN_COUNT, true);
    For_each_led g(y, x);

    g.fill_array(g_order, LED_COUNT);
}

// 0123
// 7654
// 89ab
// fedc
void Fader::set_back_forth_squares()
{
    Simple_counter y(COLUMN_COUNT, true);
    Simple_counter x1(ROW_COUNT, true);
    Simple_counter x2(ROW_COUNT, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x);

    g.fill_array(g_order, LED_COUNT);
}

// 0123
// 89ab
// fedc
// 7654
void Fader::set_alternate_back_forth_squares()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Alternator y(y1, y2);
    Simple_counter x1(COLUMN_COUNT, true);
    Simple_counter x2(COLUMN_COUNT, false);
    Concatenator x(x1, x2, true);
    For_each_led g(y, x);

    g.fill_array(g_order, LED_COUNT);
}

// 0246
// 8ace
// 9bdf
// 1357
void Fader::set_top_and_bottom()
{
    Simple_counter y1(ROW_COUNT / 2, true);
    Simple_counter x1(COLUMN_COUNT, true);
    For_each_led g1(y1, x1);

    Counter y2(ROW_COUNT - 1, ROW_COUNT / 2, -1);
    Simple_counter x2(COLUMN_COUNT, true);
    For_each_led g2(y2, x2);

    Alternator g(g1, g2);

    g.fill_array(g_order, LED_COUNT);
}

// 0246
// 8ace
// fdb9
// 7531
void Fader::set_top_and_bottom_reversed()
{
    int x_pos = 0;
    int y_pos = 0;

    int x_delta = 1;
    for (int order = 0; order < LED_COUNT; order += 2) {
	g_order[get_led(x_pos, y_pos)] = order;
	g_order[get_led(flip_x(x_pos), flip_y(y_pos))] = order + 1;
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
void Fader::set_spiral()
{
    int x_pos = 0;
    int y_pos = 0;

    int x_delta = 1;
    int y_delta = 0;

    int x_end = COLUMN_COUNT - 1;
    int y_end = ROW_COUNT - 1;
    for (int order = 0; order < LED_COUNT; ++order) {
	g_order[get_led(x_pos, y_pos)] = order;
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
void Fader::set_inner_spiral()
{
    set_spiral();
    reverse_array(g_order, LED_COUNT);
}

// 0145
// 2367
// 89cd
// abef
void Fader::set_inorder_2x2()
{
    int x = 0;
    int y = 0;

    int x_pos = 0;
    int y_pos = 0;
    int length = 2;

    int x_delta = 1;
    for (int order = 0; order < LED_COUNT / (length * length); ++order) {
	for (int x_pos = length * x; x_pos < length * (x + 1); ++x_pos) {
	    for (int y_pos = length * y; y_pos < length * (y + 1); ++y_pos) {
		g_order[get_led(x_pos, y_pos)] = order;
	    }
	}
	x += x_delta;
	if (x == COLUMN_COUNT / length) {
	    ++y_pos;
	}
    }
}

// TODO: Consider just setting things to the same order.
void Fader::set_staggered_pixels(int stagger, int duration, int count)
{
#if 1
    for (int order = 0, rank = 0; order < LED_COUNT; order += count, ++rank) {
	for (int i = 0; i < count; ++i) {
	    g_fade_start[order + i] = stagger * rank;
	    g_fade_duration[order + i] = duration;
	}
    }
#else
    for (int order = 0; order < LED_COUNT; ++order) {
	g_fade_start[order] = stagger * order;
	g_fade_duration[order] = duration;
    }
#endif
}

Colour Fader::get_initial(int led)
{
    return g_initial[led];
}

Colour Fader::get_final(int led)
{
    return g_final[led];
}

int Fader::get_order(int led)
{
    return g_order[led];
}

int Fader::fade_start(int order)
{
    return g_fade_start[order];
}

int Fader::fade_duration(int order)
{
    return g_fade_duration[order];
}

int Fader::fade_end(int order)
{
    return fade_start(order) + fade_duration(order);
}

int Fader::linear_end()
{
    return fade_end(LED_COUNT - 1);
}

int Fader::exact_end()
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
