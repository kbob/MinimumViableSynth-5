#ifndef CONTROLS_included
#define CONTROLS_included

// #include ""
// #include "synth/core/links.h"
// #include "synth/core/ports.h"

typedef float DEFAULT_SAMPLE_TYPE;

// ABC for controls.
//
// parameterized by type.
// subclassed for MIDI, GUI.
// further subclassed for note, velocity, CC, NRPN, etc.
//
// a control knows how to apply itself through a ControlLink.

class Control {

    // virtual void copy(size_t frame_count,
    //                   class InputPort *,
    //                   const class ControlLink&) = 0;
    //
    // virtual void add (size_t frame_count,
    //                   class InputPort *,
    //                   const class ControlLink&) = 0;

protected:

    Control() = default;
    virtual ~Control() = default;

};

template <class ElementType = DEFAULT_SAMPLE_TYPE>
class ControlType : public Control {

protected:

    ControlType() = default;
    virtual ~ControlType() = default;

};

#endif /* !CONTROLS_included */
