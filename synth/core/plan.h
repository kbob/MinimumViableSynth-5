#ifndef PLAN_included
#define PLAN_included

#include <iostream>

#include "synth/core/actions.h"
#include "synth/core/modules.h"
#include "synth/core/resolver.h"
#include "synth/util/noalloc.h"

// A Plan has five sequences of Steps.

class Plan {

public:

    // XXX need a limits module.
    static const size_t MAX_PREP_STEPS = 16;
    static const size_t MAX_RENDER_STEPS = 16;
    typedef fixed_vector<PrepStep, MAX_PREP_STEPS> prep_step_sequence;
    typedef fixed_vector<RenderStep, MAX_RENDER_STEPS> render_step_sequence;

    Plan()                         = default;
    Plan(const Plan&)              = delete;
    Plan(Plan&&)                   = default;
    Plan& operator = (const Plan&) = delete;
    Plan& operator = (Plan&&)      = default;

#ifdef NOT_YET
    const prep_step_sequence& t_prep()        const { return m_t_prep; }
    const prep_step_sequence& v_prep()        const { return m_v_prep; }
    const render_step_sequence&  pre_render() const { return m_pre_render; }
    const render_step_sequence&  v_render()   const { return m_v_render; }
    const render_step_sequence&  post_rende() const { return m_post_render; }

    void push_back_t_prep(const PrepStep& a)        { m_t_prep.push_back(a); }
    void push_back_v_prep(const PrepStep& a)        { m_v_prep.push_back(a); }
    void push_back_pre_render(const RenderStep& a)
                                                { m_pre_render.push_back(a); }
    void push_back_v_render(const RenderStep& a)    { m_v_render.push_back(a); }
    void push_back_post_render(const RenderStep& a)
                                                { m_post_render.push_back(a); }
#else
    const prep_step_sequence& prep() const { return m_t_prep; }
    const render_step_sequence& run() const { return m_pre_render; }
    void push_back_prep(const PrepStep& a) { m_t_prep.push_back(a); }
    void push_back_run(const RenderStep& a) { m_pre_render.push_back(a); }
#endif
    // // XXX can't do this until Timbre is defined and PrepSteps have
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
    // some_type make_pre_actions(...) ...
    // some_type make_voice_actions(...) ...
    // some_type make_post_actions(...) ...

private:

    prep_step_sequence   m_t_prep;
    prep_step_sequence   m_v_prep;
    render_step_sequence m_pre_render;
    render_step_sequence m_v_render;
    render_step_sequence m_post_render;

};

#endif /* !PLAN_included */
