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

struct Mode g_modes[] = {
    { &g_pattern_huey },
    { &g_pattern_counter },
};
const int MODE_COUNT = sizeof(g_modes) / sizeof(g_modes[0]);
Value g_current_mode(0, 0, MODE_COUNT - 1, true);

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
    analogReadResolution(ANALOG_READ_RESOLUTION);
    analogReadAveraging(ANALOG_READ_AVERAGING);

    pinMode(ENCODER_1_SW_PIN, INPUT_PULLUP);

    // Indicate power status.
    pinMode(POWER_LED_PIN, OUTPUT);
    digitalWrite(POWER_LED_PIN, HIGH);

    // Begin output.
    g_octo.begin();

    g_pattern_counter.setup();
    g_pattern_huey.setup();
    update_pattern();

    g_hue.set_velocity(256, 10000);
}

void idle() {
    delay(10);
}

void loop()
{
    ui_loop();
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
	    // On release
	    if (element.get_current_change() < 0) {
		if (element.get_previous_millis() > 5000) {
		    turn_off();
		} else {
		    ui_advance_mode();
		}
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
