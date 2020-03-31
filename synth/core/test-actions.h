#include "synth/core/actions.h"

#include <cxxtest/TestSuite.h>

class ActionsUnitTest : public CxxTest::TestSuite {

public:

    // XXX need tests that have the actions actually acting.

    void test_clear()
    {
        ClearAction a(2);
        TS_ASSERT(a.m_in_port_index == 2);
    }

    void test_alias()
    {
        AliasAction a(3, 4);
        TS_ASSERT(a.m_out_port_index == 3);
        TS_ASSERT(a.m_in_port_index == 4);
    }

    void test_prep()
    {
        PrepAction p0;
        TS_ASSERT(p0.m_tag == PrepActionType::NONE);

        PrepAction p1(ClearAction(2));
        TS_ASSERT(p1.m_tag == PrepActionType::CLEAR);
        TS_ASSERT(p1.m_u.clear.m_in_port_index == 2);

        PrepAction p2(AliasAction(3, 4));
        TS_ASSERT(p2.m_tag == PrepActionType::ALIAS);
        TS_ASSERT(p2.m_u.alias.m_out_port_index == 3);
        TS_ASSERT(p2.m_u.alias.m_in_port_index == 4);
    }

    void test_copy()
    {
        CopyAction a(5, 6, nullptr);
        TS_ASSERT(a.m_out_port_index == 5);
        TS_ASSERT(a.m_in_port_index == 6);
    }

    void test_add()
    {
        AddAction a(7, 6, nullptr);
        TS_ASSERT(a.m_out_port_index == 7);
        TS_ASSERT(a.m_in_port_index == 6);
    }

    void test_render()
    {
        RenderAction a(8);
        TS_ASSERT(a.m_mod_index == 8);
    }

    void test_run()
    {
        RunAction r0;
        TS_ASSERT(r0.m_tag == RunActionType::NONE);

        RunAction r1(CopyAction(5, 6, nullptr));
        TS_ASSERT(r1.m_tag == RunActionType::COPY);
        TS_ASSERT(r1.m_u.copy.m_out_port_index == 5);
        TS_ASSERT(r1.m_u.copy.m_in_port_index == 6);

        RunAction r2(AddAction(7, 6, nullptr));
        TS_ASSERT(r2.m_tag == RunActionType::ADD);
        TS_ASSERT(r2.m_u.add.m_out_port_index == 7);
        TS_ASSERT(r2.m_u.add.m_in_port_index == 6);

        RunAction r3(RenderAction(8));
        TS_ASSERT(r3.m_tag == RunActionType::RENDER);
        TS_ASSERT(r3.m_u.render.m_mod_index == 8);
    }

};
