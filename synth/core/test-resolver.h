#include "resolver.h"

#include <array>
#include <list>

#include <cxxtest/TestSuite.h>

class ConcreteControl : public ControlType<ConcreteControl> {};

class ConcreteModule : public ModuleType<ConcreteModule> {
public:
    ConcreteModule()
    {
        in.name("in");
        out.name("out");
        ports(in, out);
    }
    Input<> in;
    Output<> out;
    void render(size_t) {}
};

class resolver_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Resolver();
    }

    typedef std::list<Control *> CC;
    typedef std::array<Module *, 2> MC;

    ConcreteControl tc0, tc1, vc2;
    ConcreteModule tm0, tm1, vm2, vm3;
    CC tcontrols{&tc0, &tc1};
    CC vcontrols{&vc2};
    MC tmodules{&tm0, &tm1};
    MC vmodules{&vm2, &vm3};
    Resolver tr, vr;

    resolver_unit_test()
    {
        tr.add_controls(tcontrols.begin(), tcontrols.end());
        tr.add_modules(tmodules.begin(), tmodules.end());
        tr.finalize();

        // N.B., must add timbre objects before voice objects.
        vr.add_controls(tcontrols.begin(), tcontrols.end());
        vr.add_modules(tmodules.begin(), tmodules.end());
        vr.add_controls(vcontrols.begin(), vcontrols.end());
        vr.add_modules(vmodules.begin(), vmodules.end());
        vr.finalize();
    }

    void test_controls()
    {
        TS_ASSERT_EQUALS(tr.controls().size(), 2);
        TS_ASSERT_EQUALS(tr.controls()[0], &tc0);
        TS_ASSERT_EQUALS(tr.controls()[1], &tc1);
        TS_ASSERT_EQUALS(tr.controls().all, 0b11);
        TS_ASSERT_EQUALS(tr.controls().none, 0b00);
        TS_ASSERT_EQUALS(tr.controls().find(&tc1), 1);
        TS_ASSERT_EQUALS(tr.controls().index(&tc1), 1);
        TS_ASSERT_EQUALS(tr.controls().find(&vc2), -1);
        TS_ASSERT_THROWS(tr.controls().index(&vc2), std::logic_error);

        TS_ASSERT_EQUALS(vr.controls().size(), 3);
        TS_ASSERT_EQUALS(vr.controls()[0], &tc0);
        TS_ASSERT_EQUALS(vr.controls()[1], &tc1);
        TS_ASSERT_EQUALS(vr.controls()[2], &vc2);
        TS_ASSERT_EQUALS(vr.controls().all, 0b111);
    }

    void test_modules()
    {
        TS_ASSERT_EQUALS(tr.modules().size(), 2);
        TS_ASSERT_EQUALS(tr.modules()[0], &tm0);
        TS_ASSERT_EQUALS(tr.modules()[1], &tm1);
        TS_ASSERT_EQUALS(tr.modules().all, 0b11);
        TS_ASSERT_EQUALS(tr.modules().none, 0b00);
        TS_ASSERT_EQUALS(tr.modules().find(&tm1), 1);
        TS_ASSERT_EQUALS(tr.modules().index(&tm1), 1);
        TS_ASSERT_EQUALS(tr.modules().find(&vm2), -1);
        TS_ASSERT_THROWS(tr.modules().index(&vm2), std::logic_error);

        TS_ASSERT_EQUALS(vr.modules().size(), 4);
        TS_ASSERT_EQUALS(vr.modules()[0], &tm0);
        TS_ASSERT_EQUALS(vr.modules()[1], &tm1);
        TS_ASSERT_EQUALS(vr.modules()[2], &vm2);
        TS_ASSERT_EQUALS(vr.modules()[3], &vm3);
        TS_ASSERT_EQUALS(vr.modules().all, 0b1111);
    }

    void test_ports()
    {
        TS_ASSERT_EQUALS(tr.ports().size(), 6);
        TS_ASSERT_EQUALS(tr.ports()[0], &tc0.out);
        TS_ASSERT_EQUALS(tr.ports()[1], &tc1.out);
        TS_ASSERT_EQUALS(tr.ports()[2], &tm0.in);
        TS_ASSERT_EQUALS(tr.ports()[3], &tm0.out);
        TS_ASSERT_EQUALS(tr.ports()[4], &tm1.in);
        TS_ASSERT_EQUALS(tr.ports()[5], &tm1.out);
        TS_ASSERT_EQUALS(tr.ports().all, 0b111111);
        TS_ASSERT_EQUALS(tr.ports().none, 0b000000);
        TS_ASSERT_EQUALS(tr.ports().find(&tc1.out), 1);
        TS_ASSERT_EQUALS(tr.ports().index(&tm1.in), 4);
        TS_ASSERT_EQUALS(tr.ports().find(&vm2.in), -1);
        TS_ASSERT_THROWS(tr.ports().index(&vc2.out), std::logic_error);

        TS_ASSERT_EQUALS(vr.ports().size(), 11);
        TS_ASSERT_EQUALS(vr.ports()[0], &tc0.out);
        TS_ASSERT_EQUALS(vr.ports()[1], &tc1.out);
        TS_ASSERT_EQUALS(vr.ports()[2], &tm0.in);
        TS_ASSERT_EQUALS(vr.ports()[3], &tm0.out);
        TS_ASSERT_EQUALS(vr.ports()[4], &tm1.in);
        TS_ASSERT_EQUALS(vr.ports()[5], &tm1.out);
        TS_ASSERT_EQUALS(vr.ports()[6], &vc2.out);
        TS_ASSERT_EQUALS(vr.ports()[7], &vm2.in);
        TS_ASSERT_EQUALS(vr.ports()[8], &vm2.out);
        TS_ASSERT_EQUALS(vr.ports()[9], &vm3.in);
        TS_ASSERT_EQUALS(vr.ports()[10], &vm3.out);
        TS_ASSERT_EQUALS(vr.ports().all, 0b11111111111);

        // port mapping must match between timbre resolver
        // and voice resolver.
        for (size_t i = 0; i < 6; i++)
            TS_ASSERT_EQUALS(tr.ports()[i], vr.ports()[i]);
    }

};
