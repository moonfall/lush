#undef DISABLE_DISPLAY
#undef DISABLE_AUDIO

#ifndef DISABLE_AUDIO
#include <SD.h>
#include <Wire.h>
#include <Audio.h>
#endif

#include <arm_math.h>
#include <stdint.h>

#include <Encoder.h>
#include <SPI.h>
#include <OctoWS2811.h>

#include "lush.h"
#include "dspinst.h"

#undef SAMPLE_TEST
#undef PROFILE_FFT
#undef LOG_MAGNITUDES
#undef LOG_BINS
#undef LOG_SUMMARY
#undef MAGNITUDE_AVERAGE

// Pin configuration
const int POWER_LED_PIN = 13;

// A4 == external, A3 == adafruit internal
const int AUDIO_INPUT_PIN = A3;
// Bits of resolution for ADC
const int ANALOG_READ_RESOLUTION = 12;
// Number of samples to average with each ADC reading.
#if 1
const int ANALOG_READ_AVERAGING = 16;
#else
const int ANALOG_READ_AVERAGING = 1;
#endif

const int MCP4261_CS_PIN = 10;

#undef SWAPPED_ENCODERS
#ifdef SWAPPED_ENCODERS
const int ENCODER_1_A_PIN = 19;
const int ENCODER_1_B_PIN = 22;
const int ENCODER_1_SW_PIN = 23;
const int ENCODER_2_A_PIN = 0;
const int ENCODER_2_B_PIN = 1;
const int ENCODER_2_SW_PIN = 9;
#else
const int ENCODER_1_A_PIN = 0;
const int ENCODER_1_B_PIN = 1;
const int ENCODER_1_SW_PIN = 9;
const int ENCODER_2_A_PIN = 19;
const int ENCODER_2_B_PIN = 22;
const int ENCODER_2_SW_PIN = 23;
#endif

// Constants
const uint32_t TURN_OFF_MS = 3000;
const uint32_t BUTTON_DEBOUNCE_MS = 5;
const int INITIAL_GAIN = 165;
const int INITIAL_FFT_GAIN = 100;
const int FFT_GAIN_DENOMINATOR = 1000;
const int MAX_FFT_GAIN = 10000;

// Config options
Value g_brightness(16, 0, 255);
Value g_resume_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);
Value g_number(0, 0, 1000, true);
Value g_up(DIR_UP, 0, 3, true);

// Audio gain control
Value g_gain0(INITIAL_GAIN, 0, 255);
Value g_gain1(INITIAL_GAIN, 0, 255);

// This controls the amount that the FFT bins are scaled by:
// 100 == 1.0
// TODO: find a better name
Value g_fft_gain(INITIAL_FFT_GAIN, 0, MAX_FFT_GAIN);

// Shared state state
Fader_static g_fader1;

// Patterns
Pattern_alphabet g_pattern_alphabet;
Pattern_border g_pattern_border;
Pattern_counter g_pattern_counter;
Pattern_dropper g_pattern_dropper;
Pattern_heart g_pattern_heart;
Pattern_huey g_pattern_huey;
Pattern_line g_pattern_line;
Pattern_marquee g_pattern_marquee;
Pattern_maze g_pattern_maze(g_fader1);
Pattern_plasma g_pattern_plasma;
Pattern_pulse g_pattern_pulse;
Pattern_race g_pattern_race;
Pattern_rain g_pattern_rain;
Pattern_random_fader g_random_fader(g_fader1);
#if 0
Pattern_synthesia_fire g_pattern_synthesia_fire;
#endif
Pattern_synthesia_plasma_complex g_pattern_synthesia_plasma_complex;
Pattern_wheel g_pattern_wheel;
#ifndef DISABLE_AUDIO
Pattern_peak_diagonal g_pattern_peak_diagonal;
#if 0
Pattern_peak_ordered g_pattern_peak_ordered(g_fader1);
#endif
Pattern_peak_noise g_pattern_peak_noise(g_fader1);
Pattern_peak_spike g_pattern_peak_spike;
Pattern_spectrum_bars g_pattern_spectrum_bars;
Pattern_spectrum_field g_pattern_spectrum_field;
Pattern_spectrum_timeline g_pattern_spectrum_timeline;
#endif

