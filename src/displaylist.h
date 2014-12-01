#ifndef _DISPLAYLIST_H_INCLUDED
#define _DISPLAYLIST_H_INCLUDED

#include "lush.h"

struct Object
{
  public:
    virtual ~Object()
    {
    }

    virtual void draw() = 0;
};

struct Coord
{
    Coord()
	: m_x(0), m_y(0)
    {
    }

    Coord(int x, int y)
	: m_x(x), m_y(y)
    {
    }

    bool operator==(Coord const &rhs) const
    {
	return m_x == rhs.m_x && m_y == rhs.m_y;
    }

    bool is_flipped(Coord const &rhs) const
    {
	return m_x == flip_x(rhs.m_x) && m_y == flip_y(rhs.m_y);
    }

    int m_x;
    int m_y;
};

class Pixel
    : public Object
{
  public:
    Pixel()
	: m_p(0, 0), m_c(COLOUR_BLACK)
    {
    }

    Pixel(int x, int y, Colour c)
	: m_p(x, y), m_c(c)
    {
    }

    Pixel(Coord const &pos, Colour c)
	: m_p(pos), m_c(c)
    {
    }

    void draw()
    {
	draw_pixel(m_p.m_x, m_p.m_y, m_c);
    }

    Coord m_p;
    Colour m_c;
};

class Line
    : public Object
{
  public:
    Line(Coord const &p0, Coord const &p1, Colour c)
	: m_p0(p0), m_p1(p1), m_c(c)
    {
    }
    Line(int x0, int y0, int x1, int y1, Colour c)
	: m_p0(x0, y0), m_p1(x1, y1), m_c(c)
    {
    }

    void draw()
    {
	draw_line(m_p0.m_x, m_p0.m_y, m_p1.m_x, m_p1.m_y, m_c);
    }

    Coord m_p0;
    Coord m_p1;
    Colour m_c;
};

void draw_objects(Object **objects, unsigned count);

int interpolate(int s, int e, int t, int max_t);
Coord interpolate(Coord const &s, Coord const &e, int t, int max_t);
#endif // !_DISPLAYLIST_H_INCLUDED
