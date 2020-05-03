#ifndef PORTED_included
#define PORTED_included

#include "synth/core/config.h"
#include "synth/core/ports.h"
#include "synth/util/fixed-vector.h"

// Ported is a mixin class for objects that have ports.
class Ported {

public:

    typedef fixed_vector<Port *, MODULE_MAX_PORTS> port_vector;

    const port_vector& ports() const { return m_ports; }

protected:

    Ported() = default;
    Ported(const Ported& that)
    : m_ports{}
    {
        uintptr_t int_this = reinterpret_cast<uintptr_t>(this);
        uintptr_t int_that = reinterpret_cast<uintptr_t>(&that);
        ptrdiff_t delta = int_this - int_that;
        for (auto& p: that.m_ports) {
            uintptr_t int_p = reinterpret_cast<uintptr_t>(p);
            int_p += delta;
            Port *ptr_p = reinterpret_cast<Port *>(int_p);
            m_ports.push_back(ptr_p);
        }
    }
    Ported& operator = (const Ported&) = delete;
    virtual ~Ported() = default;

    template <typename... Types>
    void ports(Port& p, Types&... rest)
    {
        m_ports.push_back(&p);
        p.owner(*this);
        ports(rest...);
    }

private:

    port_vector m_ports;

    friend class PortedUnitTest;

};

#endif /* !PORTED_included */
