#include "ports.h"

#include <cxxtest/TestSuite.h>

#include "modules.h"

class EmptyModule : public ModuleType<EmptyModule> {
public:
    void render(size_t) {}
};

class IOModule : public ModuleType<IOModule> {
public:
    IOModule()
    {
        ports(in, out);
    }
    Input<> in;
    Output<> out;
    void render(size_t) {}
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

        TS_ASSERT(i.owner() == nullptr);
        TS_ASSERT(o.owner() == nullptr);
        i.owner(m);
        o.owner(m);
        TS_ASSERT(i.owner() == &m);
        TS_ASSERT(o.owner() == &m);
    }

    void test_module_ports()
    {
        IOModule m;

        TS_ASSERT(m.ports().at(0) == &m.in);
        TS_ASSERT(m.ports().at(1) == &m.out);
        TS_ASSERT(m.in.owner() == &m);
        TS_ASSERT(m.out.owner() == &m);
    }

    void test_types()
    {
        Input<bool> i;
        Output<char> o;
    }

    void test_data_type()
    {
        Input<bool> ib;
        Input<short> is;

        TS_ASSERT(ib.data_type() == typeid(bool));
        TS_ASSERT(is.data_type() == typeid(short));
        TS_ASSERT(ib.data_type() != is.data_type());
    }

    // XXX need more tests - use the [] operators.

};
