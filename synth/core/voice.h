#ifndef VOICE_included
#define VOICE_included

#include <vector>

#include "synth/core/plan.h"
#include "synth/core/signalgraph.h"
#include "synth/util/noalloc.h"

class Voice {

public:

    static const size_t MAX_MODULES = 16;
    static const size_t MAX_LINKS   = 16;

    typedef fixed_vector<Module *, MAX_MODULES> module_vector;

    void module(Module *m)
    {
        m_modules.push_back(m);
    }

    // void simple_connection(OutputPort& src, InputPort& dest)
    // {
    //     m_simple_links.emplace_back(SimpleLink(src, dest));
    // }
    //
    // void control_connection(ControlLink& c)
    // {
    //     m_control_links.push_back(c);
    // }

    Voice *clone() const
    {
        Voice *v = new Voice();
        for (const auto *m: m_modules)
            v->module(m->clone());
        // for (const auto& sl: m_simple_links) {
        //     auto key = sl.key();
        //     v->simple_connection(*key.src(), *key.dest());
        // }
        // for (auto cl: m_control_links) {
        //     v->control_connection(cl);
        // }
        return v;
    }

    void init(Plan *plan) { m_plan = plan; }
    const module_vector& modules() const { return m_modules; }

private:

    module_vector m_modules;
    // fixed_vector<SimpleLink, MAX_LINKS> m_simple_links;
    // fixed_vector<ControlLink, MAX_LINKS> m_control_links;
    Plan *m_plan;

    friend class VoiceUnitTest;

};

#endif /* !VOICE_included */
