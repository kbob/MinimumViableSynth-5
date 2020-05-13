#ifndef NAIVE_SQUARE_included
#define NAIVE_SQUARE_included

#include "synth/core/config.h"
#include "synth/core/modules.h"

class NaiveSquare : public ModuleType<NaiveSquare> {

public:

    NaiveSquare()
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
            out[i] = m_phase < 0.5 ? +1 : -1;
            m_phase += freq[i] * (1.0 / SAMPLE_RATE);
            if (m_phase >= 1)
                m_phase -= 1;
        }
    }

private:

    float m_phase;

};

#endif /* !NAIVE_SQUARE_included */
