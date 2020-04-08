#ifndef MODULE_included
#define MODULE_included

#include <vector>

#include "synth/core/ported.h"
#include "synth/core/ports.h"
#include "synth/util/noalloc.h"

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

// ModuleBase is an abstract base class for modules.
class ModuleBase : public Ported {

public:

    typedef std::function<void(size_t)> action;

    void name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& name() const
    {
        return m_name;
    }

    virtual ModuleBase *clone() const = 0;
    virtual void init() {}
    virtual action make_render_action() = 0;

    // virtual void render(size_t frame_count) = 0;

protected:

    virtual ~ModuleBase() = default;

private:

    std::string m_name;

    friend class ModulesUnitTest;

};

// `Module` is a templated class for modules.  It uses the curiously
// recursive template pattern.
template <class M>
class Module : public ModuleBase {

public:

    ModuleBase *clone() const override
    {
        return new M(static_cast<const M&>(*this));
    }

    action make_render_action() override
    {
        return [this] (size_t frame_count) {
            static_cast<M *>(this)->render(frame_count);
        };
    }

    friend class ModulesUnitTest;

};

#endif /* !MODULE_included */