// Modes:
// - weighted random mode
// - tour
// - select specific mode
// - configuration
struct Mode g_modes[] = {
    { &g_pattern_alphabet },
    { &g_pattern_border },
    { &g_pattern_counter },
    { &g_pattern_dropper },
    { &g_pattern_heart },
    { &g_pattern_huey },
    { &g_pattern_line },
    { &g_pattern_marquee },
    { &g_pattern_maze },
    { &g_pattern_plasma },
    { &g_pattern_pulse },
    { &g_pattern_race },
    { &g_pattern_rain },
#if 0
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_CURRENT_HUE },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_RANDOM_COLOUR },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_SINGLE_RANDOM_COLOUR },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_PURE_WHITE },
#endif
    { &g_random_fader },
#if 0
    { &g_pattern_synthesia_fire },
#endif
    { &g_pattern_synthesia_plasma_complex },
    { &g_pattern_wheel },
#ifndef DISABLE_AUDIO
    { &g_pattern_peak_diagonal },
    { &g_pattern_peak_noise },
#if 0
    { &g_pattern_peak_ordered },
#endif
    { &g_pattern_peak_spike },
    { &g_pattern_spectrum_bars },
    { &g_pattern_spectrum_field },
    { &g_pattern_spectrum_timeline },
#endif
};
const int MODE_COUNT = sizeof(g_modes) / sizeof(g_modes[0]);

Pattern_option g_option_brightness("BR", g_brightness, true);
Pattern_option g_option_gain0("G0", g_gain0, true);
Pattern_option g_option_gain1("G1", g_gain1, true);
#if 0
Pattern_option g_option_fft_gain("FG", g_fft_gain, true);
#endif
Pattern_option g_option_number("N", g_number, true);
Pattern_option g_option_up("UP", g_up, true);
struct Mode g_config_options[] = {
    { &g_option_brightness },
    { &g_option_gain0 },
    { &g_option_gain1 },
#if 0
    { &g_option_fft_gain },
#endif
    { &g_option_number },
    { &g_option_up },
};
const int CONFIG_OPTION_COUNT = sizeof(g_config_options) /
				sizeof(g_config_options[0]);

Pattern_random g_pattern_random(g_modes, MODE_COUNT);
Pattern_selector g_pattern_selector(g_modes, MODE_COUNT);
Pattern_config g_pattern_config(g_config_options, CONFIG_OPTION_COUNT);
Pattern_off g_pattern_off;
struct Mode g_main_modes[] = {
    { &g_pattern_random, NULL, "R" },
    { &g_pattern_selector, NULL, "S" },
    { &g_pattern_config, NULL, "C" },
    { &g_pattern_off, NULL, "" },
};
const int MAIN_MODE_COUNT = sizeof(g_main_modes) / sizeof(g_main_modes[0]);
Pattern_main_menu g_pattern_main_menu(g_main_modes, MAIN_MODE_COUNT);

Pattern *g_root = &g_pattern_main_menu;

