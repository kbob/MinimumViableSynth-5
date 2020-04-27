#include "steps.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/links.h"

class steps_unit_test : public CxxTest::TestSuite {

public:

    // XXX need tests that have the steps generating actions.

    void test_clear()
    {
        ClearStep s(2, 0.1f);
        TS_ASSERT(s.m_dest_port_index == 2);
        TS_ASSERT(s.m_scale == 0.1f);
    }

    void test_alias()
    {
        AliasStep s(4, 3);
        TS_ASSERT(s.m_dest_port_index == 4);
        TS_ASSERT(s.m_src_port_index == 3);
    }

    void test_prep()
    {
        PrepStep p0;
        TS_ASSERT(p0.m_tag == PrepStepTag::NONE);

        PrepStep p1(ClearStep(2, 0.1f));
        TS_ASSERT(p1.m_tag == PrepStepTag::CLEAR);
        TS_ASSERT(p1.m_u.clear.m_dest_port_index == 2);

        PrepStep p2(AliasStep(4, 3));
        TS_ASSERT(p2.m_tag == PrepStepTag::ALIAS);
        TS_ASSERT(p2.m_u.alias.m_dest_port_index == 4);
        TS_ASSERT(p2.m_u.alias.m_src_port_index == 3);
    }

    void test_control_render()
    {
        ControlRenderStep s(5);
        TS_ASSERT(s.m_ctl_index == 5);
    }

    void test_module_render()
    {
        ModuleRenderStep s(6);
        TS_ASSERT(s.m_mod_index == 6);
    }

    void test_copy()
    {
        Input<> dest;
        Output<> src, ctl;
        Link link{&dest, &src, &ctl, 0.2f};
        CopyStep s(9, 7, 8, &link);
        TS_ASSERT(s.m_dest_port_index == 9);
        TS_ASSERT(s.m_src_port_index == 7);
        TS_ASSERT(s.m_ctl_port_index == 8);
        TS_ASSERT(s.m_link == &link);
    }

    void test_add()
    {
        Input<> dest;
        Output<> src, ctl;
        Link link{&dest, &src, &ctl, 0.3f};
        AddStep s(9, 10, 11, &link);
        TS_ASSERT(s.m_dest_port_index == 9);
        TS_ASSERT(s.m_src_port_index == 10);
        TS_ASSERT(s.m_ctl_port_index == 11);
        TS_ASSERT(s.m_link == &link);
    }

    void test_run()
    {
        RenderStep r0;
        TS_ASSERT(r0.m_tag == RenderStepTag::NONE);

        RenderStep r1(ControlRenderStep(5));
        TS_ASSERT(r1.m_tag == RenderStepTag::CONTROL_RENDER);
        TS_ASSERT(r1.m_u.mrend.m_mod_index == 5);

        RenderStep r2(ModuleRenderStep(6));
        TS_ASSERT(r2.m_tag == RenderStepTag::MODULE_RENDER);
        TS_ASSERT(r2.m_u.mrend.m_mod_index == 6);

        Input<> dest3;
        Output<> src3, ctl3;
        Link link3{&dest3, &src3, &ctl3, 0.2f};
        RenderStep r3(CopyStep(9, 7, 8, &link3));
        TS_ASSERT(r3.m_tag == RenderStepTag::COPY);
        TS_ASSERT(r3.m_u.copy.m_dest_port_index == 9);
        TS_ASSERT(r3.m_u.copy.m_src_port_index == 7);
        TS_ASSERT(r3.m_u.copy.m_ctl_port_index == 8);
        TS_ASSERT(r3.m_u.copy.m_link == &link3);

        Input<> dest4;
        Output<> src4, ctl4;
        Link link4{&dest4, &src4, &ctl4, 0.3f};
        RenderStep r4(AddStep(9, 10, 11, &link4));
        TS_ASSERT(r4.m_tag == RenderStepTag::ADD);
        TS_ASSERT(r4.m_u.add.m_dest_port_index == 9);
        TS_ASSERT(r4.m_u.add.m_src_port_index == 10);
        TS_ASSERT(r4.m_u.add.m_ctl_port_index == 11);
        TS_ASSERT(r4.m_u.add.m_link == &link4);
    }

};
