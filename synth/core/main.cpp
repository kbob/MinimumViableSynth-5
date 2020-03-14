#include "synth/core/signalgraph.h"

#include <iostream>
#include <memory>
#include <sstream>

class Attenuator : public Module {

public:

    Attenuator()
    {
        in.name("in");
        out.name("out");
        gain.name("gain");
        ports(in, out, gain);
    }

    Input<> in;
    Output<> out;
    ControlInput<> gain;

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
        pitch_bend.name("pitch_bend");
        modulation.name("modulation");
        ports(pitch_bend, modulation);
    }

    ControlInput<> pitch_bend;
    ControlInput<> modulation;

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

    struct State : public super::State {
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
    ControlInput<> gain[ChannelsIn];
    Output<> out;

    virtual void render(Module::State *, size_t) const
    {}

};

class StupidSynth {

public:

    StupidSynth()
    {
        o.name("Osc1");
        gain_link = std::unique_ptr<Link>(make_link(o.out, a.gain));
        g.module(o)
         .module(a)
         .module(m)
         .connect(o.out, a.in)
         .connect(a.out, m.in[0])
         .connection(gain_link.get())
         // .disconnect(o.out, a.in)
         ;
    }

    SignalGraph& graph()
    {
        return g;
    }

    void enable_attenuator_gain()
    {
        auto link = gain_link.get();
        auto gain = dynamic_cast<ControlLink *>(link);
        assert(gain);
        gain->enabled(true);
    }

private:

    Attenuator a;
    QBLOscillator o;
    Mixer<1> m;
    std::unique_ptr<Link> gain_link;
    SignalGraph g;

};

static void print_order(const SignalGraph::Order& order)
{
    std::cout << "order = [\n";
    for (auto a : order)
        std::cout << "    " << a->repr() << "\n";
    std::cout << "]\n" << std::endl;
}

int main()
{
    // heap_map<int, char> hm;
    // hm[42] = 'a';
    // hm[2] = 'b';
    // hm.crbegin();
    // hm.crend();
    // *hm.begin();
    // for (auto it : hm) {
    //     std::cout << it.first << ": " << it.second << ", ";
    // }
    // std::cout << std::endl;
    // return 0;

    StupidSynth ss;
    ss.graph().dump_maps();

    SignalGraph::Order order = ss.graph().plan();
    std::cout << "# With attenuator gain disabled" << std::endl;
    print_order(order);

    ss.enable_attenuator_gain();

    order = ss.graph().plan();
    std::cout << "# With attenuator gain enabled" << std::endl;
    print_order(order);

    return 0;
}
