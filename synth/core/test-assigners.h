#include "assigners.h"

#include <cxxtest/TestSuite.h>

class assigner_unit_test : public CxxTest::TestSuite {

public:

    class FooAssign : public Assigner {
    public:
        Voice *allocate_voice() override { return nullptr; }
    };

    void test_instantiate()
    {
        (void)FooAssign();
    }

    // allocate too many voices.
    // void test_allocate_all()
    // {
    //     Synth s("TestSynth", 2, 1);
    //     Config cfg;
    //     cfg.set_sample_rate(44100);
    //     s.finalize(cfg);
    //     PriorityAllocator::prioritizer f = [&] (const Voice& v) -> int
    //     {
    //         return &v - s.voices().data();
    //     };
    //     PriorityAllocator alloc(s, f);
    //
    //     Voice *v0 = alloc.allocate_voice();
    //     TS_ASSERT_EQUALS(v0, &s.voices()[0]);
    //     v0->start_note();
    //
    //     Voice *v1 = alloc.allocate_voice();
    //     TS_ASSERT_EQUALS(v1, &s.voices()[1]);
    //     v1->start_note();
    //
    //     Voice *v2 = alloc.allocate_voice();
    //     TS_ASSERT_EQUALS(v2, nullptr);
    //     TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
    //     TS_ASSERT_EQUALS(v1->state(), Voice::State::SOUNDING);
    //
    //     Voice *v3 = alloc.allocate_voice();
    //     TS_ASSERT_EQUALS(v3, nullptr);
    //     TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
    //     TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);
    //
    //     Voice *v4 = alloc.allocate_voice();
    //     TS_ASSERT_EQUALS(v4, nullptr);
    //     TS_ASSERT_EQUALS(v0->state(), Voice::State::STOPPING);
    //     TS_ASSERT_EQUALS(v1->state(), Voice::State::STOPPING);
    // }

};
