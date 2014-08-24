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

#define SWAPPED_ENCODERS
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

// Current state
Fader_static g_fader1;
Pattern_random_fader g_random_fader(g_fader1);
Pattern_alphabet g_pattern_alphabet;
Pattern_border g_pattern_border;
Pattern_counter g_pattern_counter;
Pattern_dropper g_pattern_dropper;
Pattern_heart g_pattern_heart;
Pattern_huey g_pattern_huey;
Pattern_line g_pattern_line;
Pattern_marquee g_pattern_marquee;
Pattern_maze g_pattern_maze(g_fader1);
Pattern_peak g_pattern_peak;
Pattern_plasma g_pattern_plasma;
Pattern_pulse g_pattern_pulse;
Pattern_rain g_pattern_rain;
#ifndef DISABLE_AUDIO
Pattern_spectrum_bars g_pattern_spectrum_bars;
Pattern_spectrum_field g_pattern_spectrum_field;
Pattern_spectrum_timeline g_pattern_spectrum_timeline;
Pattern_synthesia_fire g_pattern_synthesia_fire;
#endif
Pattern_synthesia_plasma_complex g_pattern_synthesia_plasma_complex;
Pattern_wheel g_pattern_wheel;

// Modes:
// - weighted random mode
// - tour
// - select specific mode
// - configuration
struct Mode g_modes[] = {
    { &g_pattern_marquee },
    { &g_pattern_alphabet },
    { &g_pattern_plasma },
    { &g_pattern_maze },
    { &g_random_fader },
    { &g_pattern_heart },
    { &g_pattern_line },
    { &g_pattern_dropper },
    { &g_pattern_border },
    { &g_pattern_huey },
    { &g_pattern_rain },
#if 0
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_CURRENT_HUE },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_RANDOM_COLOUR },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_SINGLE_RANDOM_COLOUR },
    { &g_pattern_rain, (void *) Pattern_rain::RAIN_PURE_WHITE },
#endif
#ifndef DISABLE_AUDIO
    { &g_pattern_peak },
#endif
    { &g_pattern_counter },
    { &g_pattern_pulse },
    { &g_pattern_wheel },
#ifndef DISABLE_AUDIO
    { &g_pattern_spectrum_bars },
#if 0
    { &g_pattern_spectrum_field },
    { &g_pattern_spectrum_timeline },
#endif
#endif
#if 0
    { &g_pattern_synthesia_fire },
#endif
    { &g_pattern_synthesia_plasma_complex },
};
const int MODE_COUNT = sizeof(g_modes) / sizeof(g_modes[0]);

Pattern_random g_pattern_random(g_modes, MODE_COUNT);
Pattern_static g_pattern_static(g_modes, MODE_COUNT);
Pattern_off g_pattern_off;
struct Mode g_main_modes[] = {
    { &g_pattern_random, NULL, "R" },
    { &g_pattern_static, NULL, "S" },
    { &g_pattern_off, NULL, "" },
};
const int MAIN_MODE_COUNT = sizeof(g_main_modes) / sizeof(g_main_modes[0]);
Pattern_main_menu g_pattern_main_menu(g_main_modes, MAIN_MODE_COUNT);

Pattern *g_root = &g_pattern_main_menu;

Value g_brightness(16, 0, 255);
Value g_resume_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);

// Audio gain control
const int INITIAL_GAIN = 165;
Value g_gain0(INITIAL_GAIN, 0, 255);
Value g_gain1(INITIAL_GAIN, 0, 255);