// Audio acquisition
#ifndef DISABLE_AUDIO
AudioInputAnalog g_audio_input;
#define COEFF(x) ((int) (x * (float) (1 << 30)))
// http://forum.pjrc.com/threads/24793-Audio-Library?p=40179&viewfull=1#post40179
// http://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
int g_hp_filter_params[] = {
// highpass, 50Hz, Q=0.707
#if 0
    COEFF(0.9949753356833961),
    COEFF(-1.9899506713667923),
    COEFF(0.9949753356833961),
    -COEFF(-1.989925424437758),
    -COEFF(0.9899759182958264),
#endif

// highpass, 100Hz, Q=0.707
#if 0
    COEFF(0.9899759179893742),
    COEFF(-1.9799518359787485),
    COEFF(0.9899759179893742),
    -COEFF(-1.979851353142371),
    -COEFF(0.9800523188151258),
#endif

// highpass, 150Hz, Q=0.707
#if 0
    COEFF(0.9850016172570234),
    COEFF(-1.9700032345140468),
    COEFF(0.9850016172570234),
    -COEFF(-1.9697782746275025),
    -COEFF(0.9702281944005912),
#endif

// highpass, 200Hz, Q=0.707
#if 0
    COEFF(0.9800523027005293),
    COEFF(-1.9601046054010587),
    COEFF(0.9800523027005293),
    -COEFF(-1.9597066626643354),
    -COEFF(0.960502548137782),
#endif

// highpass, 250Hz, Q=0.707
#if 1
    COEFF(0.9751278424865795),
    COEFF(-1.950255684973159),
    COEFF(0.9751278424865795),
    -COEFF(-1.9496369766256763),
    -COEFF(0.9508743933206418),
#endif

// highpass, 1000Hz, Q=0.707
#if 0
    COEFF(0.9041514120481504),
    COEFF(-1.8083028240963008),
    COEFF(0.9041514120481504),
    -COEFF(-1.7990948352036202),
    -COEFF(0.8175108129889815),
#endif

    // highpass, 3000Hz, Q=0.707
#if 0
    COEFF( 0.7385371039326799 ),
    COEFF( -1.4770742078653598 ),
    COEFF( 0.7385371039326799 ),
    -COEFF( -1.407502284220597 ),
    -COEFF( 0.5466461315101225 ),
#endif

    // highpass, 10000Hz, Q=0.707
#if 0
    COEFF( 0.33699935872014053 ),
    COEFF( -0.6739987174402811 ),
    COEFF( 0.33699935872014053 ),
    -COEFF( -0.17124071441396285 ),
    -COEFF( 0.1767567204665992 ),
#endif
    0,
    0,
    0,
};

