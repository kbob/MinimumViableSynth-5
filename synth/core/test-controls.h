#include "controls.h"

#include <cxxtest/TestSuite.h>

#include "links.h"
#include "ports.h"

enum class Color { RED, GREEN, BLUE, PURPLE };

class ConcreteControl : public ControlType<Color> {
public:
    // virtual void copy(InputPort *, const ControlLink&) const override {}
    // virtual void add (InputPort *, const ControlLink&) const override {}
};

class ControlsUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)ConcreteControl();
    }

    // XXX now what?

};
