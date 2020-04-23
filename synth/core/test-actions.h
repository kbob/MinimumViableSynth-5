#include "actions.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/links.h"

class steps_unit_test : public CxxTest::TestSuite {

public:

    // XXX need tests that have the step generating actions.

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

    void test_copy()
    {
        Input<> dest;
        Output<> src, ctl;
        auto link = make_link(&dest, &src, &ctl, 0.3f);
        CopyStep s(7, 5, 6, &link);
        TS_ASSERT(s.m_dest_port_index == 7);
        TS_ASSERT(s.m_src_port_index == 5);
        TS_ASSERT(s.m_ctl_port_index == 6);
        TS_ASSERT(s.m_link == &link);
    }

    void test_add()
    {
        Input<> dest;
        Output<> src, ctl;
        auto link = make_link(&dest, &src, &ctl, 0.4f);
        AddStep s(7, 8, 9, &link);
        TS_ASSERT(s.m_dest_port_index == 7);
        TS_ASSERT(s.m_src_port_index == 8);
        TS_ASSERT(s.m_ctl_port_index == 9);
        TS_ASSERT(s.m_link == &link);

    }

    void test_module_render()
    {
        ModuleRenderStep s(10);
        TS_ASSERT(s.m_mod_index == 10);
    }

    void test_run()
    {
        RenderStep r0;
        TS_ASSERT(r0.m_tag == RenderStepTag::NONE);

        Input<> dest1;
        Output<> src1, ctl1;
        auto link1 = make_link(&dest1, &src1, &ctl1, 0.3f);
        RenderStep r1(CopyStep(7, 5, 6, &link1));
        TS_ASSERT(r1.m_tag == RenderStepTag::COPY);
        TS_ASSERT(r1.m_u.copy.m_dest_port_index == 7);
        TS_ASSERT(r1.m_u.copy.m_src_port_index == 5);
        TS_ASSERT(r1.m_u.copy.m_ctl_port_index == 6);
        TS_ASSERT(r1.m_u.copy.m_link == &link1);

        Input<> dest2;
        Output<> src2, ctl2;
        auto link2 = make_link(&dest2, &src2, &ctl2, 0.4f);
        RenderStep r2(AddStep(7, 8, 9, &link2));
        TS_ASSERT(r2.m_tag == RenderStepTag::ADD);
        TS_ASSERT(r2.m_u.add.m_dest_port_index == 7);
        TS_ASSERT(r2.m_u.add.m_src_port_index == 8);
        TS_ASSERT(r2.m_u.add.m_ctl_port_index == 9);
        TS_ASSERT(r2.m_u.add.m_link == &link2);

        RenderStep r3(ModuleRenderStep(10));
        TS_ASSERT(r3.m_tag == RenderStepTag::MODULE_RENDER);
        TS_ASSERT(r3.m_u.mrend.m_mod_index == 10);
    }

};
