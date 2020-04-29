#ifndef TIMBRE_included
#define TIMBRE_included

#include "synth/core/action.h"
#include "synth/core/config.h"
#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/plan.h"
#include "synth/util/noalloc.h"

class Patch;

// A Timbre has:
//     a patch reference
//     a "factory default" patch
//     a plan
//     per-timbre controls
//     per-timbre modules
//     a pre-voice render action sequence
//     a post-voice render action sequence
//
// A Timbre can:
//     pre-render a frame chunk.
//     post-render a frame chunk.

class Timbre {

public:

    typedef fixed_vector<Control *, MAX_TIMBRE_CONTROLS> control_vector;
    typedef fixed_vector<Module *, MAX_TIMBRE_MODULES> module_vector;

    Timbre()
    : m_current_patch{nullptr},
      m_default_patch{nullptr},
      m_plan{Plan{}},
      m_controls{},
      m_modules{},
      m_pre_actions{},
      m_post_actions{}
    {}

    Timbre(const Timbre& that)
    : m_current_patch{that.m_current_patch},
      m_default_patch{that.m_default_patch},
      m_plan{that.m_plan},
      m_controls{},
      m_modules{},
      m_pre_actions{},
      m_post_actions{}
    {
        for (auto *c: that.m_controls)
            m_controls.push_back(c->clone());
        for (auto *m: that.m_modules)
            m_modules.push_back(m->clone());
    }

    Timbre& operator = (const Timbre&) = delete;

    ~Timbre()
    {
        while (!m_modules.empty()) {
            delete m_modules.back();
            m_modules.pop_back();
        }
    }

    Patch *current_patch() const { return m_current_patch; }
    void set_patch(Patch *p) { m_current_patch = p; }

    Patch *default_patch() const { return m_default_patch; }
    void default_patch(Patch *p) { m_default_patch = p; }

    const Plan& plan() const { return m_plan; }
    void plan(const Plan& p) { m_plan = p; }

    const control_vector& controls() const { return m_controls; }
    void add_control(Control *c) { m_controls.push_back(c); }

    const module_vector& modules() const { return m_modules; }
    void add_module(Module *m) { m_modules.push_back(m); }

    const render_action_sequence pre_actions() const { return m_pre_actions; }
    void pre_actions(const render_action_sequence& a) { m_pre_actions = a; }

    const render_action_sequence post_actions() const { return m_post_actions; }
    void post_actions(const render_action_sequence& a) { m_post_actions = a; }

    void pre_render(size_t frame_count) const
    {
        for (auto& a: m_pre_actions)
            a(frame_count);
    }

    void post_render(size_t frame_count) const
    {
        for (auto& a: m_post_actions)
            a(frame_count);
    }

private:
    Patch *m_current_patch;
    Patch *m_default_patch;     // XXX should be a copy, not a pointer.
    Plan m_plan;
    control_vector m_controls;
    module_vector m_modules;
    render_action_sequence m_pre_actions;
    render_action_sequence m_post_actions;

    friend class timbre_unit_test;

};

#endif /* !TIMBRE_included */
