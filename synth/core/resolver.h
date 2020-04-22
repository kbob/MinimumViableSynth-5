#ifndef RESOLVER_included
#define RESOLVER_included

#include <cassert>

#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/ports.h"
#include "synth/util/universe.h"


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
//      Port *ptr = my_resolver.ports()[my_index];
//      size_t index = my_resolver.modules().index(some_module_ptr);
//
//
// The implementation is tricky.  To avoid memory allocation, the
// Universes are stored in the Resolver but are not constructed
// until `finalize` is called.


class Resolver {

    static const size_t MAX_CONTROLS = 4;
    static const size_t MAX_MODULES = 16;
    static const size_t MAX_PORTS = 20;

    typedef fixed_vector<Control *, MAX_CONTROLS> cvec_type;
    typedef fixed_vector<Module *, MAX_MODULES> mvec_type;
    typedef fixed_vector<Port *, MAX_PORTS> pvec_type;

public:

    typedef Universe<cvec_type, MAX_CONTROLS> controls_type;
    typedef Universe<mvec_type, MAX_MODULES> modules_type;
    typedef Universe<pvec_type, MAX_PORTS> ports_type;

    Resolver() : m_finalized{false} {}
    ~Resolver()
    {
        if (m_finalized) {
            m_controls.member.~controls_type();
            m_modules.member.~modules_type();
            m_ports.member.~ports_type();
        }
    }

    template <class I>
    void add_controls(I first, I last)
    {
        assert(!m_finalized);
        for (I it = first; it != last; ++it) {
            Control *c = *it;
            m_cvec.push_back(c);
            auto& p = c->ports();
            m_pvec.insert(m_pvec.end(), p.begin(), p.end());
        }
    }

    template <class I>
    void add_modules(I first, I last)
    {
        assert(!m_finalized);
        for (I it = first; it != last; ++it) {
            Module *m = *it;
            m_mvec.push_back(m);
            auto& p = m->ports();
            m_pvec.insert(m_pvec.end(), p.begin(), p.end());
        }
    }

    void finalize()
    {
        new (&m_controls.member) controls_type(m_cvec);
        new (&m_modules.member) modules_type(m_mvec);
        new (&m_ports.member) ports_type(m_pvec);
        m_finalized = true;
    }

    const controls_type& controls() const
    {
        assert(m_finalized);
        return m_controls.member;
    }

    const modules_type& modules() const
    {
        assert(m_finalized);
        return m_modules.member;
    }

    const ports_type& ports() const
    {
        assert(m_finalized);
        return m_ports.member;
    }

private:

    template <class U>
    union lazy {
        lazy() {}
        ~lazy() {}
        U member;
    };

    bool m_finalized;
    cvec_type m_cvec;
    mvec_type m_mvec;
    pvec_type m_pvec;
    lazy<controls_type> m_controls;
    lazy<modules_type> m_modules;
    lazy<ports_type> m_ports;

};

#endif /* !RESOLVER_included */
