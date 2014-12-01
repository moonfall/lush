#include "displaylist.h"

void draw_objects(Object **objects, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
	objects[i]->draw();
    }
}

int interpolate(int s, int e, int t, int max_t)
{
    return s + (e - s) * t / (max_t - 1);
}

Coord interpolate(Coord const &s, Coord const &e, int t, int max_t)
{
    return Coord(interpolate(s.m_x, e.m_x, t, max_t),
		 interpolate(s.m_y, e.m_y, t, max_t));
}
