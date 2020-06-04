#include "simple-beep.h"

#include <cxxtest/TestSuite.h>

class simple_beep_unit_test : public CxxTest::TestSuite {

public:

    class OutModule : public ModuleType<OutModule> {
    public:
        OutModule() { in.name("in"); ports(in); }
        Input<> in;
        void render(size_t) {}
    };

    void test_instantiate()
    {
        Config cfg;
        cfg.set_sample_rate(44100);
        OutModule out;
        (void)SimpleBeep(cfg, out);
    }

};
