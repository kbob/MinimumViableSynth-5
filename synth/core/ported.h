#ifndef PORTED_included
#define PORTED_included

#include "synth/core/ports.h"
#include "synth/util/noalloc.h"

// Ported is a mixin class for objects that have ports.
class Ported {

public:

    static const size_t MAX_PORTS = 4; // This is Procrustean.
    typedef fixed_vector<Port *, MAX_PORTS> port_vector;

    const port_vector& ports() const { return m_ports; }

protected:

    Ported() = default;
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