AudioFilterBiquad g_hp_filter(g_hp_filter_params);
#ifdef FFT1024
AudioAnalyzeFFT1024 g_fft;
#else
AudioAnalyzeFFT256 g_fft;
#endif
AudioPeak g_peak;
#define FILTER_AUDIO
#ifdef FILTER_AUDIO
AudioConnection g_audio_conn1(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn2(g_hp_filter, g_fft);
AudioConnection g_audio_conn3(g_audio_input, g_peak);
#else
AudioConnection g_audio_conn1(g_audio_input, g_fft);
AudioConnection g_audio_conn2(g_audio_input, g_peak);
#endif
#if 0
AudioConnection g_audio_conn1(g_audio_input, g_fft);
AudioConnection g_audio_conn2(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn3(g_hp_filter, g_peak);
#endif
#if 0
AudioConnection g_audio_conn1(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn2(g_hp_filter, g_fft);
AudioConnection g_audio_conn3(g_hp_filter, g_peak);
#endif
#endif

#undef OCTOWS2811_PEAK_HACK
#ifdef OCTOWS2811_PEAK_HACK
unsigned g_peak_updates_to_ignore = 0;
#endif
unsigned g_current_peak = 0;

#ifndef DISABLE_AUDIO
Sample_type g_magnitudes[MAGNITUDE_COUNT];
unsigned g_bin_count = 0;
float g_bin_scale = 1.0;
float g_fft_scale_factor = 0.0;
Bin_type g_bins[MAX_BIN_COUNT];
uint8_t g_bin_widths[MAX_BIN_COUNT];

#ifdef MAGNITUDE_AVERAGE
static int32_t g_magnitude_sums[MAGNITUDE_COUNT];
const int g_magnitude_avg_count = 100;
static int g_magnitude_avg_gathered = 0;
#endif
#endif

const float GAIN_R1 = 10000;
const float GAIN_R2 = 300000;
const float GAIN_RAB = 100000;
const float GAIN_RS_COUNT = 256;
const float GAIN_RS_TYP = GAIN_RAB / GAIN_RS_COUNT;

Value g_min_db(55.0);
Value g_max_db(65.0);

#if 1
float g_min_dbs[MAX_BIN_COUNT] = {
    55, 55, 55, 55, 55, 55, 55, 55,
};
float g_max_dbs[MAX_BIN_COUNT] = {
    65, 65, 65, 65, 65, 65, 65, 65,
};
int g_current_bin = 0;
#endif

// Output
// 24 bytes == 6 words for each LED of each strip.
DMAMEM int g_display_memory[LEDS_PER_STRIP * 6];
int g_drawing_memory[LEDS_PER_STRIP * 6];
OctoWS2811 g_octo(LEDS_PER_STRIP, g_display_memory, g_drawing_memory,
		  WS2811_GRB | WS2811_800kHz);
unsigned g_target_fps = 30;
uint32_t g_last_update = 0;

// UI state
Encoder encoder1(ENCODER_1_A_PIN, ENCODER_1_B_PIN);
Encoder encoder2(ENCODER_2_A_PIN, ENCODER_2_B_PIN);
UI_state g_ui;

void setup()
{
    Serial.begin(115200);

    // Set up SPI to allow control of digital pot for gain control.
    SPI.begin();

    delayMicroseconds(50);
    program_gain();

#ifndef DISABLE_AUDIO
    set_fft_bin_count(0);
    set_fft_scale_factor(0);

    // Set up ADC and audio input.
    AudioMemory(12);
    g_audio_input.begin(AUDIO_INPUT_PIN);
    g_peak.begin();
#endif

    pinMode(ENCODER_1_SW_PIN, INPUT_PULLUP);
    pinMode(ENCODER_2_SW_PIN, INPUT_PULLUP);

#if 0
    // Indicate power status.
    // DISABLED: POWER_LED_PIN is used for SPI.
    pinMode(POWER_LED_PIN, OUTPUT);
    digitalWrite(POWER_LED_PIN, HIGH);
#endif

    // Begin output.
    g_octo.begin();

    // TODO: Prevent multiple initialization of patterns.
    for (int i = 0; i < MODE_COUNT; ++i) {
    	g_modes[i].m_pattern->setup();
    }
    for (int i = 0; i < CONFIG_OPTION_COUNT; ++i) {
    	g_config_options[i].m_pattern->setup();
    }
    for (int i = 0; i < MAIN_MODE_COUNT; ++i) {
    	g_main_modes[i].m_pattern->setup();
    }
    g_root->activate(NULL);

    // Start cycling colours by default
    g_hue.set_velocity(256, 10000);

    // Cache the up value since it's always used
    g_up.set_callback(set_up_direction);
}

uint16_t spi_issue2(byte b1, byte b2)
{
  digitalWrite(MCP4261_CS_PIN, LOW);
  delayMicroseconds(20);
  byte high = SPI.transfer(b1);
  byte low = SPI.transfer(b2);
  delayMicroseconds(20);
  digitalWrite(MCP4261_CS_PIN, HIGH);
  delayMicroseconds(50);
  return high << 8 | low;
}

uint16_t set_wiper(int wiper, int pos)
{
  const int WR = B00 << 2;
  const int RD = B11 << 2;

  byte b1 = (wiper << 4) | WR | ((pos >> 8) & 0x3);
  byte b2 = pos & 0xff;
  uint16_t r = spi_issue2(b1, b2);

  b1 = (wiper << 4) | RD;
  b2 = 0xff;
  r = spi_issue2(b1, b2);
  uint16_t data = r & 0x1ff;

  return data;
}

void set_gain(int gain)
{
    set_gain(gain, gain);
}

void set_gain(int gain1, int gain2)
{
    g_gain0.set(gain1);
    g_gain1.set(gain2);
}

void adjust_gain(int adjustment)
{
    adjust_gain(adjustment, adjustment);
}

void adjust_gain(int adjustment1, int adjustment2)
{
    g_gain0.modify(adjustment1);
    g_gain1.modify(adjustment2);
}

void program_gain()
{
    int gain0 = set_wiper(0, g_gain0.get());
    int gain1 = set_wiper(1, g_gain1.get());
    Serial.print("gain set to ");
    Serial.print(gain0);
    Serial.print("/");
    Serial.println(gain1);

#ifdef MAGNITUDE_AVERAGE
    g_magnitude_avg_gathered = 0;
    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
	g_magnitude_sums[i] = 0;
    }
#endif
}

void adjust_fft_gain(int adjustment)
{
    g_fft_gain.modify(adjustment);
    program_fft_bin_scale();
}

void program_fft_bin_scale()
{
#ifndef DISABLE_AUDIO
    g_bin_scale = (float) g_fft_gain.get() * g_fft_scale_factor /
		  (float) FFT_GAIN_DENOMINATOR;
    Serial.printf("fft bin scale %.2f (gain %d/%d, factor %f)\n",
	          g_bin_scale, g_fft_gain.get(), FFT_GAIN_DENOMINATOR,
		  g_fft_scale_factor);
#endif
}

void set_target_fps(unsigned fps)
{
    g_target_fps = fps;
}

bool should_display()
{
    return !g_target_fps || millis() > g_last_update + (1000 / g_target_fps);
}

void loop()
{
    ui_loop();
#ifndef DISABLE_AUDIO
    sampler_loop();
#endif
    display_loop();
}

#if 0
void ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB1_ENCODER:
	    if (g_ui.m_knob2_button.get_current().m_value) {
		g_brightness.modify(element.get_current_change());
	    } else {
		g_gain0.modify(element.get_current_change());
		g_gain1.modify(element.get_current_change());
		program_gain();
	    }
	    break;

	case UI_KNOB1_BUTTON:
	    if (element.get_current_change() > 0) {
		// On press: start fading off when pressed
		g_resume_brightness = g_brightness;
		g_brightness.set_velocity(-g_brightness.get(), TURN_OFF_MS);
	    } else if (element.get_current_change() < 0) {
		// On release: turn off, or switch modes
		if (!g_off && g_brightness.get() == 0) {
		    turn_off();
		} else {
		    ui_advance_mode();
		}
		g_brightness = g_resume_brightness;
	    }
	    break;

	case UI_KNOB2_ENCODER:
#if 0
	    if (g_ui.m_knob2_button.get_current().m_value) {
		g_max_db.modify(element.get_current_change());
		if (g_max_db.get() <= g_min_db.get()) {
		    g_min_db.set(g_max_db.get() - 1);
		}
	    } else {
		g_min_db.modify(element.get_current_change());
		if (g_min_db.get() >= g_max_db.get()) {
		    g_max_db.set(g_min_db.get() + 1);
		}
	    }

	    Serial.print("db range ");
	    Serial.print(g_min_db.get());
	    Serial.print("-");
	    Serial.print(g_max_db.get());
	    Serial.println();
#else
	    if (g_ui.m_knob2_button.get_current().m_value) {
		g_max_dbs[g_current_bin] += element.get_current_change();
		if (g_max_dbs[g_current_bin] <= g_min_dbs[g_current_bin]) {
		    g_min_dbs[g_current_bin] = g_max_dbs[g_current_bin] - 1;
		}
	    } else {
		g_min_dbs[g_current_bin] += element.get_current_change();
		if (g_min_dbs[g_current_bin] >= g_max_dbs[g_current_bin]) {
		    g_max_dbs[g_current_bin] = g_min_dbs[g_current_bin] + 1;
		}
	    }

	    Serial.print("db range ");
	    Serial.print(g_current_bin);
	    Serial.print(" of ");
	    Serial.print(g_min_dbs[g_current_bin]);
	    Serial.print("-");
	    Serial.print(g_max_dbs[g_current_bin]);
	    Serial.println();
#endif
	    break;

	case UI_KNOB2_BUTTON:
	    // No action, just modify KNOB2 encoder
	    break;
    }
}
#endif

