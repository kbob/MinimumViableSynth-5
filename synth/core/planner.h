#ifndef PLANNER_included
#define PLANNER_included

#include "links.h"
#include "synth/core/modules.h"
#include "synth/core/mod-vector.h"
#include "synth/core/plan.h"
#include "synth/core/ported.h"
#include "synth/core/voice.h"
#include "synth/util/bijection.h"

// A Planner creates a Plan from a set of controls, modules, links,
//and control values.

class Planner {

public:

    static const size_t MAX_MODULES = 16;
    static const size_t MAX_PORTS = 16;
    static const size_t MAX_LINKS = 16;
    typedef std::uint16_t module_mask;
    typedef std::uint16_t port_mask;


    Planner& module(Module& m) { m_modules.push_back(&m); return *this; }
    Planner& connection(Link& l) { m_links.push_back(&l); return *this; }

    Plan make_plan() const;

private:

    typedef mod_vector::port_vector port_vector;
    typedef std::array<module_mask, MAX_MODULES> module_adjacency_matrix;
    typedef std::array<port_mask, MAX_PORTS> port_adjacency_matrix;

    mod_vector m_modules;
    fixed_vector<Link *, MAX_LINKS> m_links;

    void init_mod_predecessors(module_adjacency_matrix&) const;
    void init_port_sources(const port_vector& ports,
                           port_adjacency_matrix&) const;

    friend class planner_unit_test;

};

#endif /* !PLANNER_included */
