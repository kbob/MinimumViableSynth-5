#include "voice.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/config.h"

class FooControl : public ControlType<FooControl> {
public:
    void render(size_t) {}
};

class FooModule : public ModuleType<FooModule> {
public:
    Input<> in;
    Output<> out;
    void render(size_t) {}
};

class voice_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiation()
    {
        (void)Voice();
    }

    void test_add_control()
    {
        Voice v;
        FooControl *foo{new FooControl};
        v.add_control(foo);

        const Voice::control_vector& m = v.controls();
        TS_ASSERT(m.size() == 1);
        TS_ASSERT(m.at(0) == foo);
        TS_ASSERT(m.capacity() <= MAX_VOICE_CONTROLS);
    }

    void test_add_module()
    {
        Voice v;
        FooModule *foo{new FooModule};
        v.add_module(foo);

        const Voice::module_vector& m = v.modules();
        TS_ASSERT(m.size() == 1);
        TS_ASSERT(m.at(0) == foo);
        TS_ASSERT(m.capacity() <= MAX_VOICE_MODULES);
    }

    void test_two_modules()
    {
        Voice v;
        FooModule *foo1{new FooModule}, *foo2{new FooModule};
        v.add_module(foo1);
        v.add_module(foo2);

        auto m = v.modules();
        TS_ASSERT(m.size() == 2);
        TS_ASSERT(m.at(0) == foo1);
        TS_ASSERT(m.at(1) == foo2);
    }

    void test_copy()
    {
        Voice v;
        FooControl *c1{new FooControl}, *c2{new FooControl};
        FooModule *m1{new FooModule}, *m2{new FooModule};
        v.add_control(c1);
        v.add_control(c2);
        v.add_module(m1);
        v.add_module(m2);
        Voice v2(v);

        auto ctls = v2.controls();
        TS_ASSERT_EQUALS(ctls.size(), 2);
        TS_ASSERT(ctls.at(0) != c1 && ctls.at(0) != c2);
        TS_ASSERT(dynamic_cast<FooControl *>(ctls.at(0)));
        auto mods = v2.modules();
        TS_ASSERT(mods.size() == 2);
        TS_ASSERT(mods.at(0) != m1 && mods.at(0) != m2);
        TS_ASSERT(dynamic_cast<FooModule *>(mods.at(0)));
        TS_ASSERT(mods.at(1) != m1 && mods.at(1) != m2);
        TS_ASSERT(dynamic_cast<FooModule *>(mods.at(1)));
    }

};
