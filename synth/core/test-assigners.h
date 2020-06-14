#include "assigners.h"

#include <cxxtest/TestSuite.h>

class assigner_unit_test : public CxxTest::TestSuite {

public:

    class FooAssign : public Assigner {
    public:
        Voice *assign_idle_voice() override { return nullptr; }
        Voice *choose_voice_to_steal() override { return nullptr; }
    };

    void test_instantiate()
    {
        (void)FooAssign();
    }

};
