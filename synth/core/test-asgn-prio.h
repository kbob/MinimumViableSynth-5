#include "asgn-prio.h"

#include <cxxtest/TestSuite.h>

class priority_assigner_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        Synth s("TestSynth", 1, 1);
        PriorityAssigner::prioritizer f = [] (const Voice&) -> int
        {
            return 0;
        };
        (void)PriorityAssigner(s, f);
    }

    // allocate too many voices.
    void test_allocate_all()
    {
        Synth s("TestSynth", 2, 1);
        Config cfg;
        cfg.set_sample_rate(44100);
        s.finalize(cfg);
        PriorityAssigner::prioritizer f = [&] (const Voice& v) -> int
        {
            return &v - s.voices().data();
        };
        PriorityAssigner assign(s, f);

        Voice *v0 = assign.assign_idle_voice();
        TS_ASSERT_EQUALS(v0, &s.voices()[0]);
        v0->start_note();

        Voice *v1 = assign.assign_idle_voice();
        TS_ASSERT_EQUALS(v1, &s.voices()[1]);
        v1->start_note();

        Voice *v2 = assign.assign_idle_voice();
        TS_ASSERT_EQUALS(v2, nullptr);
        Voice *v2s = assign.choose_voice_to_steal();
        TS_ASSERT_EQUALS(v2s, v0);
        v2s->kill_note();
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::SOUNDING);

        Voice *v3 = assign.assign_idle_voice();
        TS_ASSERT_EQUALS(v3, nullptr);
        Voice *v3s = assign.choose_voice_to_steal();
        TS_ASSERT_EQUALS(v3s, v1);
        v3s->kill_note();
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);

        Voice *v4 = assign.assign_idle_voice();
        TS_ASSERT_EQUALS(v4, nullptr);
        Voice *v4s = assign.choose_voice_to_steal();
        TS_ASSERT_EQUALS(v4s, nullptr);
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);
    }

};
