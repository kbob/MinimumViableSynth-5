#ifndef VOICE_included
#define VOICE_included

#include <vector>

#include "synth/core/module.h"
#include "synth/core/plan.h"
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

    Voice *clone() const
    {
        Voice *v = new Voice();
        for (const auto *m: m_modules)
            v->module(m->clone());
        return v;
    }

    void init(Plan *plan) { m_plan = plan; }
    const module_vector& modules() const { return m_modules; }

private:

    module_vector m_modules;
    Plan *m_plan;

    friend class VoiceUnitTest;

};

#endif /* !VOICE_included */
