#ifndef SYNTH_included
#define SYNTH_included

#include <cassert>

#include "synth/core/action.h"
#include "synth/core/config.h"
#include "synth/core/patch.h"
#include "synth/core/planner.h"
#include "synth/core/resolver.h"
#include "synth/core/timbre.h"
#include "synth/core/voice.h"
#include "synth/util/fixed-vector.h"

class Control;
class Module;


// -- Synth -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A Synth has:
//     a name
//     a polyphony count
//     a timbrality count
//     a voice allocation algorithm
//     a timbre archetype (controls, modules)
//     a voice archetype  (controls, modules)
//     a list of output modules
//     a set of timbres
//     a set of voices
//
// A Synth can:
//     allocate a voice
//     apply a patch to a timbre
//     apply a patch to a voice

class Synth {

public:

    typedef fixed_vector<Timbre, MAX_TIMBRALITY> timbre_vector;
    typedef fixed_vector<Voice, MAX_POLYPHONY> voice_vector;

    const char *const name;
    const size_t polyphony;
    const size_t timbrality;

    Synth(const char *name, size_t polyphony, size_t timbrality)
    : name{name},
      polyphony{polyphony},
      timbrality{timbrality},
      m_finalized{false}
    {
        assert(polyphony && timbrality);
        // m_timbres.reserve(MAX_TIMBRALITY);  // XXX should not be needed
        // m_voices.reserve(MAX_POLYPHONY);    // XXX
        m_timbres.emplace_back(false);
        m_voices.emplace_back(false);
    }

    const timbre_vector& timbres() const { return m_timbres; }
    timbre_vector& timbres() { return m_timbres; }

    const voice_vector& voices() const { return m_voices; }
    voice_vector& voices() { return m_voices; }

    Synth& add_timbre_control(Control& ctl)
    {
        assert(!m_finalized);
        m_timbres.front().add_control(&ctl);
        return *this;
    }

    Synth& add_timbre_module(Module& mod, bool is_output = false)
    {
        assert(!m_finalized);
        m_timbres.front().add_module(&mod);
        if (is_output)
            m_output_modules.push_back(&mod);
        return *this;
    }

    Synth& add_voice_control(Control& ctl)
    {
        assert(!m_finalized);
        m_voices.front().add_control(&ctl);
        return *this;
    }

    Synth& add_voice_module(Module& mod)
    {
        assert(!m_finalized);
        m_voices.front().add_module(&mod);
        return *this;
    }

    void finalize()
    {
        assert(!m_finalized);
        // N.B., the first timbre and voice are already allocated.
        for (size_t i = 1; i < timbrality; i++) {
            m_timbres.emplace_back(m_timbres.front());
        }

        for (size_t i = 1; i < polyphony; i++)
            m_voices.emplace_back(m_voices.front());
        m_finalized = true;
    }

    void apply_patch(Patch& patch, Timbre& timbre)
    {
        assert(m_finalized);
        auto& arch_timbre = m_timbres.front();
        auto& arch_voice = m_voices.front();
        timbre.set_patch(&patch);
        auto planner = Planner(arch_timbre.controls(),
                               arch_timbre.modules(),
                               arch_voice.controls(),
                               arch_voice.modules(),
                               patch.links(),
                               m_output_modules);
        auto plan = planner.make_plan();
        timbre.set_patch(&patch);
        timbre.plan(plan);
        Resolver resolver;
        resolver.add_controls(timbre.controls().begin(),
                              timbre.controls().end())
                .add_modules(timbre.modules().begin(),
                             timbre.modules().end())
                .finalize();

        // perform the prep steps.
        for (auto& step: plan.t_prep())
            step.prep(resolver);

        // make the pre-voice actions.
        render_action_sequence pre;
        for (auto& step: plan.pre_render())
            pre.push_back(step.make_action(resolver));
        timbre.pre_actions(pre);    // XXX should construct in place

        // make the post-voice actions.
        render_action_sequence post;
        for (auto& step: plan.post_render())
            post.push_back(step.make_action(resolver));
        timbre.post_actions(post);   // XXX should construct in place
    }

    void attach_voice_to_timbre(Timbre& timbre, Voice& voice)
    {
        assert(m_finalized);
        // populate the resolver.
        Resolver resolver;
        resolver.add_controls(timbre.controls().begin(),
                              timbre.controls().end())
                .add_modules(timbre.modules().begin(),
                             timbre.modules().end())
                .add_controls(voice.controls().begin(),
                              voice.controls().end())
                .add_modules(voice.modules().begin(),
                             voice.modules().end())
                .finalize();

        auto& plan = timbre.plan();

        voice.timbre(&timbre);

        // perform the prep steps.
        for (auto& step: plan.v_prep())
            step.prep(resolver);

        // create the voice action sequence.
        render_action_sequence actions;
        for (auto& step: plan.v_render())
            actions.push_back(step.make_action(resolver));
        voice.actions(actions);
    }

private:

    bool m_finalized;
    fixed_vector<Module *, MAX_OUTPUT_MODULES> m_output_modules;
    timbre_vector m_timbres;
    voice_vector m_voices;

    friend class synth_unit_test;

};

#endif /* !SYNTH_included */
