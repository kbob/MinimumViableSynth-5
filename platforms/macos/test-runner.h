#include "runner.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/synth.h"

class runner_unit_test : public CxxTest::TestSuite {

public:

    class FooRunnable {
    public:
        FooRunnable(const AudioConfig&, Module& out)
        : m_synth("FooSynth", 1, 1)
        {
            m_synth.add_timbre_module(out, true).finalize();
            m_synth.apply_patch(m_patch, m_synth.timbres().front());
        }
        Synth& synth() { return m_synth; }
        Synth m_synth;
        Patch m_patch;
    };

    void test_instantiate()
    {
        (void)Runner<FooRunnable>();
    }

    void test_run()
    {
        Runner<FooRunnable>().run();
    }
};
