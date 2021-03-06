#include "note-mgr.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/asgn-prio.h"

using midi::ControllerNumber;
using midi::Dispatcher;
using midi::Layering;
using midi::NoteManager;
using midi::SmallMessage;
using midi::StatusByte;

class note_manager_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::NoteManager();
        // TS_TRACE("sizeof NoteManager = " +
        //          std::to_string(sizeof (midi::NoteManager)));
    }

    static class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator () () { return ss.str(); }
    } log;

    class FooControl : public ::ControlType<FooControl> {
    public:
        void render(size_t) {}
        void start_note()         override { log.ss << "s "; }
        void release_note()       override { log.ss << "r "; }
        void kill_note()          override { log.ss << "k "; }
        void idle()               override { log.ss << "i "; }
        bool note_is_done() const override { log.ss << "d "; return true; }
    };

    static int prioritize(const Voice&) { return 0; }

    class pile_of_stuff {
    public:
        ::Synth s;
        ::PriorityAssigner a;
        FooControl c;
        Dispatcher d;
        Layering l;
        NoteManager m;

        pile_of_stuff(size_t polyphony, size_t timbrality)
        : s{"Foo", polyphony, timbrality},
          a{s, ::PriorityAssigner::prioritizer(prioritize)},
          l{timbrality}
        {
            ::Config cfg;
            cfg.set_sample_rate(44100);
            s.add_voice_control(c, true)
             .finalize(cfg);
            d.attach_layering(l);

            m.attach_synth(s);
            m.attach_assigner(a);
            m.attach_dispatcher(d);

            for (size_t vi = 0; vi < polyphony; vi++) {
                m.register_portamento_note_handler(
                    vi,
                    NoteManager::portamento_note_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_portamento_note
                    >(this));
                m.register_note_number_handler(
                    vi,
                    NoteManager::note_number_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_note_number
                    >(this));
                m.register_attack_velocity_handler(
                    vi,
                    NoteManager::attack_velocity_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_attack_velocity
                    >(this));
                m.register_poly_pressure_handler(
                    vi,
                    NoteManager::poly_pressure_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_poly_pressure
                    >(this));
                m.register_release_velocity_handler(
                    vi,
                    NoteManager::release_velocity_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_release_velocity
                    >(this));
            }
        }

        void log_portamento_note(std::uint8_t pn)
        {
            log.ss << 'P' << unsigned(pn) << ' ';
        }

        void log_note_number(std::uint8_t note)
        {
            log.ss << 'N' << unsigned(note) << ' ';
        }

        void log_attack_velocity(std::uint16_t vel)
        {
            log.ss << 'A' << vel << ' ';
        }

        void log_poly_pressure(std::uint8_t pressure)
        {
            // 'K' for Key pressure
            log.ss << 'K' << unsigned(pressure) << ' ';
        }

        void log_release_velocity(std::uint8_t vel)
        {
            log.ss << 'R' << unsigned(vel) << ' ';
        }

    };

    // test Reset All Controllers message.

    enum class Reset {
        NONE,
        OMNI,
        CHANNEL,
    };

    void do_reset_test(Reset reset,
                       NoteManager::Mode mode,
                       const std::string& expected)
    {
        size_t POLY = 3, TIMB = 2;
        int CHAN = 1;
        pile_of_stuff pos(POLY, TIMB);
        pos.l.multi_mode();
        pos.m.channel_mode(CHAN, mode);

        log.clear();

        // We're going to hold C4 with sostenuto, sustain, and polyphonic
        // key pressure, G4 with sustain, and set up prefixes for
        // portamento note and high resolution velocity.
        // Then we reset (or not).
        // Then we turn B4 on and off, and see what happens.

        // damper pedal on
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 64, 127));
        // note on C4 vel. 1
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // poly pressure C4 = 12
        pos.d.dispatch_message(SmallMessage(0xA0 | CHAN, 60, 12));
        // sostenuto on
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 66, 127));
        // note on G4 vel. 2
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 67, 2));
        // portamento note = C5
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 84, 72));
        // high res velocity = 11
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 88, 11));
        // note off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // note off G4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 67, 31));

        log.ss << "| ";
        switch (reset) {

        case Reset::NONE:
            break;

        case Reset::OMNI:
            pos.m.reset_all_controllers();
            break;

        case Reset::CHANNEL:
            pos.m.reset_all_controllers(CHAN);
            break;
        }
        log.ss << "| ";

        // note on B4 velocity = 3
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 71, 3));
        // note off B4 velocity = 30
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 71, 30));


        TS_ASSERT_EQUALS(log(), expected);
    }

    void test_no_reset_poly()
    {
        do_reset_test(Reset::NONE,
                      NoteManager::Mode::POLY,
                      "N60 A129 s K12 N67 A258 s | | N71 A395 P72 s ");
    }

    void test_no_reset_mono()
    {
        do_reset_test(Reset::NONE,
                      NoteManager::Mode::MONO,
                      "N60 A129 s K12 N67 P60 | | N71 P72 ");
    }

    void test_omni_reset_poly()
    {
        do_reset_test(Reset::OMNI,
                      NoteManager::Mode::POLY,
                      "N60 A129 s K12 N67 A258 s "
                      "| K0 K0 K0 R0 r R0 r | "
                      "N71 A387 s R30 r ");
    }

    void test_omni_reset_mono()
    {
        do_reset_test(Reset::OMNI,
                      NoteManager::Mode::MONO,
                      "N60 A129 s K12 N67 P60 "
                      "| K0 K0 K0 R0 r | "
                      "N71 A387 P67 s R30 r ");
    }

    void test_channel_reset_poly()
    {
        do_reset_test(Reset::CHANNEL,
                      NoteManager::Mode::POLY,
                      "N60 A129 s K12 N67 A258 s "
                      "| K0 K0 R0 r R0 r | "
                      "N71 A387 s R30 r ");
    }

    void test_channel_reset_mono()
    {
        do_reset_test(Reset::CHANNEL,
                      NoteManager::Mode::MONO,
                      "N60 A129 s K12 N67 P60 "
                      "| K0 R0 r | "
                      "N71 A387 P67 s R30 r ");
    }

    void test_high_res_velocity()
    {
        // note on C4 -
        //  velocity = xx.xx
        // high res velocity 100
        // note on D4 -
        //  velocity - xx.100
        // note on E4 -
        //  velocity - xx.xx

        size_t POLY = 3, TIMB = 1, CHAN = 0;
        pile_of_stuff pos(POLY, TIMB);

        log.clear();
        // Note On C4, velocity 1
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // High Resolution Velocity = 100
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 88, 100));
        // Note On D4, velocity 2
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note On E4, velocity 3
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 64, 3));

        TS_ASSERT_EQUALS(log(), "N60 A129 s N62 A356 s N64 A387 s ");
    }

    void do_sound_off_test(Reset reset,
                           NoteManager::Mode mode,
                           const std::string& expected)
    {
        // have a sounding note  - C4
        // have a releasing note - D4
        // have a stopping note  - E4

        size_t POLY = 3, TIMB = 1, CHAN = 6;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, mode);

        log.clear();
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note On E4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 64, 3));
        // Note On F4 - kills E4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 65, 4));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));

        // all sound off
        log.ss << "| ";
        switch (reset) {

        case Reset::NONE:
            break;

        case Reset::OMNI:
            pos.m.all_sound_off();
            break;

        case Reset::CHANNEL:
            pos.m.all_sound_off(CHAN);
            break;
        }
        log.ss << "| ";

        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note Off E4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 64, 30));
        // Note Off E5
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 65, 29));

        TS_ASSERT_EQUALS(log(), expected);
        if (reset != Reset::NONE)
            TS_ASSERT(pos.m.m_pending_notes.empty());
    }

    void test_no_sound_off_poly()
    {
        do_sound_off_test(Reset::NONE,
                          NoteManager::Mode::POLY,
                          "N60 A129 s N62 A258 s N64 A387 s k R31 r "
                          "| | "
                          "R30 r ");
    }

    void test_no_sound_off_mono()
    {
        do_sound_off_test(Reset::NONE,
                          NoteManager::Mode::MONO,
                          "N60 A129 s N62 P60 N64 P62 N65 P64 "
                          "| | "
                          "R29 r ");
    }

    void test_omni_sound_off_poly()
    {
        do_sound_off_test(Reset::OMNI,
                          NoteManager::Mode::POLY,
                          "N60 A129 s N62 A258 s N64 A387 s k R31 r "
                          "| k k | "
                          "");
    }

    void test_omni_sound_off_mono()
    {
        do_sound_off_test(Reset::OMNI,
                          NoteManager::Mode::MONO,
                          "N60 A129 s N62 P60 N64 P62 N65 P64 "
                          "| k | "
                          "");
    }

    void test_channel_sound_off_poly()
    {
        do_sound_off_test(Reset::CHANNEL,
                          NoteManager::Mode::POLY,
                          "N60 A129 s N62 A258 s N64 A387 s k R31 r "
                          "| k k | "
                          "");
    }

    void test_channel_sound_off_mono()
    {
        do_sound_off_test(Reset::CHANNEL,
                          NoteManager::Mode::MONO,
                          "N60 A129 s N62 P60 N64 P62 N65 P64 "
                          "| k | "
                          "");
    }

    void do_note_off_test(Reset reset,
                          NoteManager::Mode mode,
                          const std::string& expected)
    {
        // have a sostenuto note  - C4
        // have a sustaining note - D4
        // have a note on         - E4

        size_t POLY = 3, TIMB = 1, CHAN = 8;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, mode);

        log.clear();
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Sostenuto On
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 66, 127));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Damper Pedal On
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 64, 127));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));
        // Note On E4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 64, 3));

        // All Sound off
        log.ss << "| ";
        switch (reset) {

        case Reset::NONE:
            break;

        case Reset::OMNI:
            pos.m.all_notes_off();
            break;

        case Reset::CHANNEL:
            pos.m.all_notes_off(CHAN);
            break;
        }
        log.ss << "| ";

        // Damper Pedal Off
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 64, 0));
        // Sostenuto Off
        pos.d.dispatch_message(SmallMessage(0xB0 | CHAN, 66, 0));
        // Note Off E4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 64, 30));

        // TS_TRACE(log());
        TS_ASSERT_EQUALS(log(), expected);
        if (reset != Reset::NONE)
            TS_ASSERT(pos.m.m_pending_notes.empty());
    }

    void test_no_notes_off_poly()
    {
        do_note_off_test(Reset::NONE,
                         NoteManager::Mode::POLY,
                         "N60 A129 s N62 A258 s N64 A387 s "
                         "| | "
                         "R0 r R0 r R30 r ");
    }

    void test_no_notes_off_mono()
    {
        do_note_off_test(Reset::NONE,
                         NoteManager::Mode::MONO,
                         "N60 A129 s N62 P60 N64 P62 "
                         "| | "
                         "R30 r ");
    }

    void test_omni_notes_off_poly()
    {
        do_note_off_test(Reset::OMNI,
                         NoteManager::Mode::POLY,
                         "N60 A129 s N62 A258 s N64 A387 s "
                         "| | "
                         "R0 r R0 r R0 r ");
    }

    void test_omni_notes_off_mono()
    {
        do_note_off_test(Reset::OMNI,
                         NoteManager::Mode::MONO,
                         "N60 A129 s N62 P60 N64 P62 "
                         "| | "
                         "N60 P64 R0 r ");
    }

    void test_channel_notes_off_poly()
    {
        do_note_off_test(Reset::CHANNEL,
                         NoteManager::Mode::POLY,
                         "N60 A129 s N62 A258 s N64 A387 s "
                         "| | "
                         "R0 r R0 r R0 r ");
    }

    void test_channel_notes_off_mono()
    {
        do_note_off_test(Reset::CHANNEL,
                         NoteManager::Mode::MONO,
                         "N60 A129 s N62 P60 N64 P62 "
                         "| | "
                         "N60 P64 R0 r ");
    }

    void test_mono_legato()
    {
        size_t POLY = 1, TIMB = 1, CHAN = 7;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, NoteManager::Mode::MONO);

        log.clear();
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));

        TS_ASSERT_EQUALS(log(), "N60 A129 s N62 P60 R31 r ");

        log.clear();
        pos.m.channel_legato(CHAN, false);
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));

        TS_ASSERT_EQUALS(log(), "N60 A129 P62 s N62 A258 P60 R31 r ");
    }

    void test_mono_off_on()
    {
        size_t POLY = 1, TIMB = 1, CHAN = 7;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, NoteManager::Mode::MONO);

        log.clear();
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));

        TS_ASSERT_EQUALS(log(), "N60 A129 s R32 r N62 A258 P60 s R31 r ");
    }

    void test_mono_extra_note_off()
    {
        size_t POLY = 1, TIMB = 1, CHAN = 7;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, NoteManager::Mode::MONO);

        log.clear();
        // Note On C4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 1));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note On D4
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 2));
        // Note Off C4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // Note Off D4
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));

        TS_ASSERT_EQUALS(log(), "N60 A129 s R32 r N62 A258 P60 s R31 r ");
    }

    // test note on message followed by note off message.
    // have to build a synth, assigner, layering, and dispatcher.
    // poly = 1 , timb = 1.

    void test_a_note()
    {
        size_t POLY = 1, TIMB = 1;
        pile_of_stuff pos(POLY, TIMB);
        pos.l.multi_mode();

        log.clear();
        pos.d.dispatch_message(SmallMessage(0x90, 60, 64));
        TS_ASSERT_EQUALS(pos.m.m_pending_notes.size(), 0);
        TS_ASSERT_EQUALS(pos.m.m_killed_voices.size(), 0);

        pos.d.dispatch_message(SmallMessage(0x80, 60, 32));
        pos.s.voices()[0].render(1);
        pos.m.render(1);

        TS_ASSERT_EQUALS(log(), "N60 A8256 s R32 r d i ");
    }

    // test two note on messages followed by two notes off
    // on a monosynth.

    void test_two_notes()
    {
        size_t POLY = 1, TIMB = 1;
        pile_of_stuff pos(POLY, TIMB);

        log.clear();
        pos.d.dispatch_message(SmallMessage(0x90, 60, 64));
        pos.d.dispatch_message(SmallMessage(0x90, 62, 63));

        TS_ASSERT_EQUALS(pos.m.m_pending_notes.size(), 1);
        TS_ASSERT_EQUALS(pos.m.m_killed_voices.size(), 1);

        pos.d.dispatch_message(SmallMessage(0x80, 62, 31));
        pos.d.dispatch_message(SmallMessage(0x80, 60, 32));
        TS_ASSERT_EQUALS(log(), "N60 A8256 s k ");
    }

    // test note on/off on a layered synth.

    void test_layered_note()
    {
        size_t POLY = 2, TIMB = 2, CHAN = 5;
        pile_of_stuff pos(POLY, TIMB);
        pos.l.multi_mode();
        pos.l.channel_timbres(CHAN, 0b11);

        log.clear();
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 64));
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        TS_ASSERT_EQUALS(log(), "N60 A8256 s N60 A8256 s R32 r R32 r ");
    }

    // test mono mode.  C on, D on, C off, D off.

    void test_mono_notes()
    {
        size_t POLY = 2, TIMB = 2, CHAN = 3;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, NoteManager::Mode::MONO);

        log.clear();

        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 64));
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 63));
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));
        // TS_TRACE(log());

        TS_ASSERT_EQUALS(log(), "N60 A8256 s N62 P60 R31 r ");
    }

    // test mono mode.  C on, D on, D off, C off.

    void test_mono_resume()
    {
        size_t POLY = 2, TIMB = 2, CHAN = 3;
        pile_of_stuff pos(POLY, TIMB);
        pos.m.channel_mode(CHAN, NoteManager::Mode::MONO);

        log.clear();

        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 60, 64));
        pos.d.dispatch_message(SmallMessage(0x90 | CHAN, 62, 63));
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 62, 31));
        pos.d.dispatch_message(SmallMessage(0x80 | CHAN, 60, 32));
        // TS_TRACE(log());

        TS_ASSERT_EQUALS(log(), "N60 A8256 s N62 P60 N60 P62 R32 r ");
    }

    // mode-independent tests
    //  reset all controllers
    //  high res velocity
    //  all sound off

    // mono tests:
    //  with and without legato enabled
    //  note off followed by note on (same or different note)
    //  extraneous note off message
    //  all notes off
    //  sustain, sostenuto
    //  poly key pressure messages
    //  portamento controller

    // poly tests:
    //  note on and note off in different orders
    //  run out of voices
    //  run out of pending voices
    //  high res velocity
    //  portamento controller
    //  multiple note on message for same note
    //  extraneous note off messages
    //  all notes off
    //  sustain, sostenuto
    //  poly key pressure messages

};

note_manager_unit_test::Logger note_manager_unit_test::log;
