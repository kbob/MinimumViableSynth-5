#include "timbre.h"

#include <sstream>
#include <string>

#include <cxxtest/TestSuite.h>

#include "synth/core/ports.h"
#include "synth/core/steps.h"

class timbre_unit_test : public CxxTest::TestSuite {

public:

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

    void test_instantiate()
    {
        (void)Timbre();
        TS_TRACE("sizeof (Timbre) = " + std::to_string(sizeof (Timbre)));
    }

    void test_copy()
    {
        FooControl *c = new FooControl;
        FooModule *m = new FooModule;
        Timbre t;
        t.add_control(c);
        t.add_module(m);
        Timbre t2 = t;

        auto ctls = t2.controls();
        TS_ASSERT_EQUALS(ctls.size(), 1);
        TS_ASSERT_DIFFERS(ctls.at(0), c);
        TS_ASSERT(dynamic_cast<FooControl *>(ctls.at(0)));
        auto mods = t2.modules();
        TS_ASSERT_EQUALS(mods.size(), 1);
        TS_ASSERT_DIFFERS(mods.at(0), m);
        TS_ASSERT(dynamic_cast<FooModule *>(mods.at(0)));
    }

    void test_patch()
    {
        Patch c, d;
        Timbre t;
        TS_ASSERT_EQUALS(t.current_patch(), nullptr);
        TS_ASSERT_EQUALS(t.default_patch(), nullptr);
        t.default_patch(&d);
        t.set_patch(&c);
        TS_ASSERT_EQUALS(t.current_patch(), &c);
        TS_ASSERT_EQUALS(t.default_patch(), &d);
        t.set_patch(nullptr);
        TS_ASSERT_EQUALS(t.current_patch(), nullptr);
    }

    void test_plan()
    {
        Plan p;
        PrepStep s{ClearStep(42, 0.5f)};
        p.t_prep().push_back(s);
        Timbre t;

        TS_ASSERT_EQUALS(t.plan().t_prep().size(), 0);
        t.plan(p);
        TS_ASSERT_EQUALS(t.plan().t_prep().size(), 1);
    }

    void test_add_control()
    {
        Timbre t;
        FooControl *foo{new FooControl};
        t.add_control(foo);

        const Timbre::control_vector& cv = t.controls();
        TS_ASSERT_EQUALS(cv.size(), 1);
        TS_ASSERT_EQUALS(cv.at(0), foo);
        TS_ASSERT_LESS_THAN_EQUALS(cv.capacity(), MAX_VOICE_CONTROLS);
    }

    void test_add_module()
    {
        Timbre t;
        FooModule *foo{new FooModule};
        t.add_module(foo);

        const Timbre::module_vector& mv = t.modules();
        TS_ASSERT_EQUALS(mv.size(), 1);
        TS_ASSERT_EQUALS(mv.at(0), foo);
        TS_ASSERT_LESS_THAN_EQUALS(mv.capacity(), MAX_VOICE_MODULES);
    }

    void test_pre_render()
    {
        std::ostringstream log;
        auto a0 = [&] (size_t n) {
            log << 'a' << n << ' ';
        };
        auto a1 = [&] (size_t n) {
            log << 'b' << n << ' ';
        };
        render_action_sequence seq{a0, a1};
        Timbre t;
        t.pre_actions(seq);
        TS_ASSERT_EQUALS(t.pre_actions().size(), 2);
        t.pre_render(10);
        TS_ASSERT_EQUALS(log.str(), "a10 b10 ");
    }

    void test_post_render()
    {
        std::ostringstream log;
        auto a0 = [&] (size_t n) {
            log << 'a' << n << ' ';
        };
        auto a1 = [&] (size_t n) {
            log << 'b' << n << ' ';
        };
        render_action_sequence seq{a0, a1};
        Timbre t;
        t.post_actions(seq);
        TS_ASSERT_EQUALS(t.post_actions().size(), 2);
        t.post_render(10);
        TS_ASSERT_EQUALS(log.str(), "a10 b10 ");
    }

};
