#include "modules.h"

#include <cxxtest/TestSuite.h>

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
    void render(size_t n) { last_size = n; }

};

class ModulesUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)FooModule();
    }

    void test_name()
    {
        FooModule foo;
        TS_ASSERT(foo.name() == "");
        foo.name("FerdinandOscarOmmitage");
        TS_ASSERT(foo.name() == "FerdinandOscarOmmitage");
    }

    void test_clone()
    {
        FooModule foo;
        foo.name("Fonzie");
        Module *bar = foo.clone();
        TS_ASSERT(bar->name() == "Fonzie");
        TS_ASSERT(dynamic_cast<FooModule *>(bar));
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
        TS_ASSERT(foo.last_size == 1);
    }

    void test_render_action()
    {
        FooModule foo;
        auto action = foo.make_render_action();
        action(2);
        TS_ASSERT(foo.last_size == 2);
    }

};
