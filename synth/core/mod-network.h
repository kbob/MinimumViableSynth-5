#ifndef MOD_NETWORK_included
#define MOD_NETWORK_included

#include "synth/core/mod-vector.h"
#include "synth/core/plan.h"
#include "synth/core/signalgraph.h"
#include "synth/core/voice.h"
#include "synth/util/bijection.h"

// ModNetwork is short for both Module Network and Modulation Network.
class ModNetwork {

public:

    static const size_t MAX_MODULES = 16;
    static const size_t MAX_PORTS = 16;
    static const size_t MAX_LINKS = 16;
    typedef std::uint16_t module_mask;
    typedef std::uint16_t port_mask;


    ModNetwork& module(Module& m) { m_modules.push_back(&m); return *this; }

    ModNetwork& simple_connection(OutputPort& src, InputPort& dest)
    {
        // assert (src, dest) not in simple_connections
        m_simple_links.push_back(SimpleLink(src, dest));
        return *this;
    }

    ModNetwork& control_connection(ControlLink& link)
    {
        m_control_links.push_back(&link);
        return *this;
    }

    Voice *make_voice() const
    {
        auto v = new Voice();
        for (const auto *m : m_modules)
            v->module(m->clone());
        return v;
    }

    Plan make_plan() const;

private:

    typedef mod_vector::port_vector port_vector;
    typedef std::array<module_mask, MAX_MODULES> module_adjacency_matrix;
    typedef std::array<port_mask, MAX_PORTS> port_adjacency_matrix;

    mod_vector m_modules;
    fixed_vector<SimpleLink, MAX_LINKS> m_simple_links;
    fixed_vector<ControlLink *, MAX_LINKS> m_control_links;

    void init_mod_predecessors(module_adjacency_matrix&) const;
    void init_port_sources(const port_vector& ports,
                           port_adjacency_matrix&) const;

    friend class ModNetworkUnitTest;

};

#endif /* !MOD_NETWORK_included */
