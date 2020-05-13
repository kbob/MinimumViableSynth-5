#include "soundscope.h"

#include <cxxtest/TestSuite.h>

class soundscope_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Soundscope();
    }

};
