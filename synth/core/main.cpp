#include "signalgraph.h"

#include <sstream>

class Attenuator : public Module {

public:

    Attenuator()
    {
        in.name("signal_in");
        out.name("signal_out");
        gain.name("gain");
        ports(in, out, gain);
    }

    Input<> in;
    Output<> out;
    Control<> gain;

    virtual void render(State *, size_t frame_count) const
    {
        for (size_t i = 0; i < frame_count; i++) {
            // out[i] = gain[i] * in[i];
        }
    }
};

class Oscillator : public Module {

public:

    Oscillator()
    {
        out.name("osc_out");
        ports(out);
    }

    Output<> out;

};

class BLOscillator : public Oscillator {

    typedef Oscillator super;

public:

    BLOscillator()
    {
        pitch_bend.name("pitch_bend");
        modulation.name("modulation");
        ports(pitch_bend, modulation);
    }

    Control<> pitch_bend;
    Control<> modulation;

    virtual void render(Module::State *, size_t) const
    {
        // auto state = dynamic_cast<State *>(modstate);
        // state->note = modulation[0];

        // auto state = dynamic_cast<State *>(modstate);
        // for (size_t i = 0; i < frame_count; i++) {
        //     out[i] = state->note + pitch_bend[i] + modulation[i];
        // }
    }

private:

    class State : public super::State {
    public:
        uint8_t note;
    };

};

template <unsigned ChannelsIn>
class Mixer : public Module {

public:

    Mixer()
    {
        for (unsigned i = 0; i < ChannelsIn; i++) {
            std::ostringstream in_name;
            in_name << "in_" << i;
            in[i].name(in_name.str());
            ports(in[i]);
            std::ostringstream gain_name;
            gain_name << "gain_" << i;
            gain[i].name(gain_name.str());
            ports(gain[i]);
        }
        out.name("out");
        ports(out);
    }

    Input<> in[ChannelsIn];
    Control<> gain[ChannelsIn];
    Output<> out;

    virtual void render(Module::State *, size_t) const
    {}

};

class StupidSynth {

public:

    StupidSynth()
    {
        g.module(o)
         .module(a)
         .module(m)
         .connect(o.out, a.in)
         .connect(a.out, m.in[0])
         ;
    }

    SignalGraph& graph()
    {
        return g;
    }

private:

    Attenuator a;
    BLOscillator o;
    Mixer<1> m;
    SignalGraph g;

};

int main()
{
    StupidSynth ss;
    return 0;
}