void ui_loop()
{
    while (Serial.available() > 0) {
	switch (Serial.read()) {
	    case 'b':
		g_brightness.modify(-1);
		break;
	    case 'B':
		g_brightness.modify(1);
		break;
#if 0
	    case 'm':
		ui_advance_mode();
		break;
#endif
	    case '0':
		g_current_bin = 0;
		break;
	    case '1':
		g_current_bin = 1;
		break;
	    case '2':
		g_current_bin = 2;
		break;
	    case '3':
		g_current_bin = 3;
		break;
	    case '4':
		g_current_bin = 4;
		break;
	    case '5':
		g_current_bin = 5;
		break;
	    case '6':
		g_current_bin = 6;
		break;
	    case '7':
		g_current_bin = 7;
		break;
	}
    }

    {
	Element &element = g_ui.m_knob1_encoder;
	Element_state state(encoder1.read());
	if (element.get_change(state, element.get_current())) {
	    element.push(state);

	    Serial.print("encoder1 new value is ");
	    Serial.print(state.m_value);
	    Serial.print(" change is ");
	    Serial.println(element.get_current_change());

	    g_root->ui_callback(UI_KNOB1_ENCODER, element);
	}
    }

    {
	Element &element = g_ui.m_knob1_button;
	int pin = ENCODER_1_SW_PIN;
	Element_state state(digitalRead(pin) == LOW);
	if (element.get_change(state, element.get_current()) &&
	    element.get_current_millis() >= BUTTON_DEBOUNCE_MS) {
	    element.push(state);

	    Serial.print("encoder1 ");
	    Serial.print(state.m_value ? "pressed" : "released");
	    Serial.print(" after ");
	    Serial.println(element.get_previous_millis());

	    g_root->ui_callback(UI_KNOB1_BUTTON, element);
	}
    }

    {
	Element &element = g_ui.m_knob2_encoder;
	Element_state state(encoder2.read());
	if (element.get_change(state, element.get_current())) {
	    element.push(state);

	    Serial.print("encoder2 new value is ");
	    Serial.print(state.m_value);
	    Serial.print(" change is ");
	    Serial.println(element.get_current_change());

	    g_root->ui_callback(UI_KNOB2_ENCODER, element);
	}
    }

    {
	Element &element = g_ui.m_knob2_button;
	int pin = ENCODER_2_SW_PIN;
	Element_state state(digitalRead(pin) == LOW);
	if (element.get_change(state, element.get_current()) &&
	    element.get_current_millis() >= BUTTON_DEBOUNCE_MS) {
	    element.push(state);

	    Serial.print("encoder2 ");
	    Serial.print(state.m_value ? "pressed" : "released");
	    Serial.print(" after ");
	    Serial.println(element.get_previous_millis());

	    g_root->ui_callback(UI_KNOB2_BUTTON, element);
	}
    }

    g_root->ui_hook();
}

