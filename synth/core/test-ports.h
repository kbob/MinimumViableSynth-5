#include "ports.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/controls.h"
#include "synth/core/modules.h"

class ports_unit_test : public CxxTest::TestSuite {

public:

    class FooControl : public ControlType<FooControl> {
    public:
        void render(size_t) {}
    };

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

    void test_instantiate()
    {
        (void)Input<>();
        (void)Output<>();
    }

    void test_name()
    {
        Input<> i;
        TS_ASSERT_EQUALS(i.name(), "");
        i.name("Ferdinand");
        TS_ASSERT_EQUALS(i.name(), "Ferdinand");
    }

    void test_control()
    {
        FooControl c;

        TS_ASSERT_EQUALS(c.out.owner(), &c);
    }

    void test_module()
    {
        Input<> i;
        Output<> o;
        EmptyModule m;

        TS_ASSERT_EQUALS(i.owner(), nullptr);
        TS_ASSERT_EQUALS(o.owner(), nullptr);
        i.owner(m);
        o.owner(m);
        TS_ASSERT_EQUALS(i.owner(), &m);
        TS_ASSERT_EQUALS(o.owner(), &m);
    }

    void test_module_ports()
    {
        IOModule m;

        TS_ASSERT_EQUALS(m.ports().at(0), &m.in);
        TS_ASSERT_EQUALS(m.ports().at(1), &m.out);
        TS_ASSERT_EQUALS(m.in.owner(), &m);
        TS_ASSERT_EQUALS(m.out.owner(), &m);
    }

    void test_types()
    {
        Input<bool> i;
        Output<char> o;
    }

    void test_data_type()
    {
        Input<bool> ib;
        Output<short> os;

        // can't use TS_ASSERT_EQUALS here.
        TS_ASSERT(ib.data_type() == typeid(bool));
        TS_ASSERT(os.data_type() == typeid(short));
        TS_ASSERT_DIFFERS(ib.data_type(), os.data_type());
    }

    void test_buf()
    {
        Input<bool> ib;
        Output<short> os;
        TS_ASSERT_EQUALS(ib.buf(), ib.m_buf);
        TS_ASSERT_EQUALS(os.buf(), os.m_buf);
    }

    void test_inport_copy()
    {
        Input<> in;
        Input<> copy(in);

        TS_ASSERT(in.m_data == in.m_buf);
        TS_ASSERT(copy.m_data == copy.m_buf);
    }

    void test_inport_clear()
    {
        Input<bool> ib;
        for (size_t i = 0; i < MAX_FRAMES; i++)
            ib.m_buf[i] = false;
        ib.clear(1.0f);
        for (size_t i = 0; i < MAX_FRAMES; i++)
            TS_ASSERT_EQUALS(ib[i], true);
    }

    void test_inport_alias()
    {
        Input<> in;
        Output<> out;
        for (size_t i = 0; i < MAX_FRAMES; i++) {
            in.m_buf[i] = 10 + i;
            out[i] = 20 - i;
        }

        in.alias(out.void_buf());
        for (size_t i = 0; i < MAX_FRAMES; i++) {
            TS_ASSERT_EQUALS(in[i], 20 - i);
            TS_ASSERT_EQUALS(out[i], 20 - i);
            TS_ASSERT_EQUALS(in.m_buf[i], 10 + i);
        }
    }

};
