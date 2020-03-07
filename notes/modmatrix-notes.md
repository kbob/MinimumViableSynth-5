Concepts
    audio streams
    control streams
    components

More Concepts
    routings
    inputs = sources
    outputs = sinks
    connections
    plans
    layers
    graph
    crossbar

Lots of ways to categorize streams

    audio vs control
    mono vs stereo vs multichannel
    by sample rate
    global vs per-voice vs per-component
    from component, from parameter, from MIDI
    by group (clump)
    units
    linear vs exponential vs other mappings
    transforms

    AU parameter info:
        name
        unitName
        clumpID
        cfNameString
        unit
        minValue
        maxValue
        defaultValue
        flags

        there are 27 kinds of units.
            generic             indexed             Boolean
            percent             seconds             sample frames
            phase               rate                Hertz
            cents (relative)    relative semitones  MIDI note number
            MIDI controller     decibels            linear gain
            degrees             EqualPowerCrossFade mixer fader curve 1
            pan                 meters              absolute cents
            octaves             BPM                 beats
            milliseconds        ratio               custom unit

        flags are a grab bag, but they include 6 display options:
            linear
            square root
            squared
            cubed
            cube root
            exponential
            logarithmic (can be composed with the others?)

component categories

    audio or control
    visible or hidden

But eventually, there's a flow graph.

Every control period:

    run control components in flow graph order
    if any audio component's input controls have changed, call update().

Every sample period:

    run audio components in flow graph order

Build the graph

    instantiate components for prototype voice
    components declare inputs and outputs
    declare permanent connections.

        class MIDIIn ... {
            ControlOutput m_pitchbend;
            ...
        };
        class Oscillator ... {
            ControlInput m_freq;
            ...
        };
        m_midi_in = MIDIIn();
        m_osc1 = FooOscillator();
        connect(m_midi_in.m_pitchbend, m_osc1.m_freq, ...);

    Graph construction algorithm

        def is_ready(c):
            return all(p.component in done
                       for i in c.inputs
                       for p in i.connections)
        done = {}
        not_done = {components}
        layers = []
        while not_done:
            layer = {c for c in not_done if is_ready(c)}
            if layer == {}:
               raise CyclicGraphException()
            layers.append(layer)
            done += layer
            not_done -= layer

Dynamic connections

    atomic operations to add and remove connections
    and check/recalculate graph order
    when graph turns out cyclic, reject changes.

    incremental graph order check: if new edge connects
    lower layer to higher layer, graph remains acyclic.
    Otherwise, recalculate whole graph.

----------------------------------
Component hierarchy

VoiceAssign (hidden)

UNzipper (hidden)

Osc
  LFO
  QBLOsc
  WTOsc
  SampleOsc
  Noise
  ...

Mixer

Filter
  Ladder
  SVF
  SEM
  ...

EnvGen
  ADSR
  ...

DCA
  DCA

Decimator
  Decimator

If we can do this with templates instead of inheritance, there will
be no virtual function overhead...





----------------------------------

Can control components depend on audio components?
I.e., can a component have both control and audio outputs?
  - envelope generator triggers voice release
  - if control rate == audio rate, there is no difference.

Could generalize audio/control to sample rate: 192KHz, 48KHz, 1Khz, none.
Could even have several control rates.
  - can't think of good reason for multiple control rates.

By connecting DCA to decimator, force DCA to run at 4X rate.

"Jack" and "cable"/"cord" are metaphors for inputs/outputs and connections.

What about clocks?  That's a stream, isn't it?