#ifndef DISABLE_AUDIO
void sampler_loop()
{
#ifdef OCTOWS2811_PEAK_HACK
    if (g_octo.busy()) {
	g_peak_updates_to_ignore = 2;
    } else if (g_peak.update_completed_at) {
	if (g_peak_updates_to_ignore) {
	    --g_peak_updates_to_ignore;
	} else {
	    g_current_peak = max(g_current_peak, g_peak.Dpp());
	}
	g_peak.begin();
    }
#else
    g_current_peak = max(g_current_peak, g_peak.Dpp());
    g_peak.begin();
#endif

    bool available = g_fft.available();
    if (available) {
	for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
	    g_magnitudes[i] = g_fft.output[i];
	}

#ifdef LOG_MAGNITUDES
	for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
	    Serial.print(g_magnitudes[i]);
	    Serial.print(" ");
	}
	Serial.println();
#endif

	fft_reduce();

#ifdef LOG_SUMMARY
#ifdef MAGNITUDE_AVERAGE
	if (g_magnitude_avg_gathered < g_magnitude_avg_count) {
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		g_magnitude_sums[i] += g_magnitudes[i];
	    }
	} else {
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		running_average(g_magnitude_sums[i], g_magnitude_avg_count,
				(int32_t)g_magnitudes[i]);
	    }
	}
	++g_magnitude_avg_gathered;
