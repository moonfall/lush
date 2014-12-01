#include "displaylist.h"

void draw_objects(Object **objects, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
	objects[i]->draw();
    }
}
