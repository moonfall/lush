#include "beat_detector.h"

#include "analyze_peak.h"

void Beat_detector::update()
{
    audio_block_t *block = receiveReadOnly();
    if (!block) {
	return;
    }
    
    const int16_t *p = block->data;
    const int16_t *end = p + AUDIO_BLOCK_SAMPLES;
    do {
	m_energy += *p * *p;
	if (m_count >= ENERGY_SAMPLES) {
	    memmove(m_history + 1, m_history,
		    sizeof(m_history) - sizeof(m_history[0]));
	    m_history[0] = m_energy;
	    m_energy = 0;
	    m_count = 0;
	}
	++p;
    } while (p < end);

    release(block);
}

