#include "facade.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/assigners.h"
#include "synth/core/synth.h"

using midi::Facade;

class facade_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::Facade(1, 1);
        TS_TRACE("sizeof Facade = " + std::to_string(sizeof (midi::Facade)));
    }

    class FooAssigner : public Assigner {
    public:
        Voice *assign_idle_voice() override { return nullptr; }
        Voice *choose_voice_to_steal() override { return nullptr; }
    };

    void test_finalize()
    {
        size_t POLY = 1, TIMB = 1;
        Synth s("Foo", POLY, TIMB);
        FooAssigner a;

        Facade f(POLY, TIMB);
        f.attach(s)
         .attach(a)
         .finalize();
    }

};
