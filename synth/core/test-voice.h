#include "voice.h"

#include <sstream>

#include <cxxtest/TestSuite.h>

#include "synth/core/ports.h"
#include "synth/core/timbre.h"

class voice_unit_test : public CxxTest::TestSuite {

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

    void test_instantiation()
    {
        (void)Voice();
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
        TS_ASSERT_DIFFERS(ctls.at(0), c1);
        TS_ASSERT_DIFFERS(ctls.at(0), c2);
        TS_ASSERT(dynamic_cast<FooControl *>(ctls.at(0)));
        auto mods = v2.modules();
        TS_ASSERT_EQUALS(mods.size(), 2);
        TS_ASSERT_DIFFERS(mods.at(0), m1)
        TS_ASSERT_DIFFERS(mods.at(0), m2);
        TS_ASSERT(dynamic_cast<FooModule *>(mods.at(0)));
        TS_ASSERT_DIFFERS(mods.at(1), m1)
        TS_ASSERT_DIFFERS(mods.at(1), m2);
        TS_ASSERT(dynamic_cast<FooModule *>(mods.at(1)));
    }

    void state()
    {
        Voice v;
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
    }

    void timbre()
    {
        Voice v;
        Timbre t;
        TS_ASSERT_EQUALS(v.timbre(), nullptr);
        v.timbre(&t);
        TS_ASSERT_EQUALS(v.timbre(), &t);
    }

    void test_add_control()
    {
        Voice v;
        FooControl *foo{new FooControl};
        v.add_control(foo);

        const Voice::control_vector& cv = v.controls();
        TS_ASSERT_EQUALS(cv.size(), 1);
        TS_ASSERT_EQUALS(cv.at(0), foo);
        TS_ASSERT_LESS_THAN_EQUALS(cv.capacity(), MAX_VOICE_CONTROLS);
    }

    void test_add_module()
    {
        Voice v;
        FooModule *foo{new FooModule};
        v.add_module(foo);

        const Voice::module_vector& mv = v.modules();
        TS_ASSERT_EQUALS(mv.size(), 1);
        TS_ASSERT_EQUALS(mv.at(0), foo);
        TS_ASSERT_LESS_THAN_EQUALS(mv.capacity(), MAX_VOICE_MODULES);
    }

    void test_two_modules()
    {
        Voice v;
        FooModule *foo1{new FooModule}, *foo2{new FooModule};
        v.add_module(foo1);
        v.add_module(foo2);

        auto m = v.modules();
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(0), foo1);
        TS_ASSERT_EQUALS(m.at(1), foo2);
    }

    void test_note()
    {
        Voice v;
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
        v.start_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::SOUNDING);
        v.release_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::RELEASING);
        v.render(1);
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
    }

    void test_kill_note()
    {
        Config cfg;
        cfg.set_sample_rate(44100);
        Voice v;
        v.configure(cfg);
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
        v.start_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::SOUNDING);
        v.kill_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::STOPPING);
        int shutdown_frames = cfg.sample_rate() * NOTE_SHUTDOWN_TIME;
        for (int frame = 0; frame < shutdown_frames; frame += MAX_FRAMES) {
            TS_ASSERT_EQUALS(v.state(), Voice::State::STOPPING);
            v.render(MAX_FRAMES);
        }
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
    }

    void test_kill_released_note()
    {
        Config cfg;
        cfg.set_sample_rate(44100);
        Voice v;
        v.configure(cfg);
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
        v.start_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::SOUNDING);
        v.release_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::RELEASING);
        v.kill_note();
        TS_ASSERT_EQUALS(v.state(), Voice::State::STOPPING);
        int shutdown_frames = cfg.sample_rate() * NOTE_SHUTDOWN_TIME;
        for (int frame = 0; frame < shutdown_frames; frame += MAX_FRAMES) {
            TS_ASSERT_EQUALS(v.state(), Voice::State::STOPPING);
            v.render(MAX_FRAMES);
        }
        TS_ASSERT_EQUALS(v.state(), Voice::State::IDLE);
    }

    void test_render()
    {
        std::ostringstream log;
        auto a0 = [&] (size_t n) {
            log << 'a' << n << ' ';
        };
        auto a1 = [&] (size_t n) {
            log << 'b' << n << ' ';
        };
        render_action_sequence seq{a0, a1};
        Voice v;
        v.actions(seq);
        v.start_note();
        TS_ASSERT_EQUALS(v.actions().size(), 2);
        TS_ASSERT_EQUALS(v.state(), Voice::State::SOUNDING);
        v.render(4);
        TS_ASSERT_EQUALS(log.str(), "a4 b4 ");
    }

};
