#ifndef NAIVE_SAW_included
#define NAIVE_SAW_included

#include "synth/core/config.h"
#include "synth/core/modules.h"

class NaiveSaw : public ModuleType<NaiveSaw> {

public:

    NaiveSaw()
    : m_phase{0}
    {
        freq.name("freq");
        out.name("out");
        ports(freq, out);
    }

    Input<> freq;
    Output<> out;

    void render(size_t frame_count)
    {
        for (size_t i = 0; i < frame_count; i++) {
            out[i] = 1 - 2 * m_phase;
            m_phase += freq[i] * (1.0 / SAMPLE_RATE);
            if (m_phase >= 1)
                m_phase -= 1;
        }
    }

private:

    float m_phase;

};

#endif /* !NAIVE_SAW_included */
