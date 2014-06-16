#ifndef _BLOCKS_H_INCLUDED
#define _BLOCKS_H_INCLUDED

#include "lush.h"

// Utility functions.
template<class T>
inline void swap(T &x, T &y)
{
    T tmp = x;
    x = y;
    y = tmp;
}

void shuffle_array(int *array, int count);
void reverse_array(int *array, int count);
void make_shuffled_array(int *array, int count);

// Object which just generates values in order and will loop around to
// the beginning when done.
class Generator
{
  public:
    virtual int current(bool *done = 0) = 0;
    virtual void next() = 0;
    void fill_array(int *array, int count);
};

// Combine two integers.
class Combination
{
  public:
    virtual int combine(int a, int b) = 0;
};

// Filter a single integer.
class Filter
{
  public:
    virtual int filter(int a) = 0;
};

// start, start + stride, start + 2 * stride, ..., start + (count - 1) * stride
// [DONE]
class Counter
    : public Generator
{
  public:
    Counter(int start, int count, int stride = 1)
	: m_start(start), m_count(count), m_stride(stride),
	  m_pos(0)
    {
    }

    virtual int current(bool *done)
    {
	if (done) {
	    *done = m_pos + 1 >= m_count;
	}
	return m_start + m_pos * m_stride;
    }

    virtual void next()
    {
	++m_pos;
	if (m_pos >= m_count) {
	    m_pos = 0;
	}
    }

    int m_start;
    int m_count;
    int m_stride;
    int m_pos;
};

// forward: 0, 1, ..., count - 1
// !forward: count - 1, count - 2, count - 3, ..., 0
class Simple_counter
    : public Counter	
{
  public:
    Simple_counter(int count, bool forward = true)
	: Counter(forward ? 0 : count - 1, count, forward ? 1 : - 1)
    {
    }
};

// N == last == (count - 1)
// c(a0,b0), c(a0,b1), c(a0,b2), ..., c(a0,bN)
// c(a1,b0), c(a1,b1), c(a1,b2), ..., c(a1,bN)
// ...
// c(aN,b0), c(aN,b1), c(aN,b2), ..., c(aN,bN) [DONE]
class For_each
    : public Generator
{
  public:
    For_each(Generator &a, Generator &b, Combination &c)
	: m_a(a), m_b(b), m_c(c)
    {
    }

    virtual int current(bool *done)
    {
	bool done_a;
	bool done_b;
	int value = m_c.combine(m_a.current(&done_a), m_b.current(&done_b));
	if (done) {
	    *done = done_a && done_b;
	}

	return value;
    }

    virtual void next()
    {
	// Advance to the next value.
	bool done_b;
	m_b.current(&done_b);
	m_b.next();
	if (done_b) {
	    m_a.next();
	}
    }

    Generator &m_a;
    Generator &m_b;
    Combination &m_c;
};


// g[0]0, g[0]1, g[0]2, ..., g[0]N,
// g[1]0, g[1]1, g[1]2, ..., g[1]N,
// ...
// g[count - 1]0, g[count - 1]1, g[count - 1]2, ..., g[count - 1]N [DONE]
class Cycler
    : public Generator
{
  public:
    Cycler(Generator **g, int count)
	: m_g(g), m_count(count), m_active(0)
    {
    }

    virtual int current(bool *done)
    {
	if (done) {
	    *done = false;
	}
	return get_current()->current(is_last() ? done : 0);
    }

    virtual void next()
    {
	get_current()->next();
	if (is_last()) {
	    m_active = 0;
	} else {
	    ++m_active;
	}
    }

    virtual Generator *get_current()
    {
	return m_g[m_active];
    }

    bool is_last()
    {
    	return m_active + 1 >= m_count;
    }

    Generator **m_g;
    int m_count;
    int m_active;
};

// a0, b0, a1, b1, a2, b2, ..., aN, bN [DONE]
class Alternator
    : public Cycler
{
  public:
    Alternator(Generator &a, Generator &b)
	: Cycler(m_array, 2)
    {
	m_array[0] = &a;
	m_array[1] = &b;
    }

    Generator *m_array[2];
};

// TODO: Change split to Switcher?
// split: a0, a1, a2, ..., aN [DONE], b0, b1, b2, ..., bN [DONE]
// joined: a0, a1, a2, ..., aN, b0, b1, b2, ..., bN [DONE]
class Concatenator
    : public Generator
{
  public:
    // split indicates that each generator group should register done
    // individually.
    Concatenator(Generator &a, Generator &b, bool split)
	: m_a(a), m_b(b), m_split(split), m_a_current(true)
    {
    }

    virtual int current(bool *done)
    {
	if (done) {
	    *done = false;
	}
	return m_a_current ? m_a.current(m_split ? done : 0) :
			     m_b.current(done);
    }

    virtual void next()
    {
	Generator &c = get_current();
	bool done;
	c.current(&done);
	c.next();
	if (done) {
	    m_a_current = !m_a_current;
	}
    }

    virtual Generator &get_current()
    {
	return m_a_current ? m_a : m_b;
    }
    Generator &m_a;
    Generator &m_b;
    bool m_split;
    bool m_a_current;
};

// f(a0), f(a1), f(a2), ..., f(aN) [DONE]
class Mapper
    : public Generator
{
  public:
    Mapper(Generator &a)
	: m_a(a), m_f(0)
    {
    }

    Mapper(Generator &a, Filter &f)
	: m_a(a), m_f(&f)
    {
    }

    virtual int current(bool *done)
    {
	int value = m_a.current(done);
	return m_f ? m_f->filter(value) : value;
    }

    virtual void next()
    {
	m_a.next();
    }

    Generator &m_a;
    Filter *m_f;
};

class Flip
    : public Filter
{
  public:
    Flip(int total)
	: m_total(total)
    {
    }

    virtual int filter(int a)
    {
	return m_total - a - 1;
    }
  
    int m_total;
};

class Make_led
    : public Combination
{
  public:
    Make_led(int columns = COLUMN_COUNT)
	: m_columns(columns)
    {
    }

    virtual int combine(int y, int x)
    {
	return y * m_columns + x;
    }

    int m_columns;
};

#if 0
// Can't be advanced
class Shadow
    : public Mapper
{
  public:
    Shadow(Generator &a)
	: Mapper(a)
    {
    }

    virtual void next()
    {
	// does not advance, just mirrors existing state.
    }
};
#endif

// Can't be advanced
class Mirror
    : public Mapper
{
  public:
    Mirror(Generator &a, int total)
	: Mapper(a, m_f), m_f(total)
    {
    }

    virtual void next()
    {
	// does not advance, just mirrors existing state.
    }

    Flip m_f;
};

class For_each_led
    : public For_each
{
  public:
    For_each_led(Generator &a, Generator &b)
	: For_each(a, b, m_c)
    {
    }

    Make_led m_c;
};

#endif // !_BLOCKS_H_INCLUDED
