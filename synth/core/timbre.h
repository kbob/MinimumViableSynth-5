#ifndef TIMBRE_included
#define TIMBRE_included

#include <algorithm>
#include <bitset>
#include <cassert>

#include "synth/core/action.h"
#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/patch.h"
#include "synth/core/plan.h"
#include "synth/core/sizes.h"
#include "synth/util/fixed-vector.h"

class Config;
class Patch;

// A Timbre has:
//     a patch reference
//     a "factory default" patch
//     a plan
//     per-timbre controls
//     per-timbre modules
//     a pre-voice render action sequence
//     a post-voice render action sequence
//     a bit vector of attached voices
//
// A Timbre can:
//     configure itself.
//     pre-render a frame chunk.
//     post-render a frame chunk.

class Timbre {

public:

    typedef fixed_vector<Control *, MAX_TIMBRE_CONTROLS> control_vector;
    typedef fixed_vector<Module *, MAX_TIMBRE_MODULES> module_vector;
    typedef std::bitset<MAX_POLYPHONY> voice_set;

    Timbre(bool delete_components = true)
    : m_delete_components{delete_components},
      m_current_patch{nullptr},
      m_default_patch{nullptr},
      m_plan{Plan{}},
      m_controls{},
      m_modules{},
      m_pre_actions{},
      m_post_actions{}
    {}

    Timbre(const Timbre& that)
    : m_delete_components{true},
      m_current_patch{that.m_current_patch},
      m_default_patch{that.m_default_patch},
      m_plan{that.m_plan},
      m_controls{},
      m_modules{},
      m_pre_actions{},
      m_post_actions{}
    {
        assert(this != &that);
        for (auto *c: that.m_controls)
            m_controls.push_back(c->clone());
        for (auto *m: that.m_modules) {
            auto new_m = m->clone();
            m_modules.push_back(new_m);
            new_m->set_timbre(this);
        }
    }

    Timbre& operator = (const Timbre&) = delete;

    ~Timbre()
    {
        if (m_delete_components) {
            while (!m_controls.empty()) {
                delete m_controls.back();
                m_controls.pop_back();
            }
            while (!m_modules.empty()) {
                delete m_modules.back();
                m_modules.pop_back();
            }
        }
    }

    Patch *current_patch() const { return m_current_patch; }
    void set_patch(Patch *p) { m_current_patch = p; }

    Patch *default_patch() const { return m_default_patch; }
    void default_patch(Patch *p) { m_default_patch = p; }

    const Plan& plan() const { return m_plan; }
    void plan(const Plan& p) { m_plan = p; }

    const control_vector& controls() const { return m_controls; }
    void add_control(Control *c)
    {
        control_vector& ctls{m_controls};
        // Verify it isn't already added.
        assert(std::find(ctls.begin(), ctls.end(), c) == ctls.end());
        ctls.push_back(c);
    }

    const module_vector& modules() const { return m_modules; }
    void add_module(Module *m)
    {
        module_vector& mods{m_modules};
        // Verify it isn't already added.
        assert(std::find(mods.begin(), mods.end(), m) == mods.end());
        mods.push_back(m);
        m->set_timbre(this);
    }

    const render_action_sequence pre_actions() const { return m_pre_actions; }
    void pre_actions(const render_action_sequence& a) { m_pre_actions = a; }

    const render_action_sequence post_actions() const { return m_post_actions; }
    void post_actions(const render_action_sequence& a) { m_post_actions = a; }

    const voice_set& attached_voices() const { return m_attached_voices; }
    void add_voice(size_t index) { m_attached_voices.set(index); }
    void remove_voice(size_t index) { m_attached_voices.reset(index); }

    void configure(const Config& cfg)
    {
        for (auto *c: m_controls)
            c->configure(cfg);
        for (auto *m: m_modules)
            m->configure(cfg);
    }

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

    bool m_delete_components;
    Patch *m_current_patch;
    Patch *m_default_patch;     // XXX should be a copy, not a pointer.
    Plan m_plan;
    control_vector m_controls;
    module_vector m_modules;
    render_action_sequence m_pre_actions;
    render_action_sequence m_post_actions;
    voice_set m_attached_voices;

    friend class timbre_unit_test;

};

#endif /* !TIMBRE_included */
