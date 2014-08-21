#include "lush.h"

void Pattern_audio::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    adjust_gain(element.get_current_change());
	    program_gain();
	default:
	    break;
    }
}

