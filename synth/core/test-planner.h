#include "planner.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cxxtest/TestSuite.h>

#include "synth/core/steps.h"

class planner_unit_test : public CxxTest::TestSuite {

public:

    class FooControl : public ControlType<FooControl> {
    public:
        void render(size_t) {}
    };

    class FooModule : public ModuleType<FooModule> {
    public:
        FooModule()
        {
            in.name("in");
            out.name("out");
            ports(in, out);
        }
        Input<> in;
        Output<> out;
        void render(size_t) {}
    };

    void test_instantiate()
    {
        Planner::tc_vec tc;
        Planner::tm_vec tm;
        Planner::vc_vec vc;
        Planner::vm_vec vm;
        Planner::link_vec links;
        Planner::om_vec om;
        (void)Planner(tc, tm, vc, vm, links, om);
        TS_TRACE("sizeof (Planner) = " + std::to_string(sizeof (Planner)));
    }

    FooControl tc0, tc1, tc2, tc3, vc0, vc1;
    FooModule tm0, tm1, tm2, vm0, vm1;
    Planner::tc_vec tc{&tc0, &tc1, &tc2, &tc3};
    Planner::vc_vec vc{&vc0, &vc1};
    Planner::tm_vec tm{&tm0, &tm1, &tm2};
    Planner::vm_vec vm{&vm0, &vm1};
    // Ports:
    // [
    //   tc0.out, tc1.out, tc2.out, tc3.out,
    //   tm0.in,  tm0.out, tm1.in,  tm1.in,
    //   tm2.in,  tm2.out, vc0.out, vc1.out,
    //   vm0.in,  vm0.out, vc1.in,  vm1.out
    // ]

    planner_unit_test()
    {
        tm0.name("tm0");
        tm1.name("tm1");
        tm2.name("tm2");
        vm0.name("vm0");
        vm1.name("vm1");
    }

    std::string
    prep_step_rep(const Plan::prep_step_sequence& seq)
    {
        std::ostringstream ss;
        ss << "[";
        std::ostream_iterator<PrepStep> joiner(ss, " ");
        std::copy(seq.begin(), seq.end(), joiner);
        auto s = ss.str();
        if (seq.begin() != seq.end())
            s = s.substr(0, s.size() - 1);
        return s + "]";
    }

    std::string
    render_rep(const Plan::render_step_sequence& seq)
    {
        std::ostringstream ss;
        ss << "[";
        std::ostream_iterator<RenderStep> joiner(ss, " ");
        std::copy(seq.begin(), seq.end(), joiner);
        auto s = ss.str();
        if (seq.begin() != seq.end())
            s = s.substr(0, s.size() - 1);
        return s + "]";
    }

    void test_reachability()
    {
        // Construct graph:
        //     (tc0 -> tm0) -> (vc0, tc1 -> vm0) -> (tc2 -> tm1)
        // tc3, vc1, tm2, and vm1 are disconnected.

        Planner::link_vec links;
        links.emplace_back(&tm0.in, nullptr, &tc0.out);
        links.emplace_back(&vm0.in, &tm0.out, &vc0.out);
        links.emplace_back(&vm0.in, nullptr, &tc1.out);
        links.emplace_back(&tm1.in, &vm0.out, &tc2.out);
        Planner::om_vec om{&tm1};
        Planner planner{tc, tm, vc, vm, links, om};
        Plan plan = planner.make_plan();
        TS_ASSERT_EQUALS(prep_step_rep(plan.t_prep()),
                         "[alias(4, 0) alias(6, -1)]");
        TS_ASSERT_EQUALS(prep_step_rep(plan.v_prep()),
                         "[alias(12, -1)]");
        TS_ASSERT_EQUALS(render_rep(plan.pre_render()),
                         "[crend(0) crend(1) crend(2) mrend(0)]");
        // TS_ASSERT_EQUALS(render_rep(plan.v_render()),
        //           "[crend(4) copy(12, 5, 10) add(12, -1, 1) mrend(3)]")
        // TS_ASSERT_EQUALS(render_rep(plan.post_render()),
        //                  "[copy(6, 13, 2) mrend(1)]");
        TS_ASSERT_EQUALS(render_rep(plan.v_render()),
                         "[crend(4) mrend(3)]")
        TS_ASSERT_EQUALS(render_rep(plan.post_render()),
                         "[mrend(1)]");
    }

