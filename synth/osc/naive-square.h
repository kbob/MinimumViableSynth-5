#ifndef NAIVE_SQUARE_included
#define NAIVE_SQUARE_included

#include <cassert>

#include "synth/core/config.h"
#include "synth/core/modules.h"
#include "synth/core/sizes.h"

class NaiveSquare : public ModuleType<NaiveSquare> {

public:

    NaiveSquare()
    : m_inv_Fs{0},
      m_phase{0}
    {
        freq.name("freq");
        out.name("out");
        ports(freq, out);
    }

    Input<> freq;
    Output<> out;

    void render(size_t frame_count)
    {
        float inv_Fs = m_inv_Fs;
        assert(inv_Fs);
        for (size_t i = 0; i < frame_count; i++) {
            out[i] = m_phase < 0.5 ? +1 : -1;
            m_phase += inv_Fs * freq[i];
            if (m_phase >= 1)
                m_phase -= 1;
        }
    }

    void configure(const Config& cfg) override
    {
        m_inv_Fs = 1.0 / cfg.sample_rate();
    }

private:

    float m_inv_Fs;
    float m_phase;

};

#endif /* !NAIVE_SQUARE_included */
