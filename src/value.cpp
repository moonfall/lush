#include <limits.h>
#include <math.h>

#include <OctoWS2811.h>

#include "lush.h"

Value::Value(int initial)
    : m_min(INT_MIN), m_max(INT_MAX), m_wraps(true),
      m_value(initial), m_value_time(0),
      m_velocity(0), m_velocity_ms(0),
      m_periodic(0), m_periodic_ms(0),
      m_callback(NULL)
{
    set(initial);
    set_velocity(0, 0);
    set_periodic(0, 0);
}

Value::Value(int initial, int min, int max, bool wraps)
    : m_min(min), m_max(max), m_wraps(wraps),
      m_value(initial), m_value_time(0),
      m_velocity(0), m_velocity_ms(0),
      m_periodic(0), m_periodic_ms(0),
      m_callback(NULL)
{
    set(initial);
    set_velocity(0, 0);
    set_periodic(0, 0);
}

void Value::set_min(int min)
{
    m_min = min;
}

void Value::set_max(int max)
{
    m_max = max;
}

void Value::set_wrap(bool wraps)
{
    m_wraps = wraps;
}

int Value::get_min()
{
    return m_min;
}

int Value::get_max()
{
    return m_max;
}

int Value::get_range()
{
    return m_max - m_min + 1;
}

bool Value::get_wrap()
{
    return m_wraps;
}

void Value::set(int value)
{
    m_value = bound(value);
    m_value_time = now();

    if (m_callback) {
	m_callback(value);
    }
}

void Value::modify(int delta)
{
    set(get() + delta);
}

void Value::set_velocity(int delta, int delta_ms)
{
    // TODO: This will cause discontinuities due to periodic.
    set(get());
    m_velocity = delta;
    m_velocity_ms = delta_ms;
}

void Value::set_periodic(int amplitude, int cycle_ms)
{
    // TODO: This will cause discontinuities due to velocity.
    set(get());
    m_periodic = amplitude;
    m_periodic_ms = cycle_ms;
}

void Value::set_callback(void (*callback)(int value))
{
    m_callback = callback;
}

int Value::get_unbounded()
{
    int value = m_value;
    int elapsed = 0;
    if (m_velocity && m_velocity_ms) {
	elapsed = now() - m_value_time;
	value += m_velocity * elapsed / m_velocity_ms;
    } 
    if (m_periodic && m_periodic_ms) {
	if (!elapsed) {
	    elapsed = now() - m_value_time;
	}
	value += m_periodic * sin(2 * M_PI * (double) elapsed / (double) m_periodic_ms);
    }

    return value;
}

int Value::get()
{
    return bound(get_unbounded());
}

int Value::now()
{
    return millis();
}

int Value::bound(int value)
{
    // Ensure that the minimum and maximum are valid.
    if (m_min > m_max) {
	return value;
    }

    if (m_min != INT_MIN && value < m_min) {
	if (m_wraps) {
	    int range = get_range();
	    // TODO: This will take longer and longer without resetting value.
	    while (value < m_min) {
		value += range;
	    }
	} else {
	    value = m_min;
	}
    } else if (m_max != INT_MAX && value > m_max) {
	if (m_wraps) {
	    int range = get_range();
	    // TODO: This will take longer and longer without resetting value.
#if 0
	    while (value > m_max) {
		value -= range;
	    }
#else
	    value = (value - m_min) % range + m_min;
#endif
	} else {
	    value = m_max;
	}
    }

    return value;
}
