#include "controls.h"

#include <cxxtest/TestSuite.h>

enum class Color { RED, GREEN, BLUE, PURPLE };

class ConcreteControl : public ControlType<ConcreteControl, Color> {
public:
    void render(size_t) {}
};

class core_controls_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)ConcreteControl();
    }

    void test_copy()
    {
        ConcreteControl c1;
        ConcreteControl c2(c1);
    }

    void test_port()
    {
        ConcreteControl c;
        Ported::port_vector ports = c.ports();
        TS_ASSERT(ports.size() == 1);
        TS_ASSERT(ports.at(0) == &c.out);
        TS_ASSERT(c.out.name() == "out");
    }

};
