#include "synth/core/mod-network.h"

// #include "synth/core/signalgraph.h"

#include <cxxtest/TestSuite.h>

class FooModule : public Module {
public:
    FooModule()
    {
        in.name("in");
        out.name("out");
        ports(in, out);
    }
    virtual FooModule *clone() const override { return new FooModule(*this); }
    Input<> in;
    Output<> out;
    virtual void render(size_t) const override {}

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

        // TS_ASSERT(p.prep().size() == 2);
        // auto *a0 = p.prep().at(0);
        // auto *a1 = p.prep().at(1);

    }
};
