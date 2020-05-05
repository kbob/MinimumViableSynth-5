#include "ported.h"

#include <typeindex>

#include <cxxtest/TestSuite.h>

class ported_unit_test : public CxxTest::TestSuite {

public:

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

    void test_instantiate()
    {
        (void)MockPorted();
    }

    void test_ports()
    {
        MockPorted pd;
        const MockPorted::port_vector& ports = pd.ports();
        TS_ASSERT(ports.size() == 2);
        TS_ASSERT(ports.at(0) == &pd.p0);
        TS_ASSERT(ports.at(1) == &pd.p1);
    }

    void test_copy()
    {
        MockPorted pd0;
        MockPorted pd1(pd0);
        TS_ASSERT(pd1.ports().size() == 2);
        TS_ASSERT(pd1.ports().at(0) == &pd1.p0);
        TS_ASSERT(pd1.ports().at(1) == &pd1.p1);
    }

};
