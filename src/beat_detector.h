#ifndef _BEAT_DETECTOR_H
#define _BEAT_DETECTOR_H

// http://archive.gamedev.net/archive/reference/programming/features/beatdetection/index.html

#include <AudioStream.h>

class Beat_detector
    : public AudioStream
{
  public:
    Beat_detector()
	: AudioStream(1, m_input_queue),
	  m_energy(0),
	  m_count(0),
	  m_beat_detected(false)
    {
	memset(m_history, 0, sizeof(m_history));
    }

    bool beat() {
	__disable_irq();
	bool beat_detected = m_beat_detected;
	m_beat_detected = false;
	__enable_irq();
	return beat_detected;
    }

    virtual void update();

  private:
    static const unsigned ENERGY_SAMPLES = 1024;
    static const unsigned HISTORY_COUNT = 43;

    audio_block_t *m_input_queue[1];
    int32_t m_energy;
    unsigned m_count;
    int32_t m_history[HISTORY_COUNT];
    volatile bool m_beat_detected;
};

#endif // !_BEAT_DETECTOR_H
