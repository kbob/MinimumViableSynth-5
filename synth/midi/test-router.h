#include "router.h"

#include <cxxtest/TestSuite.h>

class router_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::Router();
    }

};
