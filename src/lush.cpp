#undef DISABLE_DISPLAY
#undef DISABLE_AUDIO
#define DISABLE_BORING

#ifndef DISABLE_AUDIO
#include <Audio.h>
#endif

#include <arm_math.h>
#include <stdint.h>

#include <Encoder.h>
#include <SPI.h>
#include <OctoWS2811.h>

#include "lush.h"
#include "dspinst.h"
#include "patterns.h"

#undef SAMPLE_TEST
#undef PROFILE_FFT
#undef LOG_MAGNITUDES
#undef LOG_REDUCE
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
const int ANALOG_READ_AVERAGING = 16;

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
Value g_brightness(3, 0, 255);
Value g_resume_brightness(16, 0, 255);
Value g_hue(0, 0, 255, true);
Value g_number(0, 0, 1000, true);
Value g_up(DIR_DOWN, 0, 3, true);

// Audio gain control
Value g_gain0(INITIAL_GAIN, 0, 255);
Value g_gain1(INITIAL_GAIN, 0, 255);
Value g_min_power(100, 0, 1000);
Value g_max_power(300, 0, 1000);

Pattern *g_root = NULL;

// Audio acquisition
#ifndef DISABLE_AUDIO
AudioInputAnalog g_audio_input(AUDIO_INPUT_PIN);
// http://forum.pjrc.com/threads/24793-Audio-Library?p=40179&viewfull=1#post40179
// http://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/

AudioFilterBiquad g_hp_filter;
#ifdef FFT1024
AudioAnalyzeFFT1024 g_fft;
#else
AudioAnalyzeFFT256 g_fft;
#endif
AudioAnalyzePeak g_peak;
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

unsigned g_current_peak = 0;

#ifndef DISABLE_AUDIO
Sample_type g_magnitudes[MAGNITUDE_COUNT];
unsigned g_bin_count = 0;
unsigned g_fft_scale_factor = 0;
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

// Output
// 24 bytes == 6 words for each LED of each strip.
DMAMEM int g_display_memory[LEDS_PER_STRIP * 6];
DMAMEM int g_drawing_memory[LEDS_PER_STRIP * 6];
OctoWS2811 g_octo(LEDS_PER_STRIP, g_display_memory, g_drawing_memory,
		  WS2811_GRB | WS2811_800kHz);
unsigned g_target_fps = 30;
uint32_t g_last_update = 0;

// UI state
Encoder encoder1(ENCODER_1_A_PIN, ENCODER_1_B_PIN);
Encoder encoder2(ENCODER_2_A_PIN, ENCODER_2_B_PIN);
UI_state g_ui;

Pattern *setup_patterns();

void flash_led()
{
    // Indicate power status by fading out power LED.
    pinMode(POWER_LED_PIN, OUTPUT);
    const int DUTY_CYCLE = 1000;
    const int DUTY_CHANGE = 3;
    for (int on_cycle = DUTY_CYCLE; on_cycle > 0; on_cycle -= DUTY_CHANGE) {
	digitalWrite(POWER_LED_PIN, HIGH);
	delayMicroseconds(on_cycle);
	digitalWrite(POWER_LED_PIN, LOW);
	delayMicroseconds(DUTY_CYCLE - on_cycle);
    }
    digitalWrite(POWER_LED_PIN, LOW);
}