#endif

	static uint32_t last_log = 0;
	if (millis() > last_log + 0) {
#ifdef MAGNITUDE_AVERAGE
	    Serial.print(g_magnitude_avg_gathered);
	    Serial.print(": ");
#endif
	    Serial.print("gain=");
	    Serial.print(g_gain0.get());
	    Serial.print("/");
	    Serial.print(g_gain1.get());
#if 0
	    Serial.print(" mags");
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		Serial.print(" ");
#ifdef MAGNITUDE_AVERAGE
		Serial.print(g_magnitude_sums[i] / g_magnitude_avg_count);
#else
#if 0
		Serial.print(i);
		Serial.print("=");
#endif
		Serial.print(g_magnitudes[i]);
#endif
	    }
#endif
	    Serial.print(" bins");
	    for (int i = 0; i < g_bin_count; ++i) {
		Serial.printf(" %u", g_bins[i]);
	    }
	    Serial.println();
	    last_log = millis();
	}
#endif
    }
}

float calculate_actual_gain(int wiper)
{
    return ((GAIN_RS_TYP * (float) wiper) + GAIN_R2) /
	   (((GAIN_RS_COUNT - (float) wiper) * GAIN_RS_TYP) + GAIN_R1) +
	   1;
}

const unsigned FIRST_BIN = 0;
void set_fft_bin_count(unsigned bin_count)
{
    g_bin_count = bin_count;

    // Most music is within 8kHz.
    // TODO: Adjust down to 4kHZ?
    int select_count = MAGNITUDE_COUNT * 100 / 275;

#define GAMMA_BINS
#ifdef GAMMA_BINS
    const float gamma = 2.0;
    int f_start = FIRST_BIN;
    for (unsigned i = 0; i < g_bin_count; ++i) {
	int f_end = round(powf(((float)(i + 1)) / (float) g_bin_count, gamma) *
		          select_count);
	if (f_end > select_count) {
	    f_end = select_count;
	}
	int f_width = f_end - f_start;
	if (f_width <= 0) {
	    f_width = 1;
	}
	g_bin_widths[i] = f_width;
	f_start = f_end;
    }
#endif

#ifdef EQUAL_BINS
    int bin_size = select_count / g_bin_count;
    for (unsigned i = 0; i < g_bin_count; ++i) {
	g_bin_widths[i] = bin_size;
    }
#endif

#ifdef LOGARITHMIC_BINS
    int bin_size = 1;
    for (unsigned i = 0; i < g_bin_count; ++i) {
	g_bin_width[i] = bin_size;
	bin_size *= 2;
    }
#endif
}

void set_fft_scale_factor(float scale_factor)
{
    g_fft_scale_factor = scale_factor;
    program_fft_bin_scale();
}

