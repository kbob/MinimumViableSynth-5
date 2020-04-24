#include "plan.h"

#include <cxxtest/TestSuite.h>

class PlanUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Plan();
    }

    void test_prep()
    {
        ClearStep c0{0, 0};
        Plan p;
        p.t_prep().push_back(c0);
        const Plan& cp{p};
        TS_ASSERT(cp.t_prep().size() == 1);
        TS_ASSERT(cp.t_prep().at(0).tag() == PrepStepTag::CLEAR);
    }

};
