#include "plan.h"

#include <string>

#include <cxxtest/TestSuite.h>

class plan_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Plan();
        TS_TRACE("sizeof (Plan) = " + std::to_string(sizeof (Plan)));
        TS_TRACE("MAX_PREP_STEPS = "+ std::to_string(MAX_PREP_STEPS));
        TS_TRACE("MAX_RENDER_STEPS = " + std::to_string(MAX_RENDER_STEPS));
    }

    void test_tprep()
    {
        ClearStep c0{0, 0};
        Plan p;
        p.t_prep().push_back(c0);
        const Plan& cp{p};
        TS_ASSERT_EQUALS(cp.t_prep().size(), 1);
        TS_ASSERT_EQUALS(cp.t_prep().at(0).tag(), PrepStep::Tag::CLEAR);
    }

    void test_vprep()
    {
        ClearStep c0{0, 0};
        Plan p;
        p.v_prep().push_back(c0);
        const Plan& cp{p};
        TS_ASSERT_EQUALS(cp.v_prep().size(), 1);
        TS_ASSERT_EQUALS(cp.v_prep().at(0).tag(), PrepStep::Tag::CLEAR);
    }

    void test_pre_render()
    {
        ControlRenderStep crs(0);
        Plan p;
        p.pre_render().push_back(crs);
        const Plan& cp{p};
        TS_ASSERT_EQUALS(cp.pre_render().size(), 1);
        TS_ASSERT_EQUALS(cp.pre_render().at(0).tag(),
                         RenderStep::Tag::CONTROL_RENDER);
    }

    void test_v_render()
    {
        ControlRenderStep crs(0);
        Plan p;
        p.v_render().push_back(crs);
        const Plan& cp{p};
        TS_ASSERT_EQUALS(cp.v_render().size(), 1);
        TS_ASSERT_EQUALS(cp.v_render().at(0).tag(),
                         RenderStep::Tag::CONTROL_RENDER);
    }

    void test_post_render()
    {
        ControlRenderStep crs(0);
        Plan p;
        p.post_render().push_back(crs);
        const Plan& cp{p};
        TS_ASSERT_EQUALS(cp.post_render().size(), 1);
        TS_ASSERT_EQUALS(cp.post_render().at(0).tag(),
                         RenderStep::Tag::CONTROL_RENDER);
    }

};
