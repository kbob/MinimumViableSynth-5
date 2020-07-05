#ifndef SYNTH_included
#define SYNTH_included

#include <cassert>

#include "synth/core/action.h"
#include "synth/core/assigners.h"
#include "synth/core/sizes.h"
#include "synth/core/patch.h"
#include "synth/core/planner.h"
#include "synth/core/resolver.h"
#include "synth/core/sizes.h"
#include "synth/core/summer.h"
#include "synth/core/timbre.h"
#include "synth/core/voice.h"
#include "synth/util/fixed-vector.h"

class Config;
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
//     apply a patch to a timbre
//     allocate a voice
//     attach a voice to a timbre
//     detach a voice from a timbre

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
      m_finalized{false},
      m_assigner{nullptr}
    {
        assert(0 < polyphony && polyphony <= MAX_POLYPHONY);
        assert(0 < timbrality && timbrality <= MAX_TIMBRALITY);
        m_timbres.emplace_back(false);
        m_voices.emplace_back(false);
    }

    const timbre_vector& timbres() const { return m_timbres; }
    timbre_vector& timbres() { return m_timbres; }

    const voice_vector& voices() const { return m_voices; }
    voice_vector& voices() { return m_voices; }

    const Assigner *assigner() const { return m_assigner; }
    void assigner(Assigner *a) { m_assigner = a; }

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

    Synth& add_voice_control(Control& ctl, bool is_lifetime_moderator = false)
    {
        assert(!m_finalized);
        m_voices.front().add_control(&ctl, is_lifetime_moderator);
        return *this;
    }

    Synth& add_voice_module(Module& mod, bool is_lifetime_moderator = false)
    {
        assert(!m_finalized);
        m_voices.front().add_module(&mod, is_lifetime_moderator);
        return *this;
    }

    template <class ElementType = DEFAULT_SAMPLE_TYPE>
    Synth& add_summer(Summer<ElementType>& sum)
    {
        assert(!m_finalized);
        add_timbre_module(sum.timbre_side);
        add_voice_module(sum.voice_side);
        return *this;
    }

    void finalize(const Config& cfg)
    {
        assert(!m_finalized);
        // N.B., the first timbre and voice are already allocated.
        for (size_t i = 1; i < timbrality; i++) {
            m_timbres.emplace_back(m_timbres.front());
        }

        for (size_t i = 1; i < polyphony; i++)
            m_voices.emplace_back(m_voices.front());
        m_finalized = true;

        // Walk the configurer through the subobjects.
        cfg.pre_configure(*this);
        for (auto& timbre: m_timbres) {
            cfg.pre_configure(timbre);
            timbre.configure(cfg);
            cfg.post_configure(timbre);
        }
        for (auto& voice: m_voices) {
            cfg.pre_configure(voice);
            voice.configure(cfg);
            cfg.post_configure(voice);
        }
        cfg.post_configure(*this);
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
        timbre.add_voice(&voice - m_voices.data());

        // perform the prep steps.
        for (auto& step: plan.v_prep())
            step.prep(resolver);

        // create the voice action sequence.
        render_action_sequence actions;
        for (auto& step: plan.v_render())
            actions.push_back(step.make_action(resolver));
        voice.actions(actions);
    }

    void detach_voice_from_timbre(Timbre& timbre, Voice& voice)
    {
        voice.timbre(nullptr);
        timbre.remove_voice(&voice - m_voices.data());
    }

private:

    bool m_finalized;
    fixed_vector<Module *, MAX_OUTPUT_MODULES> m_output_modules;
    timbre_vector m_timbres;
    voice_vector m_voices;
    Assigner *m_assigner;

    friend class synth_unit_test;

};

#endif /* !SYNTH_included */
