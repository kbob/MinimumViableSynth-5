#include "synth/core/mod-network.h"

#include <cxxtest/TestSuite.h>

class FooModule : public Module<FooModule> {
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

class ModNetworkUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)ModNetwork();
    }

    void test_module()
    {
        ModNetwork n;
        FooModule foo;
        n.module(foo);

        TS_ASSERT(n.m_modules.size() == 1);
        TS_ASSERT(n.m_modules.at(0) == &foo);
    }

    void test_simple_link()
    {
        ModNetwork n;
        FooModule foo1, foo2;
        n.module(foo1)
         .module(foo2)
         .simple_connection(foo1.out, foo2.in);

        TS_ASSERT(n.m_simple_links.size() == 1);
        SimpleLink& c = n.m_simple_links.at(0);
        TS_ASSERT(c.key().src() == &foo1.out);
        TS_ASSERT(c.key().dest() == &foo2.in);
        TS_ASSERT(c.key().control() == nullptr);
    }

    void test_control_link()
    {
        ModNetwork n;
        FooModule foo1, foo2;
        ControlLink cl(foo1.out, foo2.in);
        n.module(foo1)
         .module(foo2)
         .control_connection(cl);

        TS_ASSERT(n.m_control_links.size() == 1);
        ControlLink *p = n.m_control_links.at(0);
        TS_ASSERT(p->key().src() == &foo1.out);
        TS_ASSERT(p->key().dest() == &foo2.in);
        TS_ASSERT(p->key().control() == nullptr);
    }

    void test_simple_clone()
    {
        ModNetwork n;
        FooModule foo1, foo2;
        n.module(foo1)
         .module(foo2)
         .simple_connection(foo1.out, foo2.in);
        Voice *v = n.make_voice();
        Voice::module_vector m = v->modules();

        // std::cout << "\n\n";
        // std::cout << "&foo1 = " << &foo1 << "\n";
        // std::cout << "&foo2 = " << &foo2 << "\n";
        // std::cout << "&m[0] = " << m.at(0) << "\n";
        // std::cout << "\n" << std::endl;

        TS_ASSERT(m.size() == 2);
        TS_ASSERT(dynamic_cast<FooModule *>(m.at(0)));
        TS_ASSERT(m.at(0) != &foo1 && m.at(0) != &foo2);

        delete v;
    }

    void test_simple_plan()
    {
        ModNetwork n;
        FooModule foo1, foo2;
        n.module(foo1)
         .module(foo2)
         .simple_connection(foo1.out, foo2.in);
        Plan p = n.make_plan();

        TS_ASSERT(p.prep().size() == 2);
        TS_ASSERT(p.prep().at(0).type() == PrepActionType::CLEAR);
        TS_ASSERT(p.prep().at(1).type() == PrepActionType::ALIAS);
        std::cout << "run size = " << p.run().size() << std::endl;
        TS_ASSERT(p.run().size() == 2);
        TS_ASSERT(p.run().at(0).type() == RunActionType::RENDER);
        TS_ASSERT(p.run().at(1).type() == RunActionType::RENDER);
    }

    void test_cycle()
    {
        ModNetwork n;
        FooModule foo1, foo2;
        n.module(foo1)
         .module(foo2)
         .simple_connection(foo1.out, foo2.in)
         .simple_connection(foo2.out, foo1.in);

        TS_ASSERT_THROWS(n.make_plan(), std::runtime_error);
    }

};
