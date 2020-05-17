#include "synth.h"

#include <sstream>
#include <string>

#include <cxxtest/TestSuite.h>

#include "synth/core/audio-config.h"
#include "synth/core/plan.h"
#include "synth/core/ports.h"

class synth_unit_test : public CxxTest::TestSuite {

public:

    static const size_t POLY = 3;
    static const size_t TIMB = 3;

    static std::ostringstream& log()
    {
        static std::ostringstream ss;
        return ss;
    }

    class FooControl : public ControlType<FooControl> {
    public:
        const char *sig = "nosig";
        void render(size_t frame_count)
        {
            log() << sig << '.' << frame_count << ' ';
        }
    };

    class FooModule : public ModuleType<FooModule> {
    public:
        FooModule()
        : m_twin{nullptr}
        {
            in.name("in");
            out.name("out");
            ports(in, out);
        }
        Input<> in;
        Output<> out;
        void render(size_t frame_count)
        {
            log() << name() << '.' << frame_count << ' ';
        }
        Module *twin() const override { return m_twin; }
        Module *m_twin;
    };

    void test_instantiate()
    {
        (void)Synth{"Foo", POLY, TIMB};
        TS_TRACE("sizeof (Synth) = " + std::to_string(sizeof (Synth)));
    }

    FooControl tc0, tc1, vc0, vc1;
    FooModule tm0, tm1, vm0, vm1;
    AudioConfig ac;

    synth_unit_test()
    {
        tc0.sig = "tc0";
        tc1.sig = "tc1";
        vc0.sig = "vc0";
        vc1.sig = "vc1";
        tm0.name("tm0");
        tm1.name("tm1");
        vm0.name("vm0");
        vm1.name("vm1");
        tm1.m_twin = &vm0;
    }

    void test_properties()
    {
        Synth s{"Foo", POLY, TIMB};
        TS_ASSERT_EQUALS(s.name, std::string("Foo"));
        TS_ASSERT_EQUALS(s.polyphony, POLY);
        TS_ASSERT_EQUALS(s.timbrality, TIMB);
    }

    void test_construction()
    {
        Synth s{"Foo", POLY, TIMB};
        s.add_timbre_control(tc0)
         .add_timbre_control(tc1)
         .add_voice_control(vc0)
         .add_voice_control(vc1)
         .add_timbre_module(tm0)
         .add_timbre_module(tm1, true)
         .add_voice_module(vm0)
         .add_voice_module(vm1)
         .finalize(ac);

        const Timbre& t = s.m_timbres.front();
        TS_ASSERT_EQUALS(t.controls().size(), 2);
        TS_ASSERT_EQUALS(t.controls().at(0), &tc0);
        TS_ASSERT_EQUALS(t.controls().at(1), &tc1);
        TS_ASSERT_EQUALS(t.modules().at(0), &tm0);
        TS_ASSERT_EQUALS(t.modules().at(1), &tm1);

        const Voice& v = s.m_voices.front();
        TS_ASSERT_EQUALS(v.controls().size(), 2);
        TS_ASSERT_EQUALS(v.controls().at(0), &vc0);
        TS_ASSERT_EQUALS(v.controls().at(1), &vc1);
        TS_ASSERT_EQUALS(v.modules().at(0), &vm0);
        TS_ASSERT_EQUALS(v.modules().at(1), &vm1);
    }

    void test_add_summer()
    {
        Summer<> sum0;
        Synth s{"Foo", POLY, TIMB};
        s.add_summer(sum0)
         .finalize(ac);

        TS_ASSERT_EQUALS(s.timbres().front().modules()[0], &sum0.timbre_side);
        TS_ASSERT_EQUALS(s.voices().front().modules()[0], &sum0.voice_side);
    }

    void test_apply_patch()
    {
        Synth s{"Foo", POLY, TIMB};
        s.add_timbre_module(tm0)
         .add_timbre_module(tm1, true)
         .add_voice_module(vm0)
         .finalize(ac);
        Patch p;
        p.connect(vm0.in, tm0.out);
        Timbre& t = s.timbres().front();
        s.apply_patch(p, t);
        auto plan = t.plan();
        // Ports:
        //    tm0.in  tm0.out tm1.in  tm1.out
        //    vm0.in  vm0.out
        TS_ASSERT_EQUALS(prep_step_rep(plan.t_prep()),
                         "[clear(0, 0) clear(2, 0)]");
        TS_ASSERT_EQUALS(prep_step_rep(plan.v_prep()),
                         "[alias(4, 1)]");
        TS_ASSERT_EQUALS(render_step_rep(plan.pre_render()),
                         "[mrend(0)]");
        TS_ASSERT_EQUALS(render_step_rep(plan.v_render()),
                         "[mrend(2)]");
        TS_ASSERT_EQUALS(render_step_rep(plan.post_render()),
                         "[mrend(1)]");

        log().str("");
        t.pre_render(2);
        TS_ASSERT_EQUALS(log().str(), "tm0.2 ");

        log().str("");
        t.post_render(3);
        TS_ASSERT_EQUALS(log().str(), "tm1.3 ");
    }

    void test_attach_voice()
    {
        Synth s{"Foo", POLY, TIMB};
        s.add_timbre_module(tm0)
         .add_timbre_module(tm1, true)
         .add_voice_module(vm0)
         .finalize(ac);
        Patch p;
        p.connect(vm0.in, tm0.out);
        Timbre& t = s.timbres().front();
        s.apply_patch(p, t);
        Voice& v = s.voices().at(0);
        s.attach_voice_to_timbre(t, v);
        v.start_note();

        TS_ASSERT_EQUALS(v.timbre(), &t);

        log().str("");
        v.render(4);
        TS_ASSERT_EQUALS(log().str(), "vm0.4 ");

        s.detach_voice_from_timbre(t, v);
        TS_ASSERT_EQUALS(v.timbre(), nullptr);
    }

    void test_attach_two_voices()
    {
        Synth s{"Foo", POLY, TIMB};
        s.add_timbre_module(tm0)
         .add_timbre_module(tm1, true)
         .add_voice_module(vm0)
         .finalize(ac);
        Patch p;
        p.connect(vm0.in, tm0.out);
        Timbre& t = s.timbres().front();
        s.apply_patch(p, t);
        Voice& v0 = s.voices().at(0);
        Voice& v2 = s.voices().at(2);
        s.attach_voice_to_timbre(t, v0);
        s.attach_voice_to_timbre(t, v2);

        TS_ASSERT_EQUALS(v0.timbre(), &t);
        TS_ASSERT_EQUALS(v2.timbre(), &t);
        TS_ASSERT_EQUALS(t.attached_voices(), 0b101);

        s.detach_voice_from_timbre(t, v0);
        s.detach_voice_from_timbre(t, v2);
        TS_ASSERT_EQUALS(v0.timbre(), nullptr);
        TS_ASSERT_EQUALS(v2.timbre(), nullptr);
        TS_ASSERT_EQUALS(t.attached_voices(), 0b000);
    }

    std::string
    prep_step_rep(const Plan::prep_step_sequence& seq)
    {
        std::ostringstream ss;
        ss << seq;
        return ss.str();
    }

    std::string
    render_step_rep(const Plan::render_step_sequence& seq)
    {
        std::ostringstream ss;
        ss << seq;
        return ss.str();
    }

};
