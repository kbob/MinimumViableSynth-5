#ifndef MODULE_included
#define MODULE_included

#include <cassert>
#include <string>

#include "synth/core/action.h"
#include "synth/core/ported.h"
#include "synth/core/ports.h"

class Config;
class Timbre;


// -- Modules  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A module is a unit of audio processing.  It is very much analogous to
// a module in a modular synth: it accepts several continuous data
// streams, has several controls, and emits one or more continuous data
// streams.
//
// Data flows into and out of a module through ports (see above).
// The ports should be declared as public data members of the module.
//
// A module may have a name.  Its creator can set the name by calling
// `name()`.
//
// A module has a `clone` member function.  `clone` duplicates the
// module.
//
// A module has a `render` member function.  The `render` function
// processes a block of samples.  It reads from its input ports
// and write to its output ports for every sample.
//
// A module may have an `init` member function.  The `init` function
// is called whenever the module's voice is activated.  (i.e., at
// The beginning of a note.)
//
// A module's constructor must "declare" the module's ports by
// passing them to `ports()`.

// Module is an abstract base class for modules.
class Module : public Ported {

public:

    virtual ~Module() = default;

    void name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& name() const
    {
        return m_name;
    }

    void set_timbre(Timbre *t) { m_timbre = t; }


    virtual Module *clone() const = 0;
    virtual void configure(const Config&) {}
    virtual render_action make_render_action() = 0;

    // `twin` is a wart for Summers to associate their voice sides
    // with their timbre sides.
    virtual Module *twin() const { return nullptr; }

    // Voice modules may override these to participate in voice lifetime
    // management.
    virtual void start_note() {}
    virtual void release_note() {}
    virtual void kill_note() {}
    virtual void idle() {}
    virtual bool note_is_done() const
    {
        // This will never be called for most module subclasses,
        // but the default implementation is useless, so subclasses
        // that do use it (e.g. VCAs) should override.
        assert(!"subclass should override");
        return true;
    }

protected:

    Module()
    : m_timbre{nullptr}
    {}

    Module(const Module& that)
    : Ported{that},
      m_timbre{nullptr},
      m_name{that.m_name}
    {}

    Timbre *m_timbre;

private:

    std::string m_name;

    friend class modules_unit_test;

};

// `ModuleType` is a templated class for modules.  It uses the cursously
// recursive template pattern.
template <class M>
class ModuleType : public Module {

public:

    Module *clone() const override
    {
        assert(dynamic_cast<const M *>(this));
        return new M(static_cast<const M&>(*this));
    }

    render_action make_render_action() override
    {
        return [this] (size_t frame_count) {
            assert(dynamic_cast<M *>(this));
            static_cast<M *>(this)->render(frame_count);
        };
    }

    friend class modules_unit_test;

};

#endif /* !MODULE_included */
