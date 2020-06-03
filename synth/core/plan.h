#ifndef PLAN_included
#define PLAN_included

#include <iostream>

#include "synth/core/sizes.h"
#include "synth/core/steps.h"
#include "synth/util/fixed-vector.h"

// A Plan has five sequences of Steps.

class Plan {

public:

    typedef fixed_vector<PrepStep, MAX_PREP_STEPS> prep_step_sequence;
    typedef fixed_vector<RenderStep, MAX_RENDER_STEPS> render_step_sequence;

    const prep_step_sequence&   t_prep()      const { return m_t_prep; }
    const prep_step_sequence&   v_prep()      const { return m_v_prep; }
    const render_step_sequence& pre_render()  const { return m_pre_render; }
    const render_step_sequence& v_render()    const { return m_v_render; }
    const render_step_sequence& post_render() const { return m_post_render; }

    prep_step_sequence&         t_prep()            { return m_t_prep; }
    prep_step_sequence&         v_prep()            { return m_v_prep; }
    render_step_sequence&       pre_render()        { return m_pre_render; }
    render_step_sequence&       v_render()          { return m_v_render; }
    render_step_sequence&       post_render()       { return m_post_render; }

private:

    prep_step_sequence          m_t_prep;
    prep_step_sequence          m_v_prep;
    render_step_sequence        m_pre_render;
    render_step_sequence        m_v_render;
    render_step_sequence        m_post_render;

};

inline std::ostream&
operator << (std::ostream& o, const Plan::prep_step_sequence& seq)
{
    o << '[';
    const char *sep = "";
    for (auto& step: seq) {
        o << sep << step;
        sep = " ";
    }
    return o << ']';
}

inline std::ostream&
operator << (std::ostream& o, const Plan::render_step_sequence& seq)
{
    o << '[';
    const char *sep = "";
    for (auto& step: seq) {
        o << sep << step;
        sep = " ";
    }
    return o << ']';
}

#endif /* !PLAN_included */