void setup()
{
    flash_led();

    Serial.begin(115200);

    randomSeed(analogRead(A13));

    // Set up SPI to allow control of digital pot for gain control.
    pinMode(MCP4261_CS_PIN, OUTPUT);
    SPI.begin();
    delayMicroseconds(50);

    program_gain();

#ifndef DISABLE_AUDIO
    set_fft_bin_count(0);
    set_fft_scale_factor(0);

    // Set up ADC and audio input.
    g_hp_filter.setHighpass(0, 250, 0.707);
    // driley-20140923: Max used 4
    AudioMemory(6);
#endif

#if 0
    for (int i = 0; i < 1000; ++i) {
	Serial.println(i);
	delay(1);
    }
    flash_led();
#endif
    pinMode(ENCODER_1_SW_PIN, INPUT_PULLUP);
    pinMode(ENCODER_2_SW_PIN, INPUT_PULLUP);

    // Begin output.
    g_octo.begin();

    // Start cycling colours by default
    g_hue.set_velocity(256, 10000);

    // Cache the up value since it's always used
    set_up_direction(g_up.get());
    g_up.set_callback(set_up_direction);

    g_root = setup_patterns();	
    g_root->activate(NULL);

    flash_led();
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
    int target0 = g_gain0.get();
    int target1 = g_gain1.get();
    int gain0 = set_wiper(0, target0);
    int gain1 = set_wiper(1, target1);
    Serial.print("gain set to ");
    Serial.print(target0);
    Serial.print("/");
    Serial.print(target1);
    Serial.print("->");
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

void adjust_fft_max_power(int adjustment)
{
    g_max_power.modify(adjustment);
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
    float peak = g_peak.read();
    g_current_peak = max(g_current_peak, peak * 65535);

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
#endif
#if 1

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
}
#endif
#ifndef DISABLE_AUDIO

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

    // TODO: Ensure LOG_SCALE is set appropriately to allow proper scaling
    // even if fft_scale_factor is large.
    const float LOG_SCALE = 1000.0;
    int min_power = g_min_power.get() * 10;
    int max_power = g_max_power.get() * 10;
    int power_range = max_power - min_power;
    for (unsigned i = 0; i < g_bin_count; ++i) {
	int bin_magnitude = 0;
	int bin_width = 0;
	for (bin_width = 0;
	     bin_width < g_bin_widths[i] &&
	     f_start + bin_width < MAGNITUDE_COUNT; ++bin_width) {
	    int p = g_magnitudes[f_start + bin_width];
#define MAX_POWER
#ifdef MAX_POWER
	    if (p > bin_magnitude) {
		bin_magnitude = p;
	    }
#else
	    // Average.
	    bin_magnitude += p;
#endif
	}

#ifndef MAX_POWER
	// Average
	bin_magnitude /= bin_width;
#endif

	int bin_power = (int) (LOG_SCALE * log10((float) bin_magnitude));
	int scaled_power = bin_power;
	if (scaled_power < min_power) {
	    scaled_power = 0;
	} else if (scaled_power > max_power) {
	    scaled_power = g_fft_scale_factor;
	} else {
	    scaled_power = (scaled_power - min_power) * g_fft_scale_factor /
		        power_range;
	}

#ifdef LOG_REDUCE
	Serial.printf("%5u/%3u ", bin_power, scaled_power);
#endif

#ifdef USE_SMOOTHING
	g_bins[i] = (int)((float) g_bins[i] * smoothing +
		          (scaled_power * (1.0 - smoothing)));
#else
	g_bins[i] = (int) scaled_power;
#endif

	f_start += bin_width;
    }

#ifdef LOG_REDUCE
    Serial.println();
#endif

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
#if 1
    const unsigned MAX_VALUE = 65535;
    const unsigned NOISE_FLOOR = 19000;

    if (g_current_peak < NOISE_FLOOR) {
	return 0;
    }

    unsigned peak = g_current_peak - NOISE_FLOOR;

    return min(max_value,
	       peak * (max_value + 1) /
	       (unsigned) (MAX_VALUE - NOISE_FLOOR));
#else
    const float LOG_SCALE = 350.0;
    int power = (int) (LOG_SCALE * log10((float) g_current_peak));
#if 0
    int min_power = g_min_power.get() * 10;
    int max_power = g_max_power.get() * 10;
#endif
    int min_power = 1000;
    int max_power = 3000;
    if (power < min_power) {
	return 0;
    } else if (power > max_power) {
	return max_value;
    } else {
	return (power - min_power) * max_value / (max_power - min_power);
    }
#endif
}

void display_loop()
{
#ifdef DISABLE_DISPLAY
    return;
#endif
    if (should_display() ) {
	if (!g_root->is_full_screen()) {
	    draw_pixels(COLOUR_BLACK);
	} else {
	    Serial.print("skip clear\n");
	}

	if (g_root->display()) {
	    show_pixels();
	    g_last_update = millis();

	    // TODO: Is this the best place?
	    reset_peak();

#if 0
	    Serial.printf("mem %u\n", AudioMemoryUsageMax());
	    Serial.print("Free ram:");Serial.println(FreeRam());
#endif
	}
    }
}

