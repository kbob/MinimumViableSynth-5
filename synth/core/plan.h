#ifndef PLAN_included
#define PLAN_included

#include <iostream>

#include "synth/core/actions.h"
#include "synth/core/modules.h"
#include "synth/core/resolver.h"
#include "synth/util/noalloc.h"

class Plan {

public:

    // XXX need a limits module.
    static const size_t MAX_PREP_ACTIONS = 16;
    static const size_t MAX_RUN_ACTIONS = 16;
    typedef fixed_vector<PrepAction, MAX_PREP_ACTIONS> prep_action_sequence;
    typedef fixed_vector<RunAction, MAX_RUN_ACTIONS> run_action_sequence;

    Plan()                         = default;
    Plan(const Plan&)              = delete;
    Plan(Plan&&)                   = default;
    Plan& operator = (const Plan&) = delete;
    Plan& operator = (Plan&&)      = default;

    const prep_action_sequence& prep() const
    {
        return m_prep;
    }

    const run_action_sequence& run() const
    {
        return m_run;
    }

    void push_back_prep(const PrepAction& action)
    {
        m_prep.push_back(action);
    }

    void push_back_run(const RunAction& action)
    {
        m_run.push_back(action);
    }

private:

    prep_action_sequence m_prep;
    run_action_sequence m_run;
    // Resolver m_resolver;

};

#endif /* !PLAN_included */
