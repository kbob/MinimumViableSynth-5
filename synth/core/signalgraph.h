#ifndef SIGNALGRAPH_included
#define SIGNALGRPAH_included

#include <cstddef>

typedef float sample;

// module has subclasses.
// module subclasses have inputs and outputs.
// module has way of looking up i's and o's.
// i's and o's are tied to their module somehow.

class Module {

public:

    virtual ~Module();

    class State {};

    virtual State *create_state() const { return new State(); }
    virtual void init_note_state(State *) const {}

    virtual void render(State *, unsigned frame_count) const;

};

class Port {};
// Port has operator [] that knows how to find the right value.
// Should it also have iterators?  Sigh...

template <class ElementType = sample>
class Input : public Port {

public:

    virtual ElementType operator [] (size_t i) const;

};

template <class ElementType = sample>
class Output : public Port {

public:

    virtual ElementType& operator [](size_t i);

};

// A control can be a modulation input, a MIDI CC, or a GUI parameter.
// Control will need ways to specify name, min, max, range, display,
// etc.
template <class ValueType = double>
class Control : public Port {

public:

    ValueType operator [](size_t i) const;

};

class Attenuator : public Module {

public:

    Input<> in;
    Output<> out;
    Control<> gain;

};

class Oscillator : public Module {

public:

    Output<> out;

};

class BLOscillator : public Oscillator {

    typedef Oscillator super;

public:

    class State : super::State {
    public:
        uint8_t note;
    };

    Control<> pitch_bend;
    Control<> modulation;

    void render(State *state, size_t frame_count)
    {
        for (size_t i = 0; i < frame_count; i++) {
            out[i] = state->note + pitch_bend[i] + modulation[i];
        }
    }

};

template <unsigned ChannelsIn>
class Mixer : public Module {

public:

    Input<> in[ChannelsIn];
    Output<> out;

};

class SignalGraph {

public:

    SignalGraph& module(const Module&);

    template <class OutputType, class InputType>
    SignalGraph& connect(const Output<OutputType>&, const Input<InputType>&);

    template <class OutputType, class InputType>
    void disconnect(const Output<OutputType>&, const Input<InputType>&);

};

#endif /* !SIGNALGRAPH_included */

SignalGraph *foo()
{
    auto a = Attenuator();
    auto o = BLOscillator();
    auto m = Mixer<1>();

    auto g = &(*new SignalGraph())
        .module(o)
        .module(a)
        .module(m)
        .connect(o.out, a.in)
        .connect(a.out, m.in[0])
        ;

    return nullptr;

    // This will never work.
}
