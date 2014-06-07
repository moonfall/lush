#include <arm_math.h>
#include <stdint.h>

#include <ADC.h>
#include <Encoder.h>
#include <SPI.h>
#include <Mcp4261.h>
#include <OctoWS2811.h>

#include "lush.h"
#include "sqrt_integer.h"
#include "dspinst.h"

#undef SAMPLE_TEST
#undef PROFILE_FFT
#undef LOG_RAW_LEVELS
#undef LOG_SAMPLES
#undef LOG_FFT
#undef LOG_MAGNITUDES
#undef LOG_BINS
#undef LOG_SUMMARY
#define MAGNITUDE_AVERAGE

// Pin configuration
const int POWER_LED_PIN = 13;

// A4 == external, A3 == adafruit internal
const int AUDIO_INPUT_PIN = A4;
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
Pattern_huey g_pattern_huey;
Pattern_counter g_pattern_counter;
Pattern_spectrum_bars g_pattern_spectrum_bars;
Pattern_spectrum_field g_pattern_spectrum_field;
Pattern_spectrum_timeline g_pattern_spectrum_timeline;
Pattern_synthesia_fire g_pattern_synthesia_fire;
Pattern_synthesia_plasma_complex g_pattern_synthesia_plasma_complex;

// Modes:
// - weighted random mode
// - tour
// - select specific mode
// - configuration
struct Mode g_modes[] = {
    { &g_pattern_huey },
    { &g_pattern_counter },
    { &g_pattern_spectrum_bars },
    { &g_pattern_spectrum_field },
    { &g_pattern_spectrum_timeline },
    { &g_pattern_synthesia_fire },
    { &g_pattern_synthesia_plasma_complex },
};
const int MODE_COUNT = sizeof(g_modes) / sizeof(g_modes[0]);
Value g_current_mode(0, 0, MODE_COUNT - 1, true);

Value g_brightness(16, 0, 255);
Value g_resume_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);

// Audio gain control
MCP4261 g_mcp4261(MCP4261_CS_PIN, 100000);
Value g_gain0(128, 0, 255);
Value g_gain1(128, 0, 255);

// Audio acquisition
ADC *g_adc;
IntervalTimer g_sampling_timer;

int g_sample_rate_hz = 20000;
int g_sample_counter = 0;
int g_sample_generation = 0;

float g_level_avg = 0.0;
float g_level_min = 0.0;
float g_level_max = 0.0;
Sample_type g_samples[FFT_SIZE];

arm_cfft_radix4_instance g_fft_inst;
Sample_type g_fft_samples[FFT_SIZE * 2];
int g_fft_sample_generation = 0;
Sample_type *g_window = NULL;
Sample_type g_window_data[FFT_SIZE];
Sample_type g_magnitudes[FFT_SIZE];
int g_fft_generation = 0;
Sample_type g_bins[MAX_BIN_COUNT];
int g_bin_generation = 0;

const float GAIN_R1 = 10000;
const float GAIN_R2 = 100000;
const float GAIN_RAB = 100000;
const float GAIN_RS_COUNT = 256;
const float GAIN_RS_TYP = GAIN_RAB / GAIN_RS_COUNT;

Value g_min_db(55.0);
Value g_max_db(65.0);
Value g_bin_count(8);

