#ifndef SUMMER_included
#define SUMMER_included

#include <array>

#include "synth/core/config.h"
#include "synth/core/modules.h"
#include "synth/core/timbre.h"
#include "synth/util/fixed-vector.h"

template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Summer {

    typedef fixed_vector<const Input<ElementType> *, MAX_POLYPHONY> in_vector;

    class VoiceSide : public ModuleType<VoiceSide> {

        typedef ModuleType<VoiceSide> super;

    public:

        VoiceSide(Summer& s)
        : m_parent{s}
        {
            in.name("in");
            super::ports(in);
            m_parent.add_voice_side(this);
        }

        VoiceSide *clone() const override {
            Module *m = super::clone();
            assert(dynamic_cast<VoiceSide *>(m));
            VoiceSide *vs = static_cast<VoiceSide *>(m);
            m_parent.add_voice_side(vs);
            return vs;
        }

        Input<ElementType> in;
        void render(size_t) {}

    private:

        Summer& m_parent;

        friend class summer_unit_test;

    };

    class TimbreSide : public ModuleType<TimbreSide> {

        typedef ModuleType<TimbreSide> super;

    public:

        TimbreSide(VoiceSide *v_side, in_vector& voice_ports)
        : m_voice_side{v_side},
          m_voice_ports{voice_ports}
        {
            out.name("out");
            super::ports(out);
        }

        Module *twin() const override
        {
            return m_voice_side;
        }

        Output<ElementType> out;
        void render(size_t frame_count) {
            assert(super::m_timbre);
            auto& voices = super::m_timbre->attached_voices();
            for (size_t i = 0; i < frame_count; i++)
                out[i] = 0;
            for (size_t i = 0; i < voices.size(); i++) {
                if (voices[i]) {
                    const Input<ElementType>& v_buf = *m_voice_ports[i];
                    for (size_t j = 0; j < frame_count; j++) {
                        out[j] += v_buf[j];
                    }
                }
            }
        }

    private:

        VoiceSide *m_voice_side;
        in_vector& m_voice_ports;

        friend class summer_unit_test;

    };

    // N.B., m_voice_ports must be initialized before voice_side;
    in_vector m_voice_ports;

public:

    Summer()
    : voice_side(*this),
      timbre_side{&voice_side, m_voice_ports}
    {}

    VoiceSide voice_side;
    TimbreSide timbre_side;

private:

    void add_voice_side(VoiceSide *v) {m_voice_ports.push_back(&v->in); }

    friend class summer_unit_test;

};

#endif /* !SUMMER_included */
