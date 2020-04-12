#include "actions.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/links.h"

class ActionsUnitTest : public CxxTest::TestSuite {

public:

    // XXX need tests that have the actions actually acting.

    void test_clear()
    {
        ClearAction a(2, 0.1f);
        TS_ASSERT(a.m_dest_port_index == 2);
        TS_ASSERT(a.m_scale == 0.1f);
    }

    void test_alias()
    {
        AliasAction a(4, 3);
        TS_ASSERT(a.m_dest_port_index == 4);
        TS_ASSERT(a.m_src_port_index == 3);
    }

    void test_prep()
    {
        PrepAction p0;
        TS_ASSERT(p0.m_tag == PrepActionType::NONE);

        PrepAction p1(ClearAction(2, 0.2f));
        TS_ASSERT(p1.m_tag == PrepActionType::CLEAR);
        TS_ASSERT(p1.m_u.clear.m_dest_port_index == 2);

        PrepAction p2(AliasAction(4, 3));
        TS_ASSERT(p2.m_tag == PrepActionType::ALIAS);
        TS_ASSERT(p2.m_u.alias.m_dest_port_index == 4);
        TS_ASSERT(p2.m_u.alias.m_src_port_index == 3);
    }

    void test_copy()
    {
        CopyAction a(7, 5, 6, 0.3f);
        TS_ASSERT(a.m_dest_port_index == 7);
        TS_ASSERT(a.m_src_port_index == 5);
        TS_ASSERT(a.m_ctl_port_index == 6);
        TS_ASSERT(a.m_scale == 0.3f);
    }

    void test_add()
    {
        AddAction a(7, 8, 9, 0.4f);
        TS_ASSERT(a.m_dest_port_index == 7);
        TS_ASSERT(a.m_src_port_index == 8);
        TS_ASSERT(a.m_ctl_port_index == 9);
        TS_ASSERT(a.m_scale == 0.4f);

    }

    void test_render()
    {
        RenderAction a(10);
        TS_ASSERT(a.m_mod_index == 10);
    }

    void test_run()
    {
        RunAction r0;
        TS_ASSERT(r0.m_tag == RunActionType::NONE);

        RunAction r1(CopyAction(7, 5, 6, 0.3f));
        TS_ASSERT(r1.m_tag == RunActionType::COPY);
        TS_ASSERT(r1.m_u.copy.m_dest_port_index == 7);
        TS_ASSERT(r1.m_u.copy.m_src_port_index == 5);
        TS_ASSERT(r1.m_u.copy.m_ctl_port_index == 6);
        TS_ASSERT(r1.m_u.copy.m_scale == 0.3f);

        RunAction r2(AddAction(7, 8, 9, 0.4f));
        TS_ASSERT(r2.m_tag == RunActionType::ADD);
        TS_ASSERT(r2.m_u.add.m_dest_port_index == 7);
        TS_ASSERT(r2.m_u.add.m_src_port_index == 8);
        TS_ASSERT(r2.m_u.add.m_ctl_port_index == 9);
        TS_ASSERT(r2.m_u.add.m_scale == 0.4f);

        RunAction r3(RenderAction(10));
        TS_ASSERT(r3.m_tag == RunActionType::RENDER);
        TS_ASSERT(r3.m_u.render.m_mod_index == 10);
    }

};
