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
        AudioConfig ac(SR_44100, sample_format::F32, channel_config::MONO);
        OutModule out;
        (void)SimpleBeep(ac, out);
    }

};