// Filter out DC component by keeping a rolling average.
int g_dc_total = (1 << (ANALOG_READ_RESOLUTION - 1));
const int g_dc_sample_count = 32 * 1024;

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
    Serial.begin(38400);

    // Set up SPI to allow control of digital pot for gain control.
    SPI.begin();

    delayMicroseconds(50);
    set_gain();

    // Set up ADC and audio input.
    pinMode(AUDIO_INPUT_PIN, INPUT);
    g_adc = new ADC();
    g_adc->setReference(DEFAULT, 0);
    g_adc->setReference(DEFAULT, 1);
    g_adc->setResolution(ANALOG_READ_RESOLUTION, 0);
    g_adc->setResolution(ANALOG_READ_RESOLUTION, 1);
    g_adc->setAveraging(ANALOG_READ_AVERAGING, 0);
    g_adc->setAveraging(ANALOG_READ_AVERAGING, 1);

    g_dc_total = 0;
    for (int i = 0; i < g_dc_sample_count; ++i) {
	g_dc_total += g_adc->analogRead(AUDIO_INPUT_PIN);
    }

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

    g_pattern_counter.setup();
    g_pattern_huey.setup();
    g_pattern_spectrum_bars.setup();
    g_pattern_spectrum_field.setup();
    g_pattern_spectrum_timeline.setup();
    update_pattern();

    // Start cycling colours by default
    g_hue.set_velocity(256, 10000);

    fft_setup();

#ifndef SAMPLE_TEST
    // Start sampling sound.
    // TODO: Only do this if necessary
    sampler_start();
#endif
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

#ifdef MAGNITUDE_AVERAGE
static Sample_type g_magnitude_sums[FFT_SIZE];
const int g_magnitude_avg_count = 1000;
static int g_magnitude_avg_gathered = 0;
#endif

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
    for (int i = 0; i < FFT_SIZE; ++i) {
	g_magnitude_sums[i] = 0;
    }
#endif
}

void idle() {
    // TODO: Replace this with actual sleep.
    delay(10);
}

#ifdef LOG_RAW_LEVELS
int last_report = 0;
#endif
void loop()
{
    ui_loop();
#ifdef SAMPLE_TEST
    int min = g_adc->analogRead(AUDIO_INPUT_PIN);
    int max = min;
    for (int i = 0; i < 10000; ++i) {
	int value = g_adc->analogRead(AUDIO_INPUT_PIN);
	min = min(min, value);
	max = max(max, value);
    }
    Serial.print("m=");
    Serial.print(min);
    Serial.print(" M=");
    Serial.print(max);
    Serial.print(" D=");
    Serial.println(max - min);
#else
    sampler_loop();
#endif
#ifdef LOG_RAW_LEVELS
    if (millis() - last_report > 100) {
	Serial.print(g_sample_generation);
	Serial.print(": ");
	Serial.print(g_level_avg);
	Serial.print(" m=");
	Serial.print(g_level_min);
	Serial.print(" M=");
	Serial.print(g_level_max);
	Serial.print(" D=");
	Serial.println(g_level_max - g_level_min);
	last_report = millis();
    }
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
	    break;

	case UI_KNOB2_BUTTON:
	    // No action, just modify KNOB2 encoder
	    break;
    }
}

