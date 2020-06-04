#ifndef SIMPLE_BEEP_included
#define SIMPLE_BEEP_included

#include "synth/core/config.h"
#include "synth/core/synth.h"
#include "synth/osc/naive-square.h"

class SimpleBeep {

public:

    template <class OutputModule>
    SimpleBeep(const Config& cfg, OutputModule& out)
    : m_synth{"SimpleBeep", 1, 1}
    {
        m_synth.add_timbre_module(m_osc)
               .add_timbre_module(out, true)
               .finalize(cfg);
        Patch p;
        p.connect(out.in, m_osc.out)
         .connect(m_osc.freq, 440.)
         ;

        m_synth.apply_patch(p, m_synth.timbres().front());
    }
    SimpleBeep(const SimpleBeep&) = delete;
    SimpleBeep& operator = (const SimpleBeep&) = delete;

    const Synth& synth() const { return m_synth; }
    Synth& synth() { return m_synth; }

private:

    NaiveSquare m_osc;
    Synth m_synth;

};

#endif /* !SIMPLE_BEEP_included */
