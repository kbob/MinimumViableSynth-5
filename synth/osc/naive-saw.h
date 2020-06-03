#ifndef NAIVE_SAW_included
#define NAIVE_SAW_included

#include <cassert>

#include "synth/core/audio-config.h"
#include "synth/core/modules.h"
#include "synth/core/sizes.h"

class NaiveSaw : public ModuleType<NaiveSaw> {

public:

    NaiveSaw()
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
            out[i] = 1 - 2 * m_phase;
            m_phase += inv_Fs * freq[i];
            if (m_phase >= 1)
                m_phase -= 1;
        }
    }

    void configure(const AudioConfig& ac)
    {
        m_inv_Fs = 1.0 / ac.sample_rate;
    }

private:

    float m_inv_Fs;
    float m_phase;

};

#endif /* !NAIVE_SAW_included */
