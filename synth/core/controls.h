#ifndef CORE_CONTROLS_included
#define CORE_CONTROLS_included

#include <cassert>

#include "synth/core/action.h"
#include "synth/core/defs.h"
#include "synth/core/ported.h"
#include "synth/core/ports.h"
#include "synth/core/sizes.h"

class AudioConfig;

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

    virtual void configure(const AudioConfig&) {}
    virtual render_action make_render_action() = 0;

    // Voice controls may override these to participate in voice
    // lifetime management.
    virtual void start_note() {}
    virtual void release_note() {}
    virtual void kill_note() {}
    virtual bool note_is_done() const
    {
        // This will never be called for most control subclasses,
        // but the default implementation is useless, so controls
        // that do use it (e.g. note frequency) should override.
        assert(!"subclass should override");
        return true;
    }

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
        assert(dynamic_cast<const C *>(this));
        return new C(static_cast<const C&>(*this));
    }

    render_action make_render_action() override
    {
        assert(dynamic_cast<C *>(this));
        return [this] (size_t frame_count) {
            static_cast<C *>(this)->render(frame_count);
        };
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
