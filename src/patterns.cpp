#include "lush.h"
#include "patterns.h"

const int MAX_MODES = 64;
const int MAX_CONFIG_OPTIONS = 8;
const int MAX_MAIN_MODES = 8;

static Fader_static g_fader1;

static Mode *g_modes = NULL;
int g_modes_count = 0;

Mode *g_configs = NULL;
int g_config_count = 0;

Mode *g_main_modes = NULL;
int g_main_modes_count = 0;

Pattern *setup_patterns()
{
    g_modes = new Mode[MAX_MODES];
    g_modes[g_modes_count++].m_pattern = new Pattern_alphabet();
    // g_modes[g_modes_count++].m_pattern = new Pattern_border;
    g_modes[g_modes_count++].m_pattern = new Pattern_counter;
    g_modes[g_modes_count++].m_pattern = new Pattern_dropper;
    g_modes[g_modes_count++].m_pattern = new Pattern_heart;
    Pattern_huey *huey = new Pattern_huey;
    g_modes[g_modes_count++].m_pattern = huey;
    g_modes[g_modes_count++].m_pattern = new Pattern_marquee;
    g_modes[g_modes_count++].m_pattern = new Pattern_maze(g_fader1);
    g_modes[g_modes_count++].m_pattern = new Pattern_plasma;
    // g_modes[g_modes_count++].m_pattern = new Pattern_pulse;
    // g_modes[g_modes_count++].m_pattern = new Pattern_race;
    g_modes[g_modes_count++].m_pattern = new Pattern_rain;
    g_modes[g_modes_count++].m_pattern = new Pattern_random_fader(g_fader1);
    Pattern_snake *snake = new Pattern_snake;
    g_modes[g_modes_count++].m_pattern = snake;
    g_modes[g_modes_count++].m_pattern = new Pattern_spinner;
#if 0
    g_modes[g_modes_count++].m_pattern = new Pattern_synthesia_fire;
#endif
    g_modes[g_modes_count++].m_pattern = new Pattern_synthesia_plasma_complex;
    g_modes[g_modes_count++].m_pattern = new Pattern_wheel;
#ifndef DISABLE_AUDIO
    g_modes[g_modes_count++].m_pattern = new Pattern_peak_diagonal;
#if 0
    g_modes[g_modes_count++].m_pattern = new Pattern_peak_ordered;
#endif
    g_modes[g_modes_count++].m_pattern = new Pattern_peak_noise(g_fader1);
    g_modes[g_modes_count++].m_pattern = new Pattern_peak_spike;
    g_modes[g_modes_count++].m_pattern = new Pattern_spectrum_bars;
    g_modes[g_modes_count++].m_pattern = new Pattern_spectrum_field;
    g_modes[g_modes_count++].m_pattern = new Pattern_spectrum_timeline;
#endif

    // Composite patterns
    g_modes[g_modes_count++].m_pattern = new Pattern_composite_pair(
	Mode(huey), Mode(snake, (void *) Pattern_snake::SNAKE_BLACK));

    g_configs = new Mode[MAX_CONFIG_OPTIONS];
    g_configs[g_config_count++].m_pattern = new Pattern_option("BR", g_brightness, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("G0", g_gain0, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("G1", g_gain1, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("N", g_number, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("UP", g_up, true);

    g_main_modes = new Mode[MAX_MAIN_MODES];
    g_main_modes[g_main_modes_count].m_id = "R";
    g_main_modes[g_main_modes_count++].m_pattern = new Pattern_random(g_modes, g_modes_count);
    g_main_modes[g_main_modes_count].m_id = "S";
    g_main_modes[g_main_modes_count++].m_pattern = new Pattern_selector(g_modes, g_modes_count);
    g_main_modes[g_main_modes_count].m_id = "C";
    g_main_modes[g_main_modes_count++].m_pattern = new Pattern_config(g_configs, g_config_count);
    g_main_modes[g_main_modes_count].m_id = "";
    g_main_modes[g_main_modes_count++].m_pattern = new Pattern_off();

    for (int i = 0; i < g_modes_count; ++i) {
        g_modes[i].m_pattern->setup();
    }
    for (int i = 0; i < g_config_count; ++i) {
        g_configs[i].m_pattern->setup();
    }
    for (int i = 0; i < g_main_modes_count; ++i) {
        g_main_modes[i].m_pattern->setup();
    }

    Pattern *pattern = new Pattern_main_menu(g_main_modes, g_main_modes_count);
    Serial.printf("main menu %p\n", pattern);
    return pattern;
}
