#include "synth/core/signalgraph.h"

#include <array>
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

        Link *link;
        link = new ControlLink(o.out, a.gain, new Control());
        gain_link = std::unique_ptr<Link>(link);
        link = new ControlLink(o.out, a.gain, new Control());
        gain_link_2 = std::unique_ptr<Link>(link);

        g.module(o)
         .module(a)
         .module(m)
         .connect(o.out, a.in)
         .connect(a.out, m.in[0])
         .connection(gain_link.get())
         .connection(gain_link_2.get())
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
    std::unique_ptr<Link> gain_link_2;
    SignalGraph g;

};

static void print_plan(const Plan& plan)
{
    std::cout << "plan = (" << std::endl;

    std::cout << "    prep = [" << std::endl;
    for (size_t i = 0; i < plan.prep().size(); i++) {
        const Action *a = plan.prep()[i].get();
        std::cout << "        " << a->repr() << "\n";
    }
    std::cout << "    ]," << std::endl;

    std::cout << "    order = [" << std::endl;
    for (size_t i = 0; i < plan.order().size(); i++) {
        const Action *a = plan.order()[i].get();
        std::cout << "        " << a->repr() << "\n";
    }
    std::cout << "    ]," << std::endl;

    std::cout << ")\n" << std::endl;
}

// static void print_order(const SignalGraph::Order& order)
// {
//     std::cout << "order = [\n";
//     for (auto a : order)
//         std::cout << "    " << a->repr() << "\n";
//     std::cout << "]\n" << std::endl;
// }

int main()
{
    StupidSynth ss;
    ss.graph().dump_maps();

    Plan plan = ss.graph().new_plan();
    print_plan(plan);
    // SignalGraph::Order order = ss.graph().plan();
    // std::cout << "# With attenuator gain disabled" << std::endl;
    // print_order(order);

    // ss.enable_attenuator_gain();
    //
    // order = ss.graph().plan();
    // std::cout << "# With attenuator gain enabled" << std::endl;
    // print_order(order);

    return 0;
}
