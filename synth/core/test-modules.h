#include "modules.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/timbre.h"

class modules_unit_test : public CxxTest::TestSuite {

public:

    class FooModule : public ModuleType<FooModule> {
    public:
        FooModule()
        {
            in.name("in");
            out.name("out");
            ports(in, out);
        }
        Input<> in;
        Output<> out;
        size_t last_size;
        void *in_addr;
        void *out_addr;
        void render(size_t n)
        {
            last_size = n;
            in_addr = static_cast<void *>(in.buf());
            out_addr = static_cast<void *>(&out[0]);
            for (size_t i = 0; i < n; i++)
                out[i] = -in[i];
        }
    };

    void test_instantiate()
    {
        (void)FooModule();
    }

    void test_name()
    {
        FooModule foo;
        TS_ASSERT_EQUALS(foo.name(), "");
        foo.name("FerdinandOscarOmmitage");
        TS_ASSERT_EQUALS(foo.name(), "FerdinandOscarOmmitage");
    }

    void test_timbre()
    {
        Timbre t;
        FooModule foo;
        TS_ASSERT_EQUALS(foo.m_timbre, nullptr);
        foo.set_timbre(&t);
        TS_ASSERT_EQUALS(foo.m_timbre, &t);
    }

    void test_clone()
    {
        Timbre t;
        FooModule foo;
        foo.set_timbre(&t);
        foo.name("Fonzie");
        Module *bar = foo.clone();
        FooModule *fbar = dynamic_cast<FooModule *>(bar);
        TS_ASSERT(fbar);
        TS_ASSERT_EQUALS(bar->m_timbre, nullptr);
        TS_ASSERT_EQUALS(bar->name(), "Fonzie");
        TS_ASSERT_EQUALS(bar->ports().size(), 2);
        TS_ASSERT_EQUALS(bar->ports()[0], &fbar->in);
        TS_ASSERT_EQUALS(bar->ports()[1], &fbar->out);
    }

    void test_init()
    {
        FooModule foo;
        foo.init();
    }

    void test_render()
    {
        FooModule foo;
        foo.render(1);
        TS_ASSERT_EQUALS(foo.last_size, 1);
    }

    void test_render_action()
    {
        FooModule foo;
        auto action = foo.make_render_action();
        foo.in.buf()[0] = 3.3f;
        foo.in.buf()[1] = 4.4f;
        action(2);
        TS_ASSERT_EQUALS(foo.last_size, 2);
        TS_ASSERT_EQUALS(foo.in_addr, foo.in.buf());
        TS_ASSERT_EQUALS(foo.out_addr, &foo.out[0]);
        TS_ASSERT_EQUALS(foo.out[0], -3.3f);
        TS_ASSERT_EQUALS(foo.out[1], -4.4f);
    }

};
