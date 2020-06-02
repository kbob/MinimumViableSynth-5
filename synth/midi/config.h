#ifndef MIDI_CONFIG_included
#define MIDI_CONFIG_included

#include "synth/core/config.h"

namespace midi {

    // As in synth/core/config.h, platforms, targets, and configs
    // will need to override many of these values.
    //
    // The difference is that the MIDI constants are also defined
    // in the `midi` namespace.


    // Number of RPNs that can be tracked
    // ~~The MIDI standard defines 5 RPNs.~~
    // As of MIDI CA-026, 1999-03-02, 6 RPNs are defined.
    #ifndef MIDI_MAX_RPNS
    #define MIDI_MAX_RPNS 6
    #endif
    static const size_t MAX_RPNS = MIDI_MAX_RPNS;

    // Number of NRPNs that can be tracked
    #ifndef MIDI_MAX_NRPNS
    #define MIDI_MAX_NRPNS 4
    #endif
    static const size_t MAX_NRPNS = MIDI_MAX_NRPNS;

    // 409 is a good size.
    // That is the size of a KEY-BASED TUNING DUMP message
    #ifndef MIDI_MAX_SYSEX_SIZE
    #define MIDI_MAX_SYSEX_SIZE 10
    #endif
    static const size_t MAX_SYSEX_SIZE = MIDI_MAX_SYSEX_SIZE;

    // Number of distinct SYSEX manufactuter IDs recognized.
    // This does not include Universal manufacturer IDs.
    #ifndef MIDI_MAX_SYSEX_IDS
    #define MIDI_MAX_SYSEX_IDS 4
    #endif
    static const size_t MAX_SYSEX_IDS = MIDI_MAX_SYSEX_IDS;

}

#endif /* !MIDI_CONFIG_included */
