#ifndef MODULE_included
#define MODULE_included

#include <vector>

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

// `Module` is an abstract base class for modules.
class Module {

public:

    static const size_t MAX_PORTS = 4;

    typedef fixed_vector<Port *, MAX_PORTS> port_vector;

    void name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& name() const
    {
        return m_name;
    }

    const port_vector& ports() const
    {
        return m_ports;
    }

    virtual Module *clone() const = 0;

    virtual void init() {}

    virtual void render(size_t frame_count) = 0;

protected:

    virtual ~Module() = default;

    template <typename... Types>
    void ports(Port& p, Types&... rest)
    {
        m_ports.push_back(&p);
        p.module(*this);
        ports(rest...);
    }

    // void ports() {}

private:

    std::string m_name;
    port_vector m_ports;
    friend class ModuleUnitTest;

};

#endif /* !MODULE_included */
