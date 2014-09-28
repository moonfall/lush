#if 0
#include "lush.h"

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
    g_modes[g_modes_count++].m_pattern = new Pattern_border;
    g_modes[g_modes_count++].m_pattern = new Pattern_counter;
    g_modes[g_modes_count++].m_pattern = new Pattern_dropper;
    g_modes[g_modes_count++].m_pattern = new Pattern_heart;
    g_modes[g_modes_count++].m_pattern = new Pattern_huey;
    g_modes[g_modes_count++].m_pattern = new Pattern_line;
    g_modes[g_modes_count++].m_pattern = new Pattern_marquee;
    g_modes[g_modes_count++].m_pattern = new Pattern_maze(g_fader1);
#if 0
    // Allocate plasma causes a hang at startup.
    g_modes[g_modes_count++].m_pattern = new Pattern_plasma;
    g_modes[g_modes_count++].m_pattern = new Pattern_pulse;
    g_modes[g_modes_count++].m_pattern = new Pattern_race;
    g_modes[g_modes_count++].m_pattern = new Pattern_rain;
    g_modes[g_modes_count++].m_pattern = new Pattern_random_fader(g_fader1);
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

    g_configs = new Mode[MAX_CONFIG_OPTIONS];
#if 0
    g_configs[g_config_count++].m_pattern = new Pattern_option("BR", g_brightness, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("G0", g_gain0, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("G1", g_gain1, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("N", g_number, true);
    g_configs[g_config_count++].m_pattern = new Pattern_option("UP", g_up, true);
#endif

    g_main_modes = new Mode[MAX_MAIN_MODES];
    g_main_modes[g_main_modes_count++] = new Pattern_random(g_modes, g_modes_count);
    g_main_modes[g_main_modes_count++] = new Pattern_selector(g_modes, g_modes_count);
    g_main_modes[g_main_modes_count++] = new Pattern_config(g_configs, g_config_count);
    g_main_modes[g_main_modes_count++] = new Pattern_off();
#endif

    return new Pattern_huey();
    return new Pattern_main_menu(g_main_modes, g_main_modes_count);
}
#endif
