#ifndef VOICE_included
#define VOICE_included

#include "synth/core/action.h"
#include "synth/core/config.h"
#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/plan.h"
#include "synth/util/fixed-vector.h"

class Timbre;

// -- Voice -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A Voice has:
//     a state
//     an owning timbre (which changes)
//     some controls
//     some modules
//     a render action sequence
//
// A Voice can:
//     start, release, and kill a note.
//     render a frame chunk.

class Voice {

public:

    enum class State {
        IDLE,
        ACTIVE,
        RELEASING,
        STOPPING,
    };

    typedef fixed_vector<Control *, MAX_VOICE_CONTROLS> control_vector;
    typedef fixed_vector<Module *, MAX_VOICE_MODULES> module_vector;

    Voice(bool delete_components = true)
    : m_delete_components{delete_components},
      m_state{State::IDLE}
    {}

    Voice(const Voice& that)
    : m_delete_components{true},
      m_state{State::IDLE},
      m_timbre{nullptr},
      m_controls{},
      m_modules{},
      m_actions{}
    {
        for (const auto *c: that.m_controls)
            m_controls.push_back(c->clone());
        for (const auto *m: that.m_modules)
            m_modules.push_back(m->clone());
    }

    Voice& operator = (const Voice&) = delete;

    ~Voice()
    {
        if (m_delete_components) {
            while (!m_modules.empty()) {
                delete m_modules.back();
                m_modules.pop_back();
            }
            while (!m_controls.empty()) {
                delete m_controls.back();
                m_controls.pop_back();
            }
        }
    }

    State state() const { return m_state; }

    Timbre *timbre() const { return m_timbre;}
    void timbre(Timbre *t) { m_timbre = t; }

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
    }

    const render_action_sequence actions() const { return m_actions; }
    void actions(const render_action_sequence& a) { m_actions = a; }

    void start_note()
    {
        m_state = State::ACTIVE;
    }

    void release_note()
    {
        m_state = State::RELEASING;
    }

    void kill_note()
    {
        m_state = State::STOPPING;
    }

    void render(size_t frame_count) const
    {
        // XXX logic for advancing a note's state.
        for (auto& a: m_actions)
            a(frame_count);
    }

private:

    bool m_delete_components;
    State m_state;
    Timbre *m_timbre;
    control_vector m_controls;
    module_vector m_modules;
    render_action_sequence m_actions;

    friend class voice_unit_test;

};

#endif /* !VOICE_included */
