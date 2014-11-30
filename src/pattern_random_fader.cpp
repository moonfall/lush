#include "patterns.h"

Pattern_random_fader::Pattern_random_fader(Fader_static &fader)
    : m_fader(fader), m_fade_pattern(0), m_start_time(0), m_fade_out(true)
{
}

void Pattern_random_fader::activate(void *arg)
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
    for (int y = 0; y < ROW_COUNT; ++y) {
	for (int x = 0; x < COLUMN_COUNT; ++x) {
	    bool done = false;
	    draw_pixel(x, y, m_fader.generate(get_led(x, y), t, &done));
	    if (!done) {
		all_done = false;
	    }
	}
    }

    if (all_done) {
	randomize(); 
    }

    return true;
}

