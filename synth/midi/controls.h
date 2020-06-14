#ifndef MIDI_CONTROLS_included
#define MIDI_CONTROLS_included

#include <cassert>
#include <cstdint>

#include "synth/core/config.h"
#include "synth/core/controls.h"
#include "synth/core/defs.h"
#include "synth/midi/defs.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/messages.h"

// MIDI CCs that are not controllers
//   bank select        CC     0/32
//   portamento time    CC     5/37
//   data entry         CC     6/38
//   sustain            CC    64
//   portamento on/off  CC    65
//   sostenuto          CC    66
//   legato footswitch  CC    68
//   sound variation    CC    70
//   portamento         CC    84
//   data increment     CC    96
//   data decrement     CC    97
//   NRPN LSB           CC    98
//   NRPC MSB           CC    99
//   RPN LSB            CC   100
//   RPN MSB            CC   101
//   all sound off      CC   120     (channel mode message)
//   reset all ctlrs    CC   121     (channel mode message)
//   local control      CC   122     (channel mode message)
//   all notes off      CC   123     (channel mode message)
//   omni off           CC   124     (channel mode message)
//   omni on            CC   125     (channel mode message)
//   mono on/poly off   CC   126     (channel mode message)
//   poly on/mono off   CC   127     (channel mode message)
//   pitch bend range   RPN    0   0
//   tuning prog select RPN    0   3
//   tuning bank select RPN    0   4
//   null function      RPN  127 127
//   null function      NRPN 127 127

// non-CC controllers
//   note number
//   attack velocity
//   release velocity
//   poly key pressure
//   channel pressure
//   pitch bend
//
// CC controllers
//   modulation         CC     1/33
//   breath             CC     2/34
//   foot controller    CC     4/36
//   channel volume     CC     7/39
//   balance            CC     8/40
//   pan                CC    10/42
//   expression         CC    11/43
//   effect 1           CC    12/44
//   effect 2           CC    13/45
//   general purpose 1  CC    16/48
//   general purpose 2  CC    17/49
//   general purpose 3  CC    18/50
//   general purpose 4  CC    19/51
//   soft pedal         CC    67
//   hold 2             CC    69
//   timbre             CC    71
//   release time       CC    72
//   attack time        CC    73
//   brightness         CC    74
//   sound ctlr 6       CC    75
//   sound ctlr 7       CC    76
//   sound ctlr 8       CC    77
//   sound ctlr 9       CC    78
//   sound ctlr 10      CC    79
//   general purpose 5  CC    80
//   general purpose 6  CC    81
//   general purpose 7  CC    82
//   general purpose 8  CC    83
//   effects 1 (reverb) CC    91
//   effects 2 (tremolo) CC   92
//   effects 3 (chorus) CC    93
//   effects 4 (detune) CC    94
//   effects 5 (phaser) CC    95
//
// RPN controllers
//   fine tuning        RPN    0   1
//   coarse tuning      RPN    0   2
// NRPN controllers
//   vibrato rate       NRPN   1   8 (GS/XG)
//   vibrato depth      NRPN   1   9 (GS/XG)
//   vibrato delay      NRPN   1  10 (GS/XG)
//   filter cutoff      NRPN   1  32 (GS/XG)
//   filter resonance   NRPN   1  33 (GS/XG)
//   env. attack rate   NRPN   1  63 (GS/XG)
//   env. decay rate    NRPN   1  64 (GS/XG)
//   env. release rate  NRPN   1  66 (GS/XG)
//
// more CCs
//   undefined          CC     3/35
//   undefined          CC     9/41
//   undefined          CC    14/46
//   undefined          CC    15/47
//   undefined          CC    20/52
//   undefined          CC    21/53
//   undefined          CC    22/54
//   undefined          CC    23/55
//   undefined          CC    24/56
//   undefined          CC    25/57
//   undefined          CC    26/58
//   undefined          CC    27/59
//   undefined          CC    28/60
//   undefined          CC    29/61
//   undefined          CC    30/62
//   undefined          CC    31/63
//   undefined          CC    85
//   undefined          CC    86
//   undefined          CC    87
//   undefined          CC    88
//   undefined          CC    89
//   undefined          CC    90
//   undefined          CC   102
//   undefined          CC   103
//   undefined          CC   104
//   undefined          CC   105
//   undefined          CC   106
//   undefined          CC   107
//   undefined          CC   108
//   undefined          CC   109
//   undefined          CC   110
//   undefined          CC   111
//   undefined          CC   112
//   undefined          CC   113
//   undefined          CC   114
//   undefined          CC   115
//   undefined          CC   116
//   undefined          CC   117
//   undefined          CC   118
//   undefined          CC   119

