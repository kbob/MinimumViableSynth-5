#include "mode-mgr.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/asgn-prio.h"

using midi::CHANNEL_COUNT;
using midi::ChannelMode;
using midi::Dispatcher;
using midi::Layering;
using midi::ModeManager;
using midi::NoteManager;
using midi::SmallMessage;

class mode_manager_unit_test : public CxxTest::TestSuite {

public:

    // static Voice v;

    // class FooAssigner : public Assigner {
    // public:
    //     Voice *assign_idle_voice() override { return &v; }
    //     Voice *choose_voice_to_steal() override { return nullptr; }
    // };

    static int prioritize(const Voice&)
    {
        return 0;
    }

    void handle_note_number(std::uint8_t note)
    {
        // std::cout << "\n\nhere\n\n";
        (void)note;
    }

    void test_instantiate()
    {
        (void)midi::ModeManager();
    }

    void test_default_modes()
    {
        size_t TIMB = MAX_TIMBRALITY - 1;
        Layering l(TIMB);
        Dispatcher d;
        d.attach_layering(l);
        NoteManager nm;
        nm.attach_dispatcher(d);
        ModeManager mm;
        mm.attach_dispatcher(d);
        mm.attach_layering(l);
        mm.attach_note_manager(nm);

        TS_ASSERT_EQUALS(mm.basic_channel(), 0);
        TS_ASSERT_EQUALS(mm.mode(), ChannelMode::OMNI_POLY);
        TS_ASSERT(mm.is_omni());
        TS_ASSERT(!mm.is_mono());
        TS_ASSERT(!mm.is_multi());
        TS_ASSERT_EQUALS(mm.mode4_active_channels(), Layering::ALL_CHANNELS);
    }

    void test_mode_1()
    {
        size_t POLY = MAX_POLYPHONY, TIMB = MAX_TIMBRALITY - 1;
        ::Config cfg;
        cfg.set_sample_rate(44100);
        ::Synth s("Foo", POLY, TIMB);
        s.finalize(cfg);
        Layering l(TIMB);
        Dispatcher d;
        d.attach_layering(l);
        // FooAssigner fa;
        ::PriorityAssigner pa(s, ::PriorityAssigner::prioritizer(prioritize));
        NoteManager nm;
        nm.attach_dispatcher(d);
        nm.attach_assigner(pa);
        nm.attach_synth(s);
        ModeManager mm;
        mm.attach_dispatcher(d);
        mm.attach_layering(l);
        mm.attach_note_manager(nm);

        nm.register_note_number_handler(
            0,
            NoteManager::note_number_handler::binding<
                mode_manager_unit_test,
                &mode_manager_unit_test::handle_note_number
            >(this));

        // Mode 2
        d.dispatch_message(SmallMessage(0xB0, 124, 0)); // Omni Mode Off, ch#0
        TS_ASSERT(!mm.is_omni());
        TS_ASSERT(!mm.is_mono());
        TS_ASSERT(!mm.is_multi());

        // Send note on messages.  Ensure they come out on all timbres.
        d.dispatch_message(SmallMessage(0x90, 60, 64));

    }

