#ifndef RESOLVER_included
#define RESOLVER_included

#include <cassert>

#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/sizes.h"
#include "synth/util/deferred.h"
#include "synth/util/fixed-vector.h"
#include "synth/util/universe.h"

class Port;


// -- Resolver -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A Resolver resolves object indices into pointers to the objects.
// It handles three types of indices: Control, Module, and Port.
//
// The client loads the Resolver with controls and modules using
// `add_controls`and `add_modules`.  (The resolver finds the ports
// itself by searching the controls and modules.)
//
// Then, the client finalizes the Resolver.  After finalization, the
// Resolver provides a Universes for the three object types.
// Each Universe maps indices to pointers by subscripting, and maps
// pointers to indices with the `find` or `index` methods.
//
// Example.
//
//      Resolver my_resolver;
//      my_resolver.add_modules(my_modules.begin(), my_modules.end());
//      my_resolver.finalize();
//      Port *ptr = my_resolver.ports()[my_index];
//      size_t index = my_resolver.modules().index(some_module_ptr);
//
//
// The implementation is tricky.  To avoid memory allocation, the
// Universes are stored in the Resolver but are not constructed
// until `finalize` is called.

class Resolver {

    typedef fixed_vector<Control *, MAX_CONTROLS> cvec_type;
    typedef fixed_vector<Module *, MAX_MODULES> mvec_type;
    typedef fixed_vector<Port *, MAX_PORTS> pvec_type;

public:

    typedef Universe<cvec_type, MAX_CONTROLS> controls_type;
    typedef Universe<mvec_type, MAX_MODULES> modules_type;
    typedef Universe<pvec_type, MAX_PORTS> ports_type;

    template <class I>
    Resolver& add_controls(I first, I last)
    {
        assert(!m_controls.is_constructed());
        for (I it = first; it != last; ++it) {
            Control *c = *it;
            m_cvec.push_back(c);
            auto& p = c->ports();
            m_pvec.insert(m_pvec.end(), p.begin(), p.end());
        }
        return *this;
    }

    template <class I>
    Resolver& add_modules(I first, I last)
    {
        assert(!m_modules.is_constructed());
        for (I it = first; it != last; ++it) {
            Module *m = *it;
            m_mvec.push_back(m);
            auto& p = m->ports();
            m_pvec.insert(m_pvec.end(), p.begin(), p.end());
        }
        return *this;
    }

    void finalize()
    {
        m_controls.construct(m_cvec);
        m_modules.construct(m_mvec);
        m_ports.construct(m_pvec);
    }

    const controls_type& controls() const
    {
        assert(m_controls.is_constructed());
        return *m_controls;
    }

    const modules_type& modules() const
    {
        assert(m_modules.is_constructed());
        return *m_modules;
    }

    const ports_type& ports() const
    {
        assert(m_ports.is_constructed());
        return *m_ports;
    }

private:

    cvec_type m_cvec;
    mvec_type m_mvec;
    pvec_type m_pvec;

    deferred<controls_type> m_controls;
    deferred<modules_type> m_modules;
    deferred<ports_type> m_ports;

};

#endif /* !RESOLVER_included */
