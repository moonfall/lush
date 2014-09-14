#include "lush.h"

void Pattern_audio::ui_callback(Element_id id, Element const &element)
{
    switch (id) {
	case UI_KNOB2_ENCODER:
	    if (g_ui.m_knob2_button.get_current().m_value) {	
		adjust_fft_gain(element.get_current_change());
	    } else {
		adjust_gain(element.get_current_change());
		program_gain();
	    }
	default:
	    break;
    }
}

