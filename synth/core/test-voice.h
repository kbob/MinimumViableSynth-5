#include "synth/core/voice.h"

#include <cxxtest/TestSuite.h>

class FooModule : public Module<FooModule> {
public:
    Input<> in;
    Output<> out;
    void render(size_t) {}
};

class VoiceUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiation()
    {
        (void)Voice();
    }

    void test_add_module()
    {
        Voice v;
        FooModule foo;
        v.module(&foo);

        const Voice::module_vector& m = v.modules();
        TS_ASSERT(m.size() == 1);
        TS_ASSERT(m.at(0) == &foo);
        TS_ASSERT(m.capacity() <= Voice::MAX_MODULES);
    }

    void test_two_modules()
    {
        Voice v;
        FooModule foo1, foo2;
        v.module(&foo1);
        v.module(&foo2);

        auto m = v.modules();
        TS_ASSERT(m.size() == 2);
        TS_ASSERT(m.at(0) == &foo1);
        TS_ASSERT(m.at(1) == &foo2);
    }

    // XXX test port enumeration

    void test_clone()
    {
        Voice v;
        FooModule foo1, foo2;
        v.module(&foo1);
        v.module(&foo2);
        Voice *v2 = v.clone();

        auto m2 = v2->modules();
        TS_ASSERT(m2.size() == 2);
        TS_ASSERT(m2.at(0) != &foo1 && m2.at(0) != &foo2);
        TS_ASSERT(dynamic_cast<FooModule *>(m2.at(0)));
        TS_ASSERT(m2.at(1) != &foo1 && m2.at(1) != &foo2);
        TS_ASSERT(dynamic_cast<FooModule *>(m2.at(1)));
    }

};
