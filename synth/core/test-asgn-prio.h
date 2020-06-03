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
        AudioConfig ac;
        s.finalize(ac);
        PriorityAssigner::prioritizer f = [&] (const Voice& v) -> int
        {
            return &v - s.voices().data();
        };
        PriorityAssigner assign(s, f);

        Voice *v0 = assign.allocate_voice();
        TS_ASSERT_EQUALS(v0, &s.voices()[0]);
        v0->start_note();

        Voice *v1 = assign.allocate_voice();
        TS_ASSERT_EQUALS(v1, &s.voices()[1]);
        v1->start_note();

        Voice *v2 = assign.allocate_voice();
        TS_ASSERT_EQUALS(v2, nullptr);
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::SOUNDING);

        Voice *v3 = assign.allocate_voice();
        TS_ASSERT_EQUALS(v3, nullptr);
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);

        Voice *v4 = assign.allocate_voice();
        TS_ASSERT_EQUALS(v4, nullptr);
        TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
        TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);
    }

};