    // void test_multi()
    // {
    //     size_t TIMB = MAX_TIMBRALITY - 1;
    //     Layering l(TIMB);
    //     Dispatcher d;
    //     d.attach_layering(l);
    //     NoteManager nm;
    //     nm.attach_dispatcher(d);
    //     ModeManager mm;
    //     mm.attach_layering(l);
    //     mm.attach_dispatcher(d);
    //     mm.attach_note_manager(nm);
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), l.all_timbres);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     mm.multi_mode(true);
    //     TS_ASSERT(!mm.is_omni());
    //     TS_ASSERT(!mm.is_mono());
    //     TS_ASSERT(mm.is_multi());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), (ti < TIMB ? 1 << ti : 0));
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     mm.multi_mode(false);
    //     TS_ASSERT(mm.is_omni());
    //     TS_ASSERT(!mm.is_mono());
    //     TS_ASSERT(!mm.is_multi());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), l.all_timbres);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    // }
    //
    // // Test receiving omni on/off messages.
    // // Use basic channel and non-basic channel.
    // // Change the basic channel.
    // void test_omni()
    // {
    //     size_t TIMB = MAX_TIMBRALITY - 1;
    //     Layering l(TIMB);
    //     Dispatcher d;
    //     d.attach_layering(l);
    //     NoteManager nm;
    //     nm.attach_dispatcher(d);
    //     ModeManager mm;
    //     mm.attach_layering(l);
    //     mm.attach_dispatcher(d);
    //     mm.attach_note_manager(nm);
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), l.all_timbres);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //     TS_ASSERT(mm.basic_channel() == 0);
    //
    //     // message = Channel Mode, channel 1, number OMNI_MODE_OFF
    //     // expected to no-op.
    //     d.dispatch_message(SmallMessage(0xB1, 124, 0));
    //     TS_ASSERT(mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), l.all_timbres);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     // message = Channel Mode, channel 0, number OMNI_MODE_OFF
    //     // expected to switch to non-omni mode.
    //     d.dispatch_message(SmallMessage(0xB0, 124, 0));
    //     TS_ASSERT(!mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), (ti < TIMB ? 1 << ti : 0));
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     // message = Channel Mode, channel 2, number OMNI_MODE_ON
    //     // expected to no-op.
    //     d.dispatch_message(SmallMessage(0xB2, 125, 0));
    //     TS_ASSERT(!mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), (ti < TIMB ? 1 << ti : 0));
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     mm.basic_channel(2);
    //
    //     // message = Channel Mode, channel 2, number OMNI_MODE_ON
    //     // expected to switch to omni mode.
    //     d.dispatch_message(SmallMessage(0xB2, 125, 0));
    //     TS_ASSERT(mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), l.all_timbres);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    // }
    //
    // // Test receiving omni on/off messages while in multi mode.
    // void test_omni_multi()
    // {
    //     size_t TIMB = MAX_TIMBRALITY - 1;
    //     Layering l(TIMB);
    //     Dispatcher d;
    //     d.attach_layering(l);
    //     NoteManager nm;
    //     nm.attach_dispatcher(d);
    //     ModeManager mm;
    //     mm.attach_layering(l);
    //     mm.attach_dispatcher(d);
    //     mm.attach_note_manager(nm);
    //
    //     mm.multi_mode(true);
    //     TS_ASSERT(!mm.is_omni());
    //
    //     // message = Channel Mode, channel 0, number OMNI_MODE_OFF
    //     // expected to remain in non-omni mode.
    //     d.dispatch_message(SmallMessage(0xB0, 124, 0));
    //     TS_ASSERT(!mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), (ti < TIMB ? 1 << ti : 0));
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    //
    //     // message = Channel Mode, channel 0, number OMNI_MODE_ON
    //     // expected to switch to non-omni mode.
    //     d.dispatch_message(SmallMessage(0xB0, 125, 0));
    //     TS_ASSERT(!mm.is_omni());
    //     for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
    //         TS_ASSERT_EQUALS(l.channel_timbres(ti), (ti < TIMB ? 1 << ti : 0));
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_mode(ci), NoteManager::Mode::POLY);
    // }
    //
    // // Test receiving mono/poly messages.
    // void test_mono_poly()
    // {
    //     size_t TIMB = MAX_TIMBRALITY - 1;
    //     Layering l(TIMB);
    //     Dispatcher d;
    //     d.attach_layering(l);
    //     NoteManager nm;
    //     nm.attach_dispatcher(d);
    //     ModeManager mm;
    //     mm.attach_layering(l);
    //     mm.attach_dispatcher(d);
    //     mm.attach_note_manager(nm);
    //     for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_modes(ci), NoteManager::Mode::Poly);
    //
    //     // message = Channel Mode, channel 0, number MONO_MODE_ON
    //     // expected to switch channel 0 to mono mode.
    //     d.dispatch_message(SmallMessage(0xB0, 126, 0));
    //     TS_ASSERT_EQUALS(nm.channel_modes(0), NoteManager::Mode::Mono);
    //     for (size_t ci = 1; ci < CHANNEL_COUNT; ci++)
    //         TS_ASSERT_EQUALS(nm.channel_modes(ci), NoteManager::Mode::Poly);
    //
    //
    //     // message = Channel Mode, channel 0, number POLY_MODE_ON
    //     // expected to switch channel 1 to poly mode.
    // }
    //
    // // Test receiving mono/poly messages while in multi mode.

};

// Voice mode_manager_unit_test::v;
