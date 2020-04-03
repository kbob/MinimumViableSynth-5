#include "synth/core/module.h"

#include <cxxtest/TestSuite.h>

class FooModule : public Module {
public:
    FooModule()
    {
        in.name("in");
        out.name("out");
        ports(in, out);
    }
    virtual FooModule *clone() const override { return new FooModule(*this); }
    Input<> in;
    Output<> out;
    virtual void render(size_t) override {}

};

class ModuleUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        //(void)Module();
        (void)FooModule();
    }
};
