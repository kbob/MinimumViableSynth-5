#include "config.h"

#include <cxxtest/TestSuite.h>

using midi::Dispatcher;
using midi::Layering;
using midi::NoteManager;
using midi::TimbreManager;

class config_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        midi::Dispatcher d;
        midi::NoteManager nm;
        midi::TimbreManager tm;
        (void)midi::Config(d, nm, tm);
    }

    static class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator * () { return ss.str(); }
    } log;

    // class pile_of_stuff {
    // public:
    //     ::Config core_c;
    //     Layering l;
    //     Dispatcher d;
    //     NoteManager nm;
    //     TimbreManager tm;
    //     midi::Config c;
    //
    //     pile_of_stuff(size_t timb)
    //     : core_c{},
    //       l{timb},
    //       d{},
    //       nm{},
    //       tm{},
    //       c{d, nm, tm}
    //     {
    //         d.attach_layering(l);
    //         tm.attach_dispatcher(d);
    //         core_c.register_subsystem(c);
    //
    //         // also need ::Synth, ::Assigner, ::Voice, ::Timbre.
    //         // what we really need is for NoteManager and TimbreManager
    //         // to be mockable.
    //         //
    //         // something like this.
    //         //    NoteManager<mock> nm;
    //         //    TimbreManager<mock> tm;
    //         //    Voice<mock> v;
    //         //    midi::Config c(d, nm, tm);
    //         //
    //         //    c.begin_voice(v);
    //         //    c.register_foo_handler(v, log_a_foo);
    //         //    c.end_voice(v);
    //         //    d.send_message(...);
    //         //    TS_ASSERT_EQUALS(*log, "...");
    //     }
    // };

};

// config_unit_test::Logger config_unit_test::log;
