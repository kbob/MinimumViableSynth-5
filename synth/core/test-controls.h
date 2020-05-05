#include "controls.h"

#include <string>

#include <cxxtest/TestSuite.h>

class core_controls_unit_test : public CxxTest::TestSuite {

public:

    enum class Color { RED, GREEN, BLUE, PURPLE };

    class ConcreteControl : public ControlType<ConcreteControl, Color> {
    public:
        void render(size_t frame_count)
        {
            for (size_t i = 0; i < frame_count; i++)
                out[i] = static_cast<Color>(i % 4);
        }
    };

    void test_instantiate()
    {
        (void)ConcreteControl();
        TS_TRACE("sizeof (Control) = " + std::to_string(sizeof (Control)));
        TS_TRACE("sizeof (ConcreteControl) = "
                 + std::to_string(sizeof (ConcreteControl)));
    }

    void test_copy()
    {
        ConcreteControl c1;
        ConcreteControl c2(c1);
        TS_ASSERT(c2.ports().size() == 1)
        TS_ASSERT(c2.ports().at(0) == &c2.out);
    }

    void test_clone()
    {
        ConcreteControl c1;
        Control *c2 = c1.clone();
        ConcreteControl *cc2 = dynamic_cast<ConcreteControl *>(c2);
        TS_ASSERT(cc2);
        TS_ASSERT(c2->ports().size() == 1)
        TS_ASSERT(c2->ports().at(0) == &cc2->out);
    }

    void test_port()
    {
        ConcreteControl c;
        Ported::port_vector ports = c.ports();
        TS_ASSERT(ports.size() == 1);
        TS_ASSERT(ports.at(0) == &c.out);
        TS_ASSERT(c.out.name() == "out");
    }

    void test_render_action()
    {
        ConcreteControl c;
        render_action render = c.make_render_action();
        for (size_t i = 0; i < MAX_FRAMES; i++)
            c.out[i] = Color::RED;
        render(MAX_FRAMES);
        for (size_t i = 0; i < MAX_FRAMES; i++)
            TS_ASSERT(c.out[i] == static_cast<Color>(i % 4));
    }

};
