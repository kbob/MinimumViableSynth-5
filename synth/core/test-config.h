#include "config.h"

#include <cxxtest/TestSuite.h>

class config_unit_test : public CxxTest::TestSuite {

public:

    class FooSub : public Config::Subsystem {
    public:
        std::string member_function() const { return "foo"; }
    };

    void test_instantiate()
    {
        (void)Config();
    }

    void test_rate()
    {
        Config c;
        c.set_sample_rate(48000);
        TS_ASSERT_EQUALS(c.sample_rate(), 48000);
    }

    void test_subsys()
    {
        Config c;
        FooSub f;
        TS_ASSERT_THROWS(c.get<FooSub>(), std::out_of_range);
        c.register_subsystem(&f);
        auto p = c.get<FooSub>();
        TS_ASSERT(p);
        TS_ASSERT_EQUALS(p->member_function(), "foo");
    }
};
