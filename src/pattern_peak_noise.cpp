#include <Audio.h>
#include "lush.h"

Pattern_peak_noise::Pattern_peak_noise(Fader_static &fader)
    : Pattern_peak_ordered(fader)
{
}

void Pattern_peak_noise::activate(void *arg)
{
    m_fader.set_shuffled();
}

bool Pattern_peak_noise::display()
{
    bool needs_update = Pattern_peak_ordered::display();
    if (needs_update) {
	m_fader.set_shuffled();
    }
    return needs_update;
}
