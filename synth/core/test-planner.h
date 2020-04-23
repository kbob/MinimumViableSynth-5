#include "planner.h"

#include <cxxtest/TestSuite.h>

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

class planner_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Planner();
    }

    void test_module()
    {
        Planner n;
        FooModule foo;
        n.module(foo);

        TS_ASSERT(n.m_modules.size() == 1);
        TS_ASSERT(n.m_modules.at(0) == &foo);
    }

    void test_simple_link()
    {
        Planner n;
        FooModule foo1, foo2;
        auto link = make_link(&foo2.in, &foo1.out, nullptr);
        n.module(foo1)
         .module(foo2)
         .connection(link);

        TS_ASSERT(n.m_links.size() == 1);
        const Link *c = n.m_links.at(0);
        TS_ASSERT(c == &link);
    }

    void test_control_link()
    {
        Planner n;
        FooModule foo1, foo2;
        auto cl = make_link(&foo2.in, &foo1.out, nullptr);
        n.module(foo1)
         .module(foo2)
         .connection(cl);

        TS_ASSERT(n.m_links.size() == 1);
        Link *p = n.m_links.at(0);
        TS_ASSERT(p == &cl);
    }

    // void test_simple_clone()
    // {
    //     Planner n;
    //     FooModule foo1, foo2;
    //     n.module(foo1)
    //      .module(foo2)
    //      .simple_connection(foo1.out, foo2.in);
    //     Voice *v = n.make_voice();
    //     Voice::module_vector m = v->modules();
    //
    //     // std::cout << "\n\n";
    //     // std::cout << "&foo1 = " << &foo1 << "\n";
    //     // std::cout << "&foo2 = " << &foo2 << "\n";
    //     // std::cout << "&m[0] = " << m.at(0) << "\n";
    //     // std::cout << "\n" << std::endl;
    //
    //     TS_ASSERT(m.size() == 2);
    //     TS_ASSERT(dynamic_cast<FooModule *>(m.at(0)));
    //     TS_ASSERT(m.at(0) != &foo1 && m.at(0) != &foo2);
    //
    //     delete v;
    // }

    void test_simple_plan()
    {
        Planner n;
        FooModule foo1, foo2;
        auto link = make_link(&foo2.in, &foo1.out, nullptr);
        n.module(foo1)
         .module(foo2)
         .connection(link);
        Plan p = n.make_plan();

        TS_ASSERT(p.prep().size() == 2);
        TS_ASSERT(p.prep().at(0).tag() == PrepStepTag::CLEAR);
        TS_ASSERT(p.prep().at(1).tag() == PrepStepTag::ALIAS);
        TS_ASSERT(p.run().size() == 2);
        TS_ASSERT(p.run().at(0).tag() == RenderStepTag::MODULE_RENDER);
        TS_ASSERT(p.run().at(1).tag() == RenderStepTag::MODULE_RENDER);
    }

    void test_cycle()
    {
        Planner n;
        FooModule foo1, foo2;
        auto link1 = make_link(&foo2.in, &foo1.out, nullptr);
        auto link2 = make_link(&foo1.in, &foo2.out, nullptr);
        n.module(foo1)
         .module(foo2)
         .connection(link1)
         .connection(link2);

        TS_ASSERT_THROWS(n.make_plan(), std::runtime_error);
    }

};
