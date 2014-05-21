#include <stdint.h>

#include <Encoder.h>
#include <OctoWS2811.h>

#include "lush.h"

// Pin configuration
const int POWER_LED_PIN = 13;

// A3 == external, A4 == adafruit internal
const int AUDIO_INPUT_PIN = A4;
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

// Current state
Pattern *g_current_pattern = NULL;
Pattern_huey g_pattern_huey;
Pattern_counter g_pattern_counter;

Value g_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);

// Audio acquisition
IntervalTimer g_sampling_timer;
float g_samples[FFT_SIZE * 2];
float g_magnitudes[FFT_SIZE];
int g_sample_counter;

// Output
// 24 bytes == 6 words for each LED of each strip.
DMAMEM int g_display_memory[LEDS_PER_STRIP * 6];
int g_drawing_memory[LEDS_PER_STRIP * 6];
OctoWS2811 g_octo(LEDS_PER_STRIP, g_display_memory, g_drawing_memory,
		  WS2811_GRB | WS2811_800kHz);

// UI state
Encoder encoder(ENCODER_1_A_PIN, ENCODER_1_B_PIN);

void setup()
{
    Serial.begin(38400);

    // Set up ADC and audio input.
    pinMode(AUDIO_INPUT_PIN, INPUT);
    analogReadResolution(ANALOG_READ_RESOLUTION);
    analogReadAveraging(ANALOG_READ_AVERAGING);

    pinMode(ENCODER_1_SW_PIN, INPUT_PULLUP);

    // Indicate power status.
    pinMode(POWER_LED_PIN, OUTPUT);
    digitalWrite(POWER_LED_PIN, HIGH);

    // Begin output.
    g_octo.begin();

    g_current_pattern = &g_pattern_counter;
    g_pattern_counter.setup();
    g_pattern_huey.setup();

    g_hue.set_velocity(256, 10000);
}

void loop()
{
    ui_loop();
    display_loop();
}

void ui_loop()
{
}

void display_loop()
{
    bool needs_update = false;
    if (g_current_pattern) {
	needs_update = g_current_pattern->display();
    }

    if (needs_update) {
	show_pixels();
    }
}
