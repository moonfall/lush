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

// Pin configuration
const int POWER_LED_PIN = 13;

// A4 == external, A3 == adafruit internal
const int AUDIO_INPUT_PIN = A4;
// Bits of resolution for ADC
const int ANALOG_READ_RESOLUTION = 12;
// Number of samples to average with each ADC reading.
const int ANALOG_READ_AVERAGING = 16;

const int MCP4261_CS_PIN = 10;

const int ENCODER_1_A_PIN = 0;
const int ENCODER_1_B_PIN = 1;
const int ENCODER_1_SW_PIN = 9;

// Constants
const int ROW_COUNT = 8;
const int COLUMN_COUNT = 8;
const int LED_COUNT = ROW_COUNT * COLUMN_COUNT;
const int LEDS_PER_STRIP = LED_COUNT;
const int TURN_OFF_MS = 3000;

// Current state
Pattern *g_current_pattern = NULL;
Pattern_huey g_pattern_huey;
Pattern_counter g_pattern_counter;
Pattern_spectrum g_pattern_spectrum;

// Modes:
// - weighted random mode
// - tour
// - select specific mode
// - configuration
struct Mode g_modes[] = {
    { &g_pattern_huey },
    { &g_pattern_counter },
    { &g_pattern_spectrum },
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
Sample_type g_magnitudes[FFT_SIZE];
int g_fft_generation = 0;
Sample_type g_bins[BIN_COUNT];
int g_bin_generation = 0;

// Filter out DC component by keeping a rolling average.
int g_dc_total = (1 << (ANALOG_READ_RESOLUTION - 1));
const int g_dc_sample_count = 1024;

// Output
// 24 bytes == 6 words for each LED of each strip.
DMAMEM int g_display_memory[LEDS_PER_STRIP * 6];
int g_drawing_memory[LEDS_PER_STRIP * 6];
OctoWS2811 g_octo(LEDS_PER_STRIP, g_display_memory, g_drawing_memory,
		  WS2811_GRB | WS2811_800kHz);
bool g_force_update = false;

// UI state
Encoder encoder1(ENCODER_1_A_PIN, ENCODER_1_B_PIN);
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

void set_gain()
{
    int gain0 = set_wiper(0, g_gain0.get());
    int gain1 = set_wiper(1, g_gain1.get());
    Serial.print("gain set to ");
    Serial.print(gain0);
    Serial.print("/");
    Serial.println(gain1);
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
#if 0
	    g_brightness.modify(element.get_current_change());
#else
	    g_gain0.modify(element.get_current_change());
	    g_gain1.modify(element.get_current_change());
	    set_gain();
#endif
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
    }
}

void ui_loop()
{
    {
	Element &element = g_ui.m_knob1_encoder;
	Element_state state(encoder1.read());
	if (element.get_change(state, element.get_current())) {
	    element.push(state);

	    Serial.print("new value is ");
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

	    Serial.print(state.m_value ? "pressed" : "released");
	    Serial.print(" after ");
	    Serial.println(element.get_previous_millis());

	    ui_callback(UI_KNOB1_BUTTON, element);
	}
    }
}

void sampler_callback()
{
    int sample = g_adc->analogRead(AUDIO_INPUT_PIN);
    int dc_mean = g_dc_total / g_dc_sample_count;
    g_dc_total = g_dc_total + sample - dc_mean;
    sample -= dc_mean;
    g_samples[g_sample_counter] = sample;

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
	float sum = 0;
	float min = g_samples[0];
	float max = g_samples[0];
	for (int i = 0; i < FFT_SIZE; ++i) {
	    sum += abs(g_samples[i]);
	    min = min(min, g_samples[i]);
	    max = max(max, g_samples[i]);
	}
	g_level_avg = sum / FFT_SIZE;
	g_level_min = min;
	g_level_max = max;

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

#ifdef PROFILE_FFT
	int start_process = micros();
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
	Serial.print(" fft ");
	Serial.print(end_process - start_process);
	Serial.print(" reduce ");
	Serial.print(end_reduce - end_process);
	Serial.println();
	last = end_process;
#endif
    }
}

void fft_setup()
{
    arm_cfft_radix4_init(&g_fft_inst, FFT_SIZE, 0, 1);
}

void fft_prepare()
{
    const Sample_type *src = g_samples;
#ifdef Q15
    uint32_t *dst = (uint32_t *)g_fft_samples;
    for (int i = 0; i < FFT_SIZE; i++) {
	*dst++ = *src++;  // real sample plus a zero for imaginary
    }
#else
    Sample_type *dst = g_fft_samples;
    for (int i = 0; i < FFT_SIZE; i++) {
	*dst++ = *src++;  // real sample
	*dst++ = 0; // imaginary
    }
#endif
    g_fft_sample_generation = g_sample_generation;
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

    g_fft_generation = g_fft_sample_generation;
}

void fft_reduce()
{
    int bin_size = MAGNITUDE_COUNT / BIN_COUNT;
    bin_size = 1;
    Sample_type *src = g_magnitudes;
    Sample_type *src_end = g_magnitudes + MAGNITUDE_COUNT;
    for (int i = 0; i < BIN_COUNT && src < src_end; ++i) {
	g_bins[i] = 0;
	for (int j = 0; j < bin_size && src < src_end; ++j, ++src) {
	    g_bins[i] += *src;
	}
	g_bins[i] /= bin_size;
	bin_size *= 2;
    }

#ifdef LOG_BINS
    Serial.print("dc=");
    Serial.print(g_dc_total / g_dc_sample_count);
    for (int i = 0; i < BIN_COUNT; ++i) {
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