void ui_loop()
{
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

void sampler_callback()
{
    int sample = g_adc->analogRead(AUDIO_INPUT_PIN);

#define REMOVE_DC_BIAS_SAMPLER
#ifdef REMOVE_DC_BIAS_SAMPLER
    // TODO: Consider just ignoring this during FFT.
    int dc_mean = running_average(g_dc_total, g_dc_sample_count, sample);
    sample -= dc_mean;
#endif

    g_samples[g_sample_counter] = (Sample_type) sample;

    ++g_sample_counter;

    // Don't collect further samples until these have been processed
    if (sampler_done()) {
	g_sampling_timer.end();
    }
}

void sampler_start()
{
    g_sample_counter = 0;
    ++g_sample_generation;
    g_sampling_timer.begin(sampler_callback, 1000000 / g_sample_rate_hz);
}

bool sampler_done()
{
    return g_sample_counter >= FFT_SIZE;
}

void sampler_loop()
{
    if (sampler_done()) {
#ifdef PROFILE_FFT
	static int last = 0;
	int start_prepare = micros();
#endif
	fft_prepare();
#ifdef PROFILE_FFT
	int end_prepare = micros();
#endif

	// Start sampling all over again.
	sampler_start();

#ifdef REMOVE_DC_BIAS_POST_SAMPLER
	float sum = 0;
#endif
	float abs_sum = 0;
	float min = g_fft_samples[0];
	float max = g_fft_samples[0];
	for (int i = 0; i < FFT_SIZE * 2; i += 2) {
#ifdef REMOVE_DC_BIAS_POST_SAMPLER
	    sum += g_fft_samples[i];
#endif
	    abs_sum += abs(g_fft_samples[i]);
	    min = min(min, g_fft_samples[i]);
	    max = max(max, g_fft_samples[i]);
	}
	g_level_avg = abs_sum / FFT_SIZE;
	g_level_min = min;
	g_level_max = max;
#ifdef REMOVE_DC_BIAS_POST_SAMPLER
	float avg = sum / FFT_SIZE;
	for (int i = 0; i < FFT_SIZE * 2; i += 2) {
	    g_fft_samples[i] -= avg;
	}
#endif

#ifdef PROFILE_FFT
	int start_window = micros();
#endif
	fft_window();
#ifdef PROFILE_FFT
	int end_window = micros();
#endif
	fft_process();
#ifdef PROFILE_FFT
	int end_process = micros();
#endif
	fft_reduce();
	int end_reduce = micros();
#ifdef PROFILE_FFT
	Serial.print("sample ");
	Serial.print(start_prepare - last);
	Serial.print(" prepare ");
	Serial.print(end_prepare - start_prepare);
	Serial.print(" window ");
	Serial.print(end_window - start_window);
	Serial.print(" fft ");
	Serial.print(end_process - end_window);
	Serial.print(" reduce ");
	Serial.print(end_reduce - end_process);
	Serial.println();
	last = end_process;
#endif


#ifdef LOG_SUMMARY

#ifdef MAGNITUDE_AVERAGE
	if (g_magnitude_avg_gathered < g_magnitude_avg_count) {
	    for (int i = 0; i < FFT_SIZE; ++i) {
		g_magnitude_sums[i] += g_magnitudes[i];
	    }
	} else {
	    for (int i = 0; i < FFT_SIZE; ++i) {
		running_average(g_magnitude_sums[i], g_magnitude_avg_count,
				g_magnitudes[i]);
	    }
	}
	++g_magnitude_avg_gathered;
#endif

	static int last_log = 0;
	if (millis() > last_log + 100) {
	    Serial.print(g_sample_generation);
	    Serial.print(": ");
#ifdef MAGNITUDE_AVERAGE
	    Serial.print(g_magnitude_avg_gathered);
	    Serial.print(": ");
#endif
	    Serial.print("gain=");
	    Serial.print(g_gain0.get());
	    Serial.print("/");
	    Serial.print(g_gain1.get());
	    Serial.print(" dc=");
	    Serial.print(g_dc_total / g_dc_sample_count);
	    Serial.print(" avg=");
	    Serial.print(g_level_avg, 0);
	    Serial.print(" D=");
	    Serial.print(g_level_max - g_level_min, 0);
	    Serial.print(" mags");
	    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
		Serial.print(" ");
#ifdef MAGNITUDE_AVERAGE
		Serial.print(g_magnitude_sums[i] / g_magnitude_avg_count, 0);
#else
		Serial.print(g_magnitudes[i], 0);
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

void fft_setup()
{
    arm_cfft_radix4_init(&g_fft_inst, FFT_SIZE, 0, 1);
    fft_window_setup();
}

void fft_window_setup()
{
#define WINDOW_HAMMING

#ifdef WINDOW_HANNING
    // Hanning window
    float *window = g_window_data;
    for (int i = 0; i < FFT_SIZE; ++i, ++window) {
	*window = 0.5 * (1 - cos(2.0 * M_PI * i / (FFT_SIZE - 1)));
    }
    g_window = g_window_data;
#endif
#ifdef WINDOW_HAMMING
    // Hamming window
    float *window = g_window_data;
    for (int i = 0; i < FFT_SIZE; ++i, ++window) {
	*window = 0.54 - 0.46 * cos(2.0 * M_PI * i / (FFT_SIZE - 1));
    }
    g_window = g_window_data;
#endif
}

void fft_prepare()
{
    const Sample_type *src = g_samples;
#ifdef Q15
    uint32_t *dst = (uint32_t *)g_fft_samples;
    for (int i = 0; i < FFT_SIZE; ++i) {
	*dst = *src;  // real sample plus a zero for imaginary
	++dst;
	++src;
    }
#else
    Sample_type *dst = g_fft_samples;
    for (int i = 0; i < FFT_SIZE; ++i) {
	*dst = *src;  // real sample
	++dst;
	++src;
	*dst = 0; // imaginary
	++dst;
    }
#endif
    g_fft_sample_generation = g_sample_generation;
}

void fft_window()
{
    if (!g_window) {
	return;
    }

#ifdef F32
    float *sample = g_fft_samples;
    float *window = g_window;
    for (int i = 0; i < FFT_SIZE; ++i, ++sample, ++window) {
	*sample *= *window;
    }
#endif;
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

void fft_process()
{
#ifdef LOG_SAMPLES
    // Original samples.
    Serial.print("s ");
    for (int i = 0; i < FFT_SIZE; ++i) {
	Serial.print(g_samples[i]);
	Serial.print(" ");
    }
    Serial.println();
#endif

    // Perform FFT
    arm_cfft_radix4(&g_fft_inst, g_fft_samples);

#ifdef LOG_FFT
    // Raw FFT transform including complex.
    Serial.print("f ");
    for (int i = 0; i < FFT_SIZE; ++i) {
	Serial.print(g_fft_samples[i]);
	Serial.print(" ");
    }
    Serial.println();
#endif

    // Calculate magnitudes
#if defined(F32)
    arm_cmplx_mag_f32(g_fft_samples, g_magnitudes, FFT_SIZE);
#else
    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
#if defined(Q15)
	uint32_t tmp = *((uint32_t *)g_fft_samples + i);
	uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
	g_magnitudes[i] = sqrt_uint32_approx(magsq);
#else
	g_magnitudes[i] = sqrt(g_fft_samples[2 * i] * g_fft_samples[2 * i] +
			       g_fft_samples[2 * i + 1] * g_fft_samples[2 * i + 1]);
#endif
    }
#endif

#ifdef LOG_MAGNITUDES
    Serial.print("dc=");
    Serial.print(g_dc_total / g_dc_sample_count);
    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
	Serial.print(" ");
	Serial.print(g_magnitudes[i]);
    }
    Serial.println();
#endif

#ifdef APPLY_FLOOR
    float gain = calculate_actual_gain(g_gain0.get()) *
		 calculate_actual_gain(g_gain1.get());
#ifdef LOG_FLOORS
    Serial.print("gain=");
    Serial.print(gain);
    Serial.print("/");
    Serial.print(calculate_actual_gain(g_gain0.get()));
    Serial.print("/");
    Serial.print(calculate_actual_gain(g_gain1.get()));
#endif
    for (int i = 0; i < MAGNITUDE_COUNT; ++i) {
	float floor = calculate_floor(gain, i);
#ifdef LOG_FLOORS
	Serial.print(" ");
	Serial.print(g_magnitudes[i]);
	Serial.print("-");
	Serial.print(floor);
#endif
	if (g_magnitudes[i] < floor) {
	    g_magnitudes[i] = 0.0;
	} else {
	    g_magnitudes[i] -= floor;
	}
    }
#ifdef LOG_FLOORS
    Serial.println();
#endif
#endif
    
    g_fft_generation = g_fft_sample_generation;
}

void fft_reduce()
{
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
	    float p = g_magnitudes[f_start + j];
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
#else
	bin_power = 20.0 * log10(bin_power);
	bin_power -= (Sample_type) g_min_db.get();
	bin_power = bin_power < 0.0 ? 0.0 : bin_power;
	bin_power /= (Sample_type) (g_max_db.get() - g_min_db.get());
	bin_power = bin_power > 1.0 ? 1.0 : bin_power;
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

    g_bin_generation = g_fft_generation;
}

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
