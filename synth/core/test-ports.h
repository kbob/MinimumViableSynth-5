#include "ports.h"

#include <cxxtest/TestSuite.h>

#include "module.h"

class EmptyModule : public Module {
public:
    virtual EmptyModule *clone() const override
    {
        return new EmptyModule(*this);
    }
    virtual void render(size_t) override {}
};

class IOModule : public Module {
public:
    IOModule()
    {
        ports(in, out);
    }
    virtual IOModule *clone() const override { return new IOModule(*this); }
    Input<> in;
    Output<> out;
    virtual void render(size_t) override {}
};

class PortsUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Input<>();
        (void)Output<>();
    }

    void test_name()
    {
        Input<> i;
        TS_ASSERT(i.name() == "");
        i.name("Ferdinand");
        TS_ASSERT(i.name() == "Ferdinand");
    }

    void test_module()
    {
        Input<> i;
        Output<> o;
        EmptyModule m;
        TS_ASSERT(i.module() == nullptr);
        TS_ASSERT(o.module() == nullptr);
        i.module(m);
        o.module(m);
        TS_ASSERT(i.module() == &m);
        TS_ASSERT(o.module() == &m);
    }

    void test_module_ports()
    {
        IOModule m;
        TS_ASSERT(m.ports().at(0) == &m.in);
        TS_ASSERT(m.ports().at(1) == &m.out);
        TS_ASSERT(m.in.module() == &m);
        TS_ASSERT(m.out.module() == &m);
    }

    void test_types()
    {
        Input<bool> i;
        Output<char> o;
    }

    // XXX need more tests - use the [] operators.

};
