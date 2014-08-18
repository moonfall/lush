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
#define LOG_SUMMARY
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

const int ENCODER_1_A_PIN = 0;
const int ENCODER_1_B_PIN = 1;
const int ENCODER_1_SW_PIN = 9;
const int ENCODER_2_A_PIN = 19;
const int ENCODER_2_B_PIN = 22;
const int ENCODER_2_SW_PIN = 23;

// Constants
const int TURN_OFF_MS = 3000;

// Current state
Pattern *g_current_pattern = NULL;
Fader_static g_fader1;
Pattern_random_fader g_random_fader(g_fader1);
Pattern_counter g_pattern_counter;
Pattern_dropper g_pattern_dropper;
Pattern_heart g_pattern_heart;
Pattern_huey g_pattern_huey;
Pattern_maze g_pattern_maze(g_fader1);
Pattern_peak g_pattern_peak;
Pattern_plasma g_pattern_plasma;
Pattern_pulse g_pattern_pulse;
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
    { &g_pattern_peak },
    { &g_pattern_huey },
    { &g_pattern_spectrum_bars },
    { &g_pattern_dropper },
    { &g_pattern_maze },
    { &g_random_fader },
    { &g_pattern_plasma },
    { &g_pattern_heart },
    { &g_pattern_counter },
    { &g_pattern_pulse },
    { &g_pattern_wheel },
#ifndef DISABLE_AUDIO
#if 0
    { &g_pattern_spectrum_bars },
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
Value g_current_mode(0, 0, MODE_COUNT - 1, true);

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
AudioAnalyzeFFT256 g_fft;
AudioPeak g_peak;
#if 0
AudioConnection g_audio_conn1(g_audio_input, g_hp_filter);
AudioConnection g_audio_conn2(g_hp_filter, g_fft);
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

IntervalTimer g_sampling_timer;

int g_sample_rate_hz = 20000;
int g_sample_counter = 0;

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
bool g_force_update = false;

// UI state
Encoder encoder1(ENCODER_1_A_PIN, ENCODER_1_B_PIN);
Encoder encoder2(ENCODER_2_A_PIN, ENCODER_2_B_PIN);
UI_state g_ui;
bool g_off = false;

void setup()
{
    Serial.begin(115200);

    // Set up SPI to allow control of digital pot for gain control.
    SPI.begin();

    delayMicroseconds(50);
    set_gain();

    // Set up ADC and audio input.
#ifndef DISABLE_AUDIO
    AudioMemory(12);
    g_audio_input.begin(AUDIO_INPUT_PIN);
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
    update_pattern();

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

void set_gain()
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

void idle() {
    // TODO: Replace this with actual sleep.
    delay(10);
}

void loop()
{
    ui_loop();
#ifndef DISABLE_AUDIO
    sampler_loop();
#endif
    display_loop();
    if (g_off) {
	idle();
    }
}

void update_pattern()
{
    g_current_pattern = g_modes[g_current_mode.get()].m_pattern;
    g_current_pattern->activate();
    g_force_update = true;
}

void ui_advance_mode()
{
    if (g_off) {
	turn_on();
    }
    g_current_mode.modify(1);
    update_pattern();
    Serial.print("next mode is ");
    Serial.println(g_current_mode.get());
}

void ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB1_ENCODER:
	    if (g_ui.m_knob2_button.get_current().m_value) {
		g_brightness.modify(element.get_current_change());
	    } else {
		g_gain0.modify(element.get_current_change());
		g_gain1.modify(element.get_current_change());
		set_gain();
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
	    case 'm':
		ui_advance_mode();
		break;
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

	    ui_callback(UI_KNOB1_ENCODER, element);
	}
    }

    {
	Element &element = g_ui.m_knob1_button;
	int pin = ENCODER_1_SW_PIN;
	Element_state state(digitalRead(pin) == LOW);
	if (element.get_change(state, element.get_current())) {
	    element.push(state);

	    Serial.print("encoder1 ");
	    Serial.print(state.m_value ? "pressed" : "released");
	    Serial.print(" after ");
	    Serial.println(element.get_previous_millis());

	    ui_callback(UI_KNOB1_BUTTON, element);
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

	    ui_callback(UI_KNOB2_ENCODER, element);
	}
    }

    {
	Element &element = g_ui.m_knob2_button;
	int pin = ENCODER_2_SW_PIN;
	Element_state state(digitalRead(pin) == LOW);
	if (element.get_change(state, element.get_current())) {
	    element.push(state);

	    Serial.print("encoder2 ");
	    Serial.print(state.m_value ? "pressed" : "released");
	    Serial.print(" after ");
	    Serial.println(element.get_previous_millis());

	    ui_callback(UI_KNOB2_BUTTON, element);
	}
    }
}

#ifndef DISABLE_AUDIO
void sampler_loop()
{
    return;

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
	if (millis() > last_log + 100) {
#ifdef MAGNITUDE_AVERAGE
	    Serial.print(g_magnitude_avg_gathered);
	    Serial.print(": ");
#endif
	    Serial.print("gain=");
	    Serial.print(g_gain0.get());
	    Serial.print("/");
	    Serial.print(g_gain1.get());
	    Serial.print(" mags");
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		Serial.print(" ");
#ifdef MAGNITUDE_AVERAGE
		Serial.print(g_magnitude_sums[i] / g_magnitude_avg_count, 0);
#else
		Serial.print(g_magnitudes[i]);
#endif
	    }
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
#if 1
    const int nsum[16] = {1, 1, 2, 2, 3, 4, 5, 6, 6, 8, 12, 14, 16, 20, 28, 24};
    for (int i = 0; i < g_bin_count.get(); ++i ) {
	g_bins[i] = 0;
    }
    int n = 0;
    int count = 0;
    for (int i = 1; i < MAGNITUDE_COUNT; ++i) {
	g_bins[n] += g_magnitudes[i];
	++count;
	if (count > nsum[n]) {
	    ++n;
	    if (n >= 16) {
		break;
	    }
	    count = 0;
	}
    }

    int scale = 2 + 2048 / 7;
    for (int i = 0; i < g_bin_count.get(); ++i) {
	g_bins[i] = min(g_bins[i] / scale, 8);
    }

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

void display_loop()
{
    bool needs_update = false;
    if (!g_off && g_current_pattern) {
	needs_update = g_current_pattern->display();
    }

    if (g_force_update || needs_update) {
	show_pixels();
	g_force_update = false;
    }
}

void turn_on()
{
    Serial.println("hello");
    g_off = false;
}

void turn_off()
{
    Serial.println("okaybye");
    draw_pixels(COLOUR_BLACK);
    g_force_update = true;
    g_off = true;
}