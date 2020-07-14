#include "timbre-mgr.h"

#include <cxxtest/TestSuite.h>

using midi::CHANNEL_COUNT;
using midi::Dispatcher;
using midi::Layering;
using midi::SmallMessage;
using midi::StatusByte;
using midi::TimbreManager;

class timbre_manager_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::TimbreManager();
    }

    // set up a layering
    // dispatch some messages
    // verify handlers are called.

    // channel pressure messages

    static class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator () () { return ss.str(); }
    } log;

    class pile_of_stuff {
    public:
        Layering l;
        Dispatcher d;
        TimbreManager m;

        pile_of_stuff(size_t timbrality,
                      Layering::channel_mask c_mask,
                      TimbreManager::timbre_mask t_mask)
        : l{timbrality}
        {
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                l.channel_timbres(ci, (c_mask & (1 << ci)) ? t_mask : 0);
            d.attach_layering(l);
            m.attach_dispatcher(d);
            m.register_channel_pressure_handler(
                t_mask,
                TimbreManager::channel_pressure_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_channel_pressure
                    >(this)
            );
            m.register_pitch_bend_handler(
                t_mask,
                TimbreManager::pitch_bend_handler::binding<
                        pile_of_stuff,
                        &pile_of_stuff::log_pitch_bend
                    >(this)
            );
        }

        void log_channel_pressure(TimbreManager::channel_pressure_type cp)
        {
            log.ss << 'c' << int(cp) << ' ';
        }

        void log_pitch_bend(TimbreManager::pitch_bend_type cp)
        {
            log.ss << 'b' << int(cp) << ' ';
        }

    };

    void test_channel_pressure()
    {
        const size_t TIMB = 3, CHAN_MASK = 0x1111, TIMB_MASK = 0x6;
        pile_of_stuff pos(TIMB, CHAN_MASK, TIMB_MASK);

        log.clear();
        for (size_t channel = 0; channel < CHANNEL_COUNT; channel++) {
            auto s = make_status_byte(StatusByte::CHANNEL_PRESSURE, channel);
            log.ss << '.';
            pos.d.dispatch_message(SmallMessage(s, 12));
        }
        // TS_TRACE(log());
        TS_ASSERT_EQUALS(log(),
                         ".c12 c12 ....c12 c12 ....c12 c12 ....c12 c12 ...");
    }

    void test_pitch_bend()
    {
        const size_t TIMB = 3, CHAN_MASK = 0x0011, TIMB_MASK = 0x6;
        pile_of_stuff pos(TIMB, CHAN_MASK, TIMB_MASK);

        log.clear();
        for (size_t channel = 0; channel < CHANNEL_COUNT; channel++) {
            auto s = make_status_byte(StatusByte::PITCH_BEND, channel);
            log.ss << '.';
            pos.d.dispatch_message(SmallMessage(s, 12, 34));
        }
        // TS_TRACE(log());
        TS_ASSERT_EQUALS(log(),
                         ".b-3828 b-3828 ....b-3828 b-3828 ...........");
    }

    void test_pitch_bend_remapped()
    {
        const size_t TIMB = 3, CHAN_MASK = 0x0011, TIMB_MASK = 0x6;
        pile_of_stuff pos(TIMB, CHAN_MASK, TIMB_MASK);
        pos.l.channel_timbres(5, pos.l.all_timbres);

        // Channel 5 is mapped to timbre 1, but T1 has no handler
        // installed.  So message to C5 will be dispatched to T0
        // and T2 only.

        log.clear();
        for (size_t channel = 0; channel < CHANNEL_COUNT; channel++) {
            auto s = make_status_byte(StatusByte::PITCH_BEND, channel);
            log.ss << '.';
            pos.d.dispatch_message(SmallMessage(s, 12, 34));
        }
        // TS_TRACE(log());
        TS_ASSERT_EQUALS(log(),
                         ".b-3828 b-3828 ..."
                         ".b-3828 b-3828 .b-3828 b-3828 .."
                         "...."
                         "....");
    }

};

timbre_manager_unit_test::Logger timbre_manager_unit_test::log;
