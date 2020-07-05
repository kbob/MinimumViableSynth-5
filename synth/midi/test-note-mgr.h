#include "note-mgr.h"

#include <cxxtest/TestSuite.h>

#include "synth/core/asgn-prio.h"
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
        size_t POLY = 2, TIMB = 2;
        int CHAN = 5;
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
