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

class Pixel
    : public Object
{
  public:
    Pixel()
	: m_x(0), m_y(0), m_c(COLOUR_BLACK)
    {
    }

    Pixel(int x, int y, Colour c)
	: m_x(x), m_y(y), m_c(c)
    {
    }

    void draw() {
	draw_pixel(m_x, m_y, m_c);
    }

    int m_x;
    int m_y;
    Colour m_c;
};

class Line
    : public Object
{
  public:
    Line(int x0, int y0, int x1, int y1, Colour c)
	: m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1), m_c(c)
    {
    }

    void draw() {
	draw_line(m_x0, m_y0, m_x1, m_y1, m_c);
    }

    int m_x0;
    int m_y0;
    int m_x1;
    int m_y1;
    Colour m_c;
};

void draw_objects(Object **objects, unsigned count);

#endif // !_DISPLAYLIST_H_INCLUDED
