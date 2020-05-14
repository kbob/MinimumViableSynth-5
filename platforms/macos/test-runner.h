#include "runner.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/synth.h"

class runner_unit_test : public CxxTest::TestSuite {

public:

    class FooTarget {
    public:
        FooTarget(const AudioConfig& ac, Module& out)
        : m_synth("FooSynth", 1, 1)
        {
            m_synth.add_timbre_module(out, true).finalize(ac);
            m_synth.apply_patch(m_patch, m_synth.timbres().front());
        }
        Synth& synth() { return m_synth; }
        Synth m_synth;
        Patch m_patch;
    };

    void test_instantiate()
    {
        (void)Runner<FooTarget>();
    }

    void test_run()
    {
        Runner<FooTarget>().run();
    }
};
