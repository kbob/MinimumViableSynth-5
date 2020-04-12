#include "ported.h"

#include <cxxtest/TestSuite.h>

class MockPort : public Port {
public:
    std::type_index data_type() const override { return typeid(void); }
};

class MockPorted : public Ported {
public:
    MockPorted()
    {
        ports(p0, p1);
    }
    MockPort p0, p1;
};

class PortedUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)MockPorted();
    }

    void test_copy()
    {
        MockPorted pd0;
        MockPorted pd1(pd0);
    }

    // void test_assign()
    // {
    //     MockPorted pd0, pd1;
    //     pd1 = pd0;
    // }

    void test_ports()
    {
        MockPorted pd;
        const MockPorted::port_vector& ports = pd.ports();
        TS_ASSERT(ports.size() == 2);
        TS_ASSERT(ports.at(0) == &pd.p0);
        TS_ASSERT(ports.at(1) == &pd.p1);
    }

};