// g_magnitudes[MAGNITUDE_COUNT] -> g_bins[g_bin_count.get()]
// logN(magnitude) [min_power, max_power] -> [0, fft_scale]
// <min_mag == 0; >max_mag == fft_scale
// patterns: 0..ROW_COUNT ->fft_scale == ROW_COUNT + 1
// TODO: allow EQ by allowing individual channels to be adjusted?
// min_power, max_power: set by config [0..100]
// g_bin_count, g_fft_scale: set by pattern [0..1000]
// TODO: set scale below such that log_power and all thing are integers
// and support bounds as listed
// magnitude [0..65536] ??
// log_power [0..5]
// min_power [0..100]
// max_power [0..100]
// max_power - min_power [0..100]
// fft_scale [0..1000]
// stay under about 8kHz
void fft_reduce()
{
    if (g_bin_count == 0) {
	return;
    }

#ifdef USE_SMOOTHING
    const float smoothing_factor = 0.00007;
    const float smoothing = powf(smoothing_factor, (float) FFT_SIZE / 60.0);
#endif
    int f_start = FIRST_BIN;
    for (unsigned i = 0; i < g_bin_count; ++i) {
	int bin_power = 0;
	int bin_width = 0;
	for (bin_width = 0;
	     bin_width < g_bin_widths[i] &&
	     f_start + bin_width < MAGNITUDE_COUNT; ++bin_width) {
	    int p = g_magnitudes[f_start + bin_width];
#define MAX_POWER
#ifdef MAX_POWER
	    if (p > bin_power) {
		bin_power = p;
	    }
#else
	    // Average.
	    bin_power += p;
#endif
	}

#ifndef MAX_POWER
	// Average
	bin_power /= bin_width;
#endif

#if 0
	float log_power = g_bin_scale * log((float)bin_power);
#endif
#if 0
	float log_power = g_bin_scale * log10((float)bin_power);
#endif
#if 0
	float log_power = 20.0 * log10((float)bin_power);
#if 0
	log_power -= (Sample_type) g_min_dbs[i];
	log_power = log_power < 0.0 ? 0.0 : log_power;
	log_power /= (Sample_type) (g_max_dbs[i] - g_min_dbs[i]);
	log_power = log_power > 1.0 ? 1.0 : log_power;
#endif
#endif
#if 1
	// TODO: use me
	// log_power *= SCALE -- SCALE
	// such that g_min_power/g_max_power == 0..255 or 0..100
	float log_power = 100.0 * log10((float)bin_power);
	float g_min_power = 100;
	float g_max_power = 200;
	if (log_power < g_min_power) {
	    log_power = 0.0;
	} else if (log_power > g_max_power) {
	    log_power = g_fft_scale_factor;
	} else {
	    log_power = (log_power - g_min_power) * g_fft_scale_factor /
		        (g_max_power - g_min_power);
	}
#endif
#if 0
	Serial.printf("bin %d width %d %d-%d power %.3f %.3f\n",
		      i, bin_width, f_start, f_start + bin_width, bin_power,
		      log_power);
#endif
	if (log_power < 0.0) {
	    log_power = 0.0;
	}

#ifdef USE_SMOOTHING
	g_bins[i] = (int)((float) g_bins[i] * smoothing +
		          (log_power * (1.0 - smoothing)));
#else
	g_bins[i] = (int) log_power;
#endif

	f_start += bin_width;
    }

#ifdef LOG_BINS
    for (unsigned i = 0; i < g_bin_count; ++i) {
	Serial.print(g_bins[i]);
	Serial.print(" ");
    }
    Serial.println();
#endif
#if 0
    static int last_report = 0;
    int now = millis();
    if (last_report + 300 > now) {
	last_report = now;
	Serial.printf("bin=[mod %u]", g_current_bin);
	for (unsigned i = 0; i < g_bin_count; ++i) {
	    Serial.print(" ");
	    Serial.print(g_bins[i]);
	}
	Serial.println();
    }
#endif
}
#endif // DISABLE_AUDIO

void reset_peak()
{
    g_current_peak = 0;
}

unsigned get_peak()
{
    return g_current_peak;
}

unsigned get_mapped_peak(unsigned max_value)
{
    const unsigned MAX_VALUE = 65535;
    const unsigned NOISE_FLOOR = 19000;

    if (g_current_peak < NOISE_FLOOR) {
	return 0;
    }

    unsigned peak = g_current_peak - NOISE_FLOOR;

    return min(max_value,
	       peak * (max_value + 1) /
	       (unsigned) (MAX_VALUE - NOISE_FLOOR));
}

void display_loop()
{
#ifdef DISABLE_DISPLAY
    return;
#endif
    if (should_display() && g_root->display()) {
	show_pixels();
	g_last_update = millis();

	// TODO: Is this the best place?
	reset_peak();
    }
}