    void test_simple_ctl_link()
    {
        // Construct graph:
        //    tc0 -> tm0

        Planner::link_vec links;
        links.emplace_back(&tm0.in, nullptr, &tc0.out);
        Planner::om_vec om{&tm0};
        Planner planner{tc, tm, vc, vm, links, om};
        auto plan = planner.make_plan();;
        TS_ASSERT_EQUALS(prep_step_rep(plan.t_prep()),
                         "[alias(4, 0)]");
        TS_ASSERT_EQUALS(prep_step_rep(plan.v_prep()),
                         "[]");
        TS_ASSERT_EQUALS(render_rep(plan.pre_render()),
                         "[crend(0)]");
        TS_ASSERT_EQUALS(render_rep(plan.v_render()),
                         "[]")
        TS_ASSERT_EQUALS(render_rep(plan.post_render()),
                         "[mrend(0)]");
    }

    void test_simple_mod_link()
    {
        // Construct graph:
        //     tm0 -> tm1

        Planner::link_vec links;
        links.emplace_back(&tm1.in, &tm0.out, nullptr);
        Planner::om_vec om{&tm1};
        Planner planner{tc, tm, vc, vm, links, om};
        auto plan = planner.make_plan();;
        TS_ASSERT_EQUALS(prep_step_rep(plan.t_prep()),
                         "[clear(4, 0) alias(6, 5)]");
        TS_ASSERT_EQUALS(prep_step_rep(plan.v_prep()),
                         "[]");
        TS_ASSERT_EQUALS(render_rep(plan.pre_render()),
                         "[]");
        TS_ASSERT_EQUALS(render_rep(plan.v_render()),
                         "[]")
        TS_ASSERT_EQUALS(render_rep(plan.post_render()),
                         "[mrend(0) mrend(1)]");
    }

    void test_v_to_post_simple_link()
    {
        // Construct graph:
        //     vm0 -> tm0

        Planner::link_vec links;
        links.emplace_back(&tm0.in, &vm0.out, nullptr);
        Planner::om_vec om{&tm0};
        Planner planner{tc, tm, vc, vm, links, om};
        auto plan = planner.make_plan();;
        TS_ASSERT_EQUALS(prep_step_rep(plan.t_prep()),
                         "[alias(4, -1)]");
        TS_ASSERT_EQUALS(prep_step_rep(plan.v_prep()),
                         "[clear(12, 0)]");
        TS_ASSERT_EQUALS(render_rep(plan.pre_render()),
                         "[]");
        TS_ASSERT_EQUALS(render_rep(plan.v_render()),
                         "[mrend(3)]")
        // TS_ASSERT_EQUALS(render_rep(plan.post_render()),
        //                  "[copy(4, 13, -1) mrend(0)]");
        TS_ASSERT_EQUALS(render_rep(plan.post_render()),
                         "[mrend(0)]");
    }

    void test_timbre_cycle()
    {
        // Construct graph:
        //     tm0 -> tm1 -> tm0

        Planner::link_vec links;
        links.emplace_back(&tm1.in, &tm0.out, nullptr);
        links.emplace_back(&tm0.in, &tm1.out, nullptr);
        Planner::om_vec om{&tm1};
        Planner planner{tc, tm, vc, vm, links, om};
        TS_ASSERT_THROWS(planner.make_plan(), std::runtime_error);
    }

    void test_voice_cycle()
    {
        // Construct graph:
        //     vm1 -> vm0 -> vm1 -> tm0

        Planner::link_vec links;
        links.emplace_back(&vm1.in, &vm0.out, nullptr);
        links.emplace_back(&vm0.in, &vm1.out, nullptr);
        links.emplace_back(&tm0.in, &vm1.out, nullptr);
        Planner::om_vec om{&tm0};
        Planner planner{tc, tm, vc, vm, links, om};
        TS_ASSERT_THROWS(planner.make_plan(), std::runtime_error);
    }

};