// Let's have these classes.
//
// per-note controls
//     MIDI Note
//     MIDI Attack Velocity
//     MIDI Release Velocity
//     MIDI Poly Pressure

// per-channel controls
//     MIDI Channel Pressure
//     MIDI Pitch Bend
//
// per-channel abstract bases
//     MIDI CC
//     MIDI RPN
//     MIDI NRPN

namespace midi {

    template <class Sample = DEFAULT_SAMPLE_TYPE>
    Sample note_to_freq(uint8_t note);
    template <>
    float note_to_freq<float>(uint8_t note)
    {
        return 440.0f * powf(2.0f, (note - 69) * (1.0f / 12.0f));
    }

    class NoteFreqControl : public ControlType<NoteFreqControl> {
        // Combine note number, pitch bend, and portamento.
    public:
        void render(size_t) { assert(false && "write me!"); }
    };

    class AttackVelocityControl : public ControlType<AttackVelocityControl> {

    };

    class ReleaseVelocityControl : public ControlType<ReleaseVelocityControl> {

    };

    class PolyPressureControl : public ControlType<PolyPressureControl> {

    };

    class ChannelPressureControl : public ControlType<ChannelPressureControl> {

    public:

        void configure(const ::Config& cfg) override
        {
            auto handler = [&] (const SmallMessage& msg){
                m_pressure = msg.channel_pressure() / 127.0;
            };

            Dispatcher *disp = cfg.get<Dispatcher>();
            disp->register_handler(StatusByte::CHANNEL_PRESSURE,
                                   Dispatcher::ALL_CHANNELS,
                                   handler);
        }

        void render(size_t frame_count) {
            for (size_t i = 0; i < frame_count; i++)
                out[i] = m_pressure;
        }

    private:

        DEFAULT_SAMPLE_TYPE m_pressure;

    };

    class PitchBendControl : public ControlType<PitchBendControl> {

    };

    template <uint8_t N>
    class CCControl : public ControlType<CCControl<N>> {
    };

    template <std::uint8_t MSB, std::uint8_t LSB>
    class RPNControl : public ControlType<RPNControl<MSB, LSB>> {
        static_assert(MSB < 128 && LSB < 128, "illegal RPN");
    };

    template <std::uint8_t MSB, std::uint8_t LSB>
    class NRPNControl : public ControlType<NRPNControl<MSB, LSB>> {
        static_assert(MSB < 128 && LSB < 128, "illegal NRPN");
    };

    typedef CCControl<1> Modulation;
    typedef CCControl<2> Breath;
    typedef CCControl<4> FootController;
    typedef CCControl<7> ChannelVolume;
    typedef CCControl<8> Balance;

    //   modulation         CC     1/33
    //   breath             CC     2/34
    //   foot controller    CC     4/36
    //   channel volume     CC     7/39
    //   balance            CC     8/40
    //   pan                CC    10/42
    //   expression         CC    11/43
    //   effect 1           CC    12/44
    //   effect 2           CC    13/45
    //   general purpose 1  CC    16/48
    //   general purpose 2  CC    17/49
    //   general purpose 3  CC    18/50
    //   general purpose 4  CC    19/51
    //   soft pedal         CC    67
    //   hold 2             CC    69
    //   timbre             CC    71
    //   release time       CC    72
    //   attack time        CC    73
    //   brightness         CC    74
    //   sound ctlr 6       CC    75
    //   sound ctlr 7       CC    76
    //   sound ctlr 8       CC    77
    //   sound ctlr 9       CC    78
    //   sound ctlr 10      CC    79
    //   general purpose 5  CC    80
    //   general purpose 6  CC    81
    //   general purpose 7  CC    82
    //   general purpose 8  CC    83
    //   effects 1 (reverb) CC    91
    //   effects 2 (tremolo) CC   92
    //   effects 3 (chorus) CC    93
    //   effects 4 (detune) CC    94
    //   effects 5 (phaser) CC    95

}

#endif /* !MIDI_CONTROLS_included */
