#ifndef PLAN_included
#define PLAN_included

#include <iostream>

#include "synth/core/actions.h"
#include "synth/core/modules.h"
#include "synth/core/resolver.h"
#include "synth/util/noalloc.h"

// A Plan has five sequences of Actions.

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

    const prep_action_sequence& t_prep()   const { return m_t_prep; }
    const prep_action_sequence& v_prep()   const { return m_v_prep; }
    const run_action_sequence&  pre_run()  const { return m_pre_run; }
    const run_action_sequence&  v_run()    const { return m_v_run; }
    const run_action_sequence&  post_run() const { return m_post_run; }

    void push_back_t_prep(const PrepAction& a)   { m_t_prep.push_back(a); }
    void push_back_v_prep(const PrepAction& a)   { m_v_prep.push_back(a); }
    void push_back_pre_run(const RunAction& a)   { m_pre_run.push_back(a); }
    void push_back_v_run(const RunAction& a)     { m_v_run.push_back(a); }
    void push_back_post_run(const RunAction& a)  { m_post_run.push_back(a); }

    // // XXX can't do this until Timbre is defined and PrepActions have
    // //     a `do_prep` method.
    // void prep_timbre(Timbre&)
    // {
    //     Resolver resolv;
    //     resolv.add_controls(timbre.controls().begin(), timbre.controls().end());
    //     resolv.add_modules(timbre.modules().begin(), timbre,modules().end());
    //     resolv.finalize();
    //     for (auto a: m_t_prep)
    //         a.do_prep(timbre, resolv);
    // }
    //
    // void prep_voice(timbre, voice) ...
    // some_type make_pre_exec(...) ...
    // some_type make_voice_exec(...) ...
    // some_type make_post_exec(...) ...

private:

    prep_action_sequence m_t_prep;
    prep_action_sequence m_v_prep;
    run_action_sequence m_pre_run;
    run_action_sequence m_v_run;
    run_action_sequence m_post_run;

};

#endif /* !PLAN_included */
