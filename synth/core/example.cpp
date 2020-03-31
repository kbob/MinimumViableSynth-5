#include "synth/core/signalgraph.h"

class Oscillator : public Module {
public:
    Oscillator()
    {
        out.name("out");
        ports(out);
    }
    Output<> out;
};

class QBLOscillator : public Oscillator {
    typedef Oscillator super;
public:
    QBLOscillator()
    {
        m_pitch.name("pitch");
        ports(m_pitch);
    }
    virtual QBLOscillator *clone() const override
    {
        return new QBLOscillator(*this);
    }


    ControlInput<> m_pitch;
    virtual void render(size_t) const override {}
private:
};

class Filter : public Module {
public:
    Filter()
    {
        cutoff.name("cutoff frequency");
        resonance.name("resonance");
        ports(in, cutoff, resonance, out);
    }
    virtual Filter *clone() const override { return new Filter(*this); }
    virtual void render(size_t) const override {}
    Input<> in;
    ControlInput<> cutoff, resonance;
    Output<> out;

};

class Amp : public Module {
public:
    Amp()
    {
        gain.name("gain");
        ports(in, gain, out);
    }
    virtual Amp *clone() const override { return new Amp(*this); }
    virtual void render(size_t frame_count) const override
    {
        (void)frame_count;
        // for (size_t i = 0; i < frame_count; i++)
        //     out[i] = gain[i] * in[i];
    }
    Input<> in;
    ControlInput<> gain;
    Output<> out;
};

class Voice {
public:
    Voice& module(Module& m)
    {
        m_modules.push_back(&m);
        return *this;
    }
    Voice& simple_connection(OutputPort& src, InputPort& dest)
    {
        m_links.push_back(SimpleLink(src, dest));
        return *this;
    }
private:
    std::vector<Module *> m_modules;
    std::vector<SimpleLink> m_links;
};

class HorseSynth {
public:
    static const size_t POLYPHONY_COUNT = 4;

    HorseSynth()
    {
        Voice& v = m_voice[0];
        v.module(m_osc1);
        v.module(m_osc2);
        v.module(m_filter);
        v.module(m_amp);
        v.simple_connection(m_osc1.out, m_filter.in);
        v.simple_connection(m_osc2.out, m_filter.in);
        v.simple_connection(m_filter.out, m_amp.in);
    }

private:
    QBLOscillator m_osc1, m_osc2;
    Filter m_filter;
    Amp m_amp;
    Voice m_voice[POLYPHONY_COUNT];
};

int main()
{
    HorseSynth hs;
    return 0;
}
