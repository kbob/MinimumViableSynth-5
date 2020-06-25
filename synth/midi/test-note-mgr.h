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
        TS_TRACE("sizeof NoteManager = " +
                 std::to_string(sizeof (midi::NoteManager)));
    }

    static int prioritize(const Voice&) { return 0; }

    // test note on message followed by note off message.
    // have to build a synth, layering, and dispatcher.
    // poly = 1 , timb = 1.

    void test_a_note()
    {
        size_t POLY = 1, TIMB = 1;
        ::Synth s("Foo", POLY, TIMB);
        ::Config cfg;
        cfg.set_sample_rate(44100);
        ::PriorityAssigner::prioritizer prioritizer(prioritize);
        ::PriorityAssigner a(s, prioritizer);
        Dispatcher d;
        Layering l(TIMB);
        l.multi_mode();
        d.attach_layering(l);
        NoteManager m;
        s.finalize(cfg);
        m.attach_synth(&s);
        m.attach_assigner(&a);
        m.attach_dispatcher(&d);

        d.dispatch_message(SmallMessage(0x90, 60, 64));


        TS_ASSERT_EQUALS(m.m_pending_notes.size(), 0);
        TS_ASSERT_EQUALS(m.m_killed_voices.size(), 0);
    }

    // test two note on messages followed by two notes off
    // on a monosynth.

    void test_two_notes()
    {
        size_t POLY = 1, TIMB = 1;
        ::Synth s("Foo", POLY, TIMB);
        ::Config cfg;
        cfg.set_sample_rate(44100);
        ::PriorityAssigner::prioritizer prioritizer(prioritize);
        ::PriorityAssigner a(s, prioritizer);
        Dispatcher d;
        Layering l(TIMB);
        d.attach_layering(l);
        NoteManager m;
        s.finalize(cfg);
        m.attach_synth(&s);
        m.attach_assigner(&a);
        m.attach_dispatcher(&d);

        d.dispatch_message(SmallMessage(0x90, 60, 64));
        d.dispatch_message(SmallMessage(0x90, 62, 63));

        TS_ASSERT_EQUALS(m.m_pending_notes.size(), 1);
        TS_ASSERT_EQUALS(m.m_killed_voices.size(), 1);
    }

    // test note on/off on a layered synth.




};
