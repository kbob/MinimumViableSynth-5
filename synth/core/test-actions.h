#include "synth/core/actions.h"

#include <cxxtest/TestSuite.h>

class ActionsUnitTest : public CxxTest::TestSuite {

public:

    void test_clear()
    {
        (void)ClearAction(2);
    }

};
