#ifndef CORE_CONTROLS_included
#define CORE_CONTROLS_included

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

public:

    virtual Control *clone() const = 0;
    virtual ~Control() = default;

protected:

    Control() = default;

    friend class core_controls_unit_test;

};

template <class C, class ElementType = DEFAULT_SAMPLE_TYPE>
class ControlType : public Control {

public:

    Output<ElementType> out;

    Control *clone() const override
    {
        return new C(static_cast<const C&>(*this));
    }

protected:

    ControlType()
    {
        out.name("out");
        ports(out);
    }
    virtual ~ControlType() = default;

    friend class core_controls_unit_test;

};

#endif /* !CORE_CONTROLS_included */
