#include "layering.h"

#include <cxxtest/TestSuite.h>

class layering_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        size_t TIMB = 2;
        (void)midi::Layering(TIMB);
    }

};
