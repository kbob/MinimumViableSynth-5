#ifndef VOICE_included
#define VOICE_included

#include <algorithm>
#include <cassert>

#include "synth/core/action.h"
#include "synth/core/config.h"
#include "synth/core/defs.h"
#include "synth/core/controls.h"
#include "synth/core/modules.h"
#include "synth/core/plan.h"
#include "synth/core/sizes.h"
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
//     configure itself.
//     start, release, and kill a note.
//     render a frame chunk.

class Voice {

public:

    enum class State {
        IDLE,
        SOUNDING,
        RELEASING,
        STOPPING,
    };

    typedef fixed_vector<Control *, MAX_VOICE_CONTROLS> control_vector;
    typedef fixed_vector<Module *, MAX_VOICE_MODULES> module_vector;
    typedef fixed_vector<size_t, MAX_VOICE_LIFE_MODULES> life_vector;
    // XXX rename life_vector to shutdown_vector?

    Voice(bool delete_components = true)
    : m_delete_components{delete_components},
      m_state{State::IDLE},
      m_timbre{nullptr}
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
    void add_module(Module *m, bool is_lifetime_monitor = false)
    {
        module_vector& mods{m_modules};
        // Verify it isn't already added.
        assert(std::find(mods.begin(), mods.end(), m) == mods.end());
        if (is_lifetime_monitor)
            m_life_modules.push_back(mods.size());
        mods.push_back(m);
    }

    const render_action_sequence actions() const { return m_actions; }
    void actions(const render_action_sequence& a) { m_actions = a; }

    void configure(const Config& cfg)
    {
        m_shutdown_frames = cfg.sample_rate() * NOTE_SHUTDOWN_TIME;

        for (auto *c: m_controls)
            c->configure(cfg);
        for (auto *m: m_modules)
            m->configure(cfg);
    }

    void start_note()
    {
        assert(m_state == State::IDLE);
        m_state = State::SOUNDING;
        for (auto *c: m_controls)
            c->start_note();
        for (auto *m: m_modules)
            m->start_note();
    }

    void release_note()
    {
        assert(m_state == State::SOUNDING);
        m_state = State::RELEASING;
        for (auto *c: m_controls)
            c->release_note();
        for (auto *m: m_modules)
            m->release_note();
    }

    void kill_note()
    {
        assert(m_state == State::SOUNDING || m_state == State::RELEASING);
        m_shutdown_remaining = m_shutdown_frames;
        m_state = State::STOPPING;
        for (auto *c: m_controls)
            c->kill_note();
        for (auto *m: m_modules)
            m->kill_note();
    }

    void render(size_t frame_count)
    {
        if (m_state == State::IDLE)
            return;

        for (auto& a: m_actions)
            a(frame_count);

        if (m_state == State::RELEASING) {
            bool done = true;
            // XXX do we need life controls?
            // for (auto ci: m_life_controls)
            //     done &= m_controls[ci]->note_is_done();
            for (auto mi: m_life_modules)
                done &= m_modules[mi]->note_is_done();
            if (done)
                m_state = State::IDLE;
        } else if (m_state == State::STOPPING) {
            m_shutdown_remaining -= frame_count;
            if (m_shutdown_remaining < 0)
                m_state = State::IDLE;
        }
    }

private:

    bool m_delete_components;
    State m_state;
    Timbre *m_timbre;
    int m_shutdown_frames;
    int m_shutdown_remaining;
    control_vector m_controls;
    module_vector m_modules;
    life_vector m_life_modules;
    render_action_sequence m_actions;

    friend class voice_unit_test;

};

#endif /* !VOICE_included */
