#ifndef CONTROLS_included
#define CONTROLS_included

#include "synth/core/config.h"
#include "synth/core/ported.h"
#include "synth/core/ports.h"

// Abstract base class for controls.
//
// parameterized by type.
// subclassed for MIDI, GUI, others?
// further subclassed for note, velocity, CC, NRPN, etc.
//
// A control writes its value to its `out` port.

class Control : public Ported {

protected:

    Control() = default;
    virtual ~Control() = default;

};

template <class ElementType = DEFAULT_SAMPLE_TYPE>
class ControlType : public Control {

public:

    Output<ElementType> out;

protected:

    ControlType()
    {
        out.name("out");
        ports(out);
    }
    virtual ~ControlType() = default;

};

#endif /* !CONTROLS_included */
