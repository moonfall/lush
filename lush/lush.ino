#include <stdint.h>

#include <ADC.h>
#include <Encoder.h>
#include <OctoWS2811.h>

#include "lush.h"

#undef SAMPLE_TEST

// Pin configuration
const int POWER_LED_PIN = 13;

// A4 == external, A3 == adafruit internal
const int AUDIO_INPUT_PIN = A3;
// Bits of resolution for ADC
const int ANALOG_READ_RESOLUTION = 10;
// Number of samples to average with each ADC reading.
const int ANALOG_READ_AVERAGING = 16;

const int ENCODER_1_A_PIN = 0;
const int ENCODER_1_B_PIN = 1;
const int ENCODER_1_SW_PIN = 9;

// Constants
const int ROW_COUNT = 8;
const int COLUMN_COUNT = 8;
const int LED_COUNT = ROW_COUNT * COLUMN_COUNT;
const int LEDS_PER_STRIP = LED_COUNT;
const int FFT_SIZE = 256;
const int TURN_OFF_MS = 3000;

// Current state
Pattern *g_current_pattern = NULL;
Pattern_huey g_pattern_huey;
Pattern_counter g_pattern_counter;

struct Mode g_modes[] = {
    { &g_pattern_huey },
    { &g_pattern_counter },
};
const int MODE_COUNT = sizeof(g_modes) / sizeof(g_modes[0]);
Value g_current_mode(0, 0, MODE_COUNT - 1, true);

Value g_brightness(16, 0, 255);
Value g_resume_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);

// Audio acquisition
ADC *g_adc;
IntervalTimer g_sampling_timer;
float g_samples[FFT_SIZE * 2];
float g_magnitudes[FFT_SIZE];
int g_sample_counter = 0;
int g_sample_iterations = 0;
int g_sample_rate_hz = 20000;
float g_level_avg = 0.0;
float g_level_min = 0.0;
float g_level_max = 0.0;

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

    // Set up ADC and audio input.
    pinMode(AUDIO_INPUT_PIN, INPUT);
    g_adc = new ADC();
    g_adc->setReference(DEFAULT);
    g_adc->setResolution(ANALOG_READ_RESOLUTION);
    g_adc->setAveraging(ANALOG_READ_AVERAGING);

    pinMode(ENCODER_1_SW_PIN, INPUT_PULLUP);

    // Indicate power status.
    pinMode(POWER_LED_PIN, OUTPUT);
    digitalWrite(POWER_LED_PIN, HIGH);

    // Begin output.
    g_octo.begin();

    g_pattern_counter.setup();
    g_pattern_huey.setup();
    update_pattern();

    // Start cycling colours by default
    g_hue.set_velocity(256, 10000);

#ifndef SAMPLE_TEST
    // Start sampling sound.
    // TODO: Only do this if necessary
    sampler_start();
#endif
}

void idle() {
    // TODO: Replace this with actual sleep.
    delay(10);
}

#if 1
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
#if 0
    if (millis() - last_report > 100) {
	Serial.print(g_sample_iterations);
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
	    g_brightness.modify(element.get_current_change());
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

int extra_count = 0;
int extra_min = 9999;
int extra_max = 0;
void sampler_callback()
{
    int sample = g_adc->analogRead(AUDIO_INPUT_PIN);
    g_samples[g_sample_counter] = sample;

    // Set imaginary part of the input to 0 for FFT.
    g_samples[g_sample_counter + 1] = 0.0;

#if 0
    ++extra_count;
    if (sample < extra_min) {
	extra_min = sample;
    } else if (sample > extra_max) {
	extra_max = sample;
    }
    if (extra_count > 10000) {
	Serial.print("extra m=");
	Serial.print(extra_min);
	Serial.print(" M=");
	Serial.print(extra_max);
	Serial.print(" D=");
	Serial.println(extra_max - extra_min);
	extra_min = 9999;
	extra_max = 0;
	extra_count = 0;
    }
#endif

    g_sample_counter += 2;

    // Don't collect further samples until these have been processed
    if (sampler_done()) {
	g_sampling_timer.end();
    }
}

void sampler_start()
{
    g_sample_counter = 0;
    ++g_sample_iterations;
    g_sampling_timer.begin(sampler_callback, 1000000 / g_sample_rate_hz);
}

bool sampler_done()
{
    return g_sample_counter >= FFT_SIZE * 2;
}

int count = 0;
void sampler_loop()
{
    if (sampler_done()) {
	float sum = 0;
	float min = g_samples[0];
	float max = g_samples[0];
	for (int i = 0; i < FFT_SIZE * 2; i += 2) {
	    if (g_samples[i] > 512) { 
		sum += g_samples[i] - 512;
	    } else {
		sum += 512 - g_samples[i];
	    }
	    min = min(min, g_samples[i]);
	    max = max(max, g_samples[i]);
	}
	g_level_avg = sum / FFT_SIZE;
	g_level_min = min;
	g_level_max = max;

#if 0
	if (count % 100 == 99) {
	    for( int i = 0; i < FFT_SIZE * 2; i += 2) {
		Serial.print(g_samples[i]);
		Serial.print(" ");
	    }
	    Serial.println();
	}
	++count;
#endif

	// Start sampling all over again.
	sampler_start();
    }
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