// Audio acquisition
#ifndef DISABLE_AUDIO
AudioInputAnalog g_audio_input;
#define COEFF(x) ((int) (x * (float) (1 << 30)))
// http://forum.pjrc.com/threads/24793-Audio-Library?p=40179&viewfull=1#post40179
// http://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
int g_hp_filter_params[] = {
// highpass, 1000Hz, Q=0.707
#if 0
    COEFF(0.9041514120481504),
    COEFF(-1.8083028240963008),
    COEFF(0.9041514120481504),
    -COEFF(-1.7990948352036202),
    -COEFF(0.8175108129889815),
#endif

    // highpass, 3000Hz, Q=0.707
#if 1
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
#if 0
AudioConnection g_audio_conn1(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn2(g_hp_filter, g_fft);
AudioConnection g_audio_conn3(g_audio_input, g_peak);
#endif
#if 0
AudioConnection g_audio_conn1(g_audio_input, g_fft);
AudioConnection g_audio_conn2(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn3(g_hp_filter, g_peak);
#endif
#if 1
AudioConnection g_audio_conn1(g_audio_input, g_fft);
AudioConnection g_audio_conn2(g_audio_input, g_peak);
#endif
#endif

#define OCTOWS2811_PEAK_HACK
#ifdef OCTOWS2811_PEAK_HACK
unsigned g_peak_updates_to_ignore = 0;
#endif
unsigned g_current_peak = 0;

#ifndef DISABLE_AUDIO
Sample_type g_magnitudes[MAGNITUDE_COUNT];
Bin_type g_bins[MAX_BIN_COUNT];

#ifdef MAGNITUDE_AVERAGE
static int32_t g_magnitude_sums[MAGNITUDE_COUNT];
const int g_magnitude_avg_count = 1000;
static int g_magnitude_avg_gathered = 0;
#endif
#endif

const float GAIN_R1 = 10000;
const float GAIN_R2 = 100000;
const float GAIN_RAB = 100000;
const float GAIN_RS_COUNT = 256;
const float GAIN_RS_TYP = GAIN_RAB / GAIN_RS_COUNT;

Value g_min_db(55.0);
Value g_max_db(65.0);
Value g_bin_count(8);

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

    // Set up ADC and audio input.
#ifndef DISABLE_AUDIO
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
    g_pattern_static.setup();
    g_pattern_random.setup();
    g_root->activate(NULL);

    // Start cycling colours by default
    g_hue.set_velocity(256, 10000);
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
				g_magnitudes[i]);
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
#if 1
	    Serial.print(" mags");
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		Serial.print(" ");
#ifdef MAGNITUDE_AVERAGE
		Serial.print(g_magnitude_sums[i] / g_magnitude_avg_count, 0);
#else
		Serial.print(i);
		Serial.print("=");
		Serial.print(g_magnitudes[i]);
#endif
	    }
#endif
	    Serial.print(" bins");
	    for (int i = 0; i < g_bin_count.get(); ++i) {
		Serial.print(" ");
		Serial.print(g_bins[i]);
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

float calculate_floor(float gain, int bin)
{
    float intercept = GAIN_INTERCEPTS[bin];
    float slope = GAIN_SLOPES[bin];
    return powf(10, sqrt(gain) * slope + intercept);
}

// g_magnitudes[MAGNITUDE_COUNT] -> g_bins[g_bin_count.get()]
void fft_reduce()
{
    g_magnitudes[0] /= 100;
    g_magnitudes[1] /= 50;
    g_magnitudes[33] /= 35;
    g_magnitudes[34] /= 100;
    g_magnitudes[35] /= 50;
    g_magnitudes[36] /= 2;
    g_magnitudes[67] /= 10;
    g_magnitudes[68] /= 20;
    g_magnitudes[69] /= 15;
    g_magnitudes[85] /= 6;
    g_magnitudes[86] /= 6;
    g_magnitudes[101] /= 8;
    g_magnitudes[102] /= 20;
    g_magnitudes[103] /= 20;
    g_magnitudes[104] /= 5;
    g_magnitudes[118] /= 2;
    g_magnitudes[119] /= 9;
    g_magnitudes[120] /= 9;

#if 1
    const int nsum[16] = {1, 1, 2, 2, 3, 4, 5, 6, 6, 8, 12, 14, 16, 18, 18, 24};
    for (int i = 0; i < g_bin_count.get(); ++i ) {
	g_bins[i] = 0;
    }
    int n = 0;
    int count = 0;
    for (int i = 1; i < MAGNITUDE_COUNT; ++i) {
	g_bins[n] += g_magnitudes[i];
	++count;
	if (count >= nsum[n]) {
	    ++n;
	    if (n >= min(g_bin_count.get(),
			 (int) (sizeof(nsum) / sizeof(nsum[0])))) {
		break;
	    }
	    count = 0;
	}
    }

#if 0
    int scale = 2 + 2048 / 7;
    for (int i = 0; i < g_bin_count.get(); ++i) {
	g_bins[i] = min(g_bins[i] / scale, 8);
    }
#endif

#endif
	

#if 0
#if 0
    int bin_size = MAGNITUDE_COUNT / g_bin_count.get();
#ifdef LOGARITHMIC_BINS
    bin_size = 1;
#endif
    Sample_type *src = g_magnitudes + 1;
    Sample_type *src_end = g_magnitudes + MAGNITUDE_COUNT;
    for (int i = 0; i < g_bin_count.get() && src < src_end; ++i) {
	g_bins[i] = 0;
	for (int j = 0; j < bin_size && src < src_end; ++j, ++src) {
	    g_bins[i] += *src;
	}
	g_bins[i] /= bin_size;
#ifdef LOGARITHMIC_BINS
	bin_size *= 2;
#endif
#ifdef F32
	g_bins[i] = 20.0 * log10(g_bins[i]);
	g_bins[i] -= (Sample_type) g_min_db.get();
	g_bins[i] = g_bins[i] < 0.0 ? 0.0 : g_bins[i];
	g_bins[i] /= (Sample_type) (g_max_db.get() - g_min_db.get());
	g_bins[i] = g_bins[i] > 1.0 ? 1.0 : g_bins[i];
#endif
    }
#else
    const float scale = 0.05;
    const float gamma = 2.0;
    const float smoothing_factor = 0.00007;
    const float smoothing = powf(smoothing_factor, (float) FFT_SIZE / 60.0);
    int f_start = 1;
    for (int i = 0; i < g_bin_count.get(); ++i) {
	int f_end = round(powf(((float)(i + 1)) / (float) g_bin_count.get(),
			       gamma) * MAGNITUDE_COUNT);
	if (f_end > MAGNITUDE_COUNT) {
	    f_end = MAGNITUDE_COUNT;
	}
	int f_width = f_end - f_start;
	if (f_width <= 0) {
	    f_width = 1;
	}
#if 0
	Serial.print("bin ");
	Serial.print(i);
	Serial.print(" width ");
	Serial.print(f_width);
#endif

	float bin_power = 0.0;
	for (int j = 0; j < f_width; ++j) {
	    float p = (float) g_magnitudes[f_start + j];
	    if (p > bin_power) {
		bin_power = p;
	    }
	}
#if 0
	Serial.print(" power ");
	Serial.print(bin_power);
#endif

#if 0
	bin_power = log(bin_power);
	if (bin_power < 0.0) {
	    bin_power = 0.0;
	}

	g_bins[i] = g_bins[i] * smoothing +
		    (bin_power * scale * (1.0 - smoothing));
#endif
#if 0
	bin_power = 20.0 * log10(bin_power);
	bin_power -= (Sample_type) g_min_db.get();
	bin_power = bin_power < 0.0 ? 0.0 : bin_power;
	bin_power /= (Sample_type) (g_max_db.get() - g_min_db.get());
	bin_power = bin_power > 1.0 ? 1.0 : bin_power;
	g_bins[i] = bin_power;
#endif
#if 0
	bin_power = 20.0 * log10(bin_power);
	bin_power -= (Sample_type) g_min_dbs[i];
	bin_power = bin_power < 0.0 ? 0.0 : bin_power;
	bin_power /= (Sample_type) (g_max_dbs[i] - g_min_dbs[i]);
	bin_power = bin_power > 1.0 ? 1.0 : bin_power;
	g_bins[i] = bin_power;
#endif
#if 0
	bin_power = 20.0 * log10(bin_power);
	g_bins[i] = bin_power;
#endif

#if 0
	Serial.print(" logpower ");
	Serial.print(bin_power);
	Serial.println();
#endif

	f_start = f_end;
    }
#endif

#ifdef LOG_BINS
    Serial.print("dc=");
    Serial.print(g_dc_total / g_dc_sample_count);
    for (int i = 0; i < g_bin_count.get(); ++i) {
	Serial.print(" ");
	Serial.print(g_bins[i]);
    }
    Serial.println();
#endif
#if 0
    static int last_report = 0;
    int now = millis();
    if (last_report + 300 > now) {
	last_report = now;
	Serial.print("bin=");
	Serial.print(g_current_bin);
	for (int i = 0; i < g_bin_count.get(); ++i) {
	    Serial.print(" ");
	    Serial.print(g_bins[i]);
	}
	Serial.println();
    }
#endif
#endif
}
#endif

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

