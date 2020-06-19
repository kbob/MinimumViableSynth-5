#include "dispatcher.h"

#include <sstream>

#include <cxxtest/TestSuite.h>

using midi::Dispatcher;
using midi::CHANNEL_COUNT;
using midi::StatusByte;
using midi::ControllerNumber;
using midi::ChannelModeNumber;
using midi::RPN;
using midi::NRPN;
using midi::SmallMessage;
using midi::SysexMessage;
using midi::ParameterNumber;
using midi::ParameterValue;

class dispatcher_unit_test : public CxxTest::TestSuite {

public:

    SmallMessage last;
    SysexMessage last_sysex;

    class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator () () { return ss.str(); }
    } log;

    void log_small(const SmallMessage& msg)
    {
        last = msg;
        auto f = log.ss.flags();
        log.ss << "["
               << std::hex << std::uppercase
               << unsigned(msg.status_byte);
        if (msg.data_byte_1 != SmallMessage::NO_DATA) {
           log.ss << ' '
                  << unsigned(msg.data_byte_1);
           if (msg.data_byte_2 != SmallMessage::NO_DATA)
               log.ss << ' '
                      << unsigned(msg.data_byte_2);
        }
        log.ss << ']';
        log.ss.flags(f);
    }

    void log_sysex(const SysexMessage& msg)
    {
        last_sysex = msg;
        auto f = log.ss.flags();
        log.ss << std::hex << std::uppercase;
        log.ss << '<';
        for (size_t i = 0; i < msg.size; i++) {
            if (i)
                log.ss << ' ';
            log.ss << unsigned(msg.data[i] & 0xFF);
        }
        log.ss << '>';
        log.ss.flags(f);
    }

    void log_xRPN(std::uint8_t channel,
                  const ParameterNumber& num,
                  const ParameterValue& val)
    {
        log.ss << '{'
               << unsigned(channel)
               << '/'
               << num.number()
               << ": "
               << val.value()
               << '}';
    }

    using small_handler = Dispatcher::small_handler;
    using sysex_handler = Dispatcher::sysex_handler;
    using  xRPN_handler = Dispatcher::xRPN_handler;
    using log_small_binding =
        small_handler::binding<dispatcher_unit_test,
                               &dispatcher_unit_test::log_small>;
    using log_sysex_binding =
        sysex_handler::binding<dispatcher_unit_test,
                               &dispatcher_unit_test::log_sysex>;
    using log_xRPN_binding =
        xRPN_handler::binding<dispatcher_unit_test,
                              &dispatcher_unit_test::log_xRPN>;
    small_handler small_logger;
    sysex_handler sysex_logger;
    xRPN_handler   xRPN_logger;

    dispatcher_unit_test()
    : small_logger{log_small_binding(this)},
      sysex_logger{log_sysex_binding(this)},
      xRPN_logger{log_xRPN_binding(this)}
    {}

    void test_instantiate()
    {
        (void)midi::Dispatcher();
        TS_TRACE("sizeof Dispatcher = " + std::to_string(sizeof (Dispatcher)));
    }

    void test_dispatch_channel_voice_message()
    {
        Dispatcher d;
        d.register_handler(StatusByte::NOTE_ON, 0x0001, small_logger);
        log.clear();
        d.dispatch_message(SmallMessage(0x90, 0x12, 0x34));
        TS_ASSERT_EQUALS(log(), "[90 12 34]");

        log.clear();
        d.dispatch_message(SmallMessage(0x91, 0x56, 0x78));
        TS_ASSERT_EQUALS(log(), "");
    }

    void test_dispatch_all_channels_voice_message()
    {
        Dispatcher d;
        d.register_handler(StatusByte::PROGRAM_CHANGE,
                           Dispatcher::ALL_CHANNELS,
                           small_logger);
        log.clear();
        d.dispatch_message(SmallMessage(0xC1, 0x23));
        d.dispatch_message(SmallMessage(0xCF, 0x76));
        TS_ASSERT_EQUALS(log(), "[C1 23][CF 76]");
    }

    void not_omni(Dispatcher& d)
    {
        for (size_t ti = 0; ti < MAX_TIMBRES && ti < CHANNEL_COUNT; ti++)
            d.set_timbre_channels(ti, 1 << ti);
    }

    void dispatch_cc(Dispatcher& d,
                     std::uint8_t channel,
                     ControllerNumber cc,
                     std::uint8_t value)
    {
        std::uint8_t s = std::uint8_t(StatusByte::CONTROL_CHANGE) | channel;
        std::uint8_t cn = std::uint8_t(cc);
        std::uint8_t cv = value;
        d.dispatch_message(SmallMessage(s, cn, cv));
    }

    void test_dispatch_cc_message()
    {
        Dispatcher d;
        not_omni(d);
        d.register_handler(ControllerNumber::GENERAL_PURPOSE_5,
                           Dispatcher::ALL_TIMBRES,
                           small_logger);
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            std::uint8_t s = std::uint8_t(StatusByte::CONTROL_CHANGE) | chan;
            std::uint8_t cn = std::uint8_t(ControllerNumber::GENERAL_PURPOSE_5);
            std::uint8_t cv = 0x10 + chan;
            std::ostringstream expected;
            if (chan < MAX_TIMBRALITY)
                expected << std::hex
                         << std::uppercase
                         << '['
                         << unsigned(s)
                         << ' '
                         << unsigned(cn)
                         << ' '
                         << unsigned(cv)
                         << ']';
            log.clear();
            dispatch_cc(d, chan, ControllerNumber::GENERAL_PURPOSE_5, cv);
            TS_ASSERT_EQUALS(log(), expected.str());
        }
    }

    void select_RPN(Dispatcher& d, std::uint8_t channel, RPN rpn)
    {
        ParameterNumber pn(rpn);

        // 1. Set RPN LSB.
        dispatch_cc(d, channel, ControllerNumber::RPN_LSB, pn.lsb());

        // 2. Set RPN MSB.
        dispatch_cc(d, channel, ControllerNumber::RPN_MSB, pn.msb());
    }

    void test_dispatch_RPN_message()
    {
        ParameterNumber pn(RPN::FINE_TUNING);
        ParameterValue pv(1234);
        Dispatcher d;
        not_omni(d);
        d.register_handler(RPN::FINE_TUNING,
                           Dispatcher::ALL_TIMBRES,
                           xRPN_logger);
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            log.clear();

            // 1.  Set RPN LSB.
            // 2.  Set RPN MSB.
            select_RPN(d, chan, RPN::FINE_TUNING);

            // 3.  Set data entry LSB.
            dispatch_cc(d, chan, ControllerNumber::DATA_ENTRY_LSB, pv.lsb());
            TS_ASSERT_EQUALS(log(), "");

            // 4.  Set data entry MSB.
            dispatch_cc(d, chan, ControllerNumber::DATA_ENTRY_MSB, pv.msb());

            // 5.  Increment data.
            dispatch_cc(d, chan, ControllerNumber::DATA_INCREMENT, 0);

            // 6.  Decrement data.
            dispatch_cc(d, chan, ControllerNumber::DATA_DECREMENT, 0);

            std::ostringstream expected;
            if (chan < MAX_TIMBRALITY)
                expected << "{" << chan << "/1: 1234}"
                         << "{" << chan << "/1: 1235}"
                         << "{" << chan << "/1: 1234}";
                // expected = "{0/1: 1234}{0/1: 1235}{0/1: 1234}";
            TS_ASSERT_EQUALS(log(), expected.str());
        }
    }

    void test_RPN_inc_dec_modes()
    {
        Dispatcher d;
        not_omni(d);

        // FINE_TUNING tested above, increments whole value.

        // COARSE_TUNING increments MSB only.
        d.register_handler(RPN::COARSE_TUNING,
                           0x0001,
                           xRPN_logger);
        log.clear();
        select_RPN(d, 0, RPN::COARSE_TUNING);
        ParameterValue pv(0x12, 0x34);
        dispatch_cc(d, 0, ControllerNumber::DATA_ENTRY_LSB, pv.lsb());
        dispatch_cc(d, 0, ControllerNumber::DATA_ENTRY_MSB, pv.msb());
        dispatch_cc(d, 0, ControllerNumber::DATA_INCREMENT, 0);
        dispatch_cc(d, 0, ControllerNumber::DATA_DECREMENT, 0);
        TS_ASSERT_EQUALS(log(), "{0/2: 2356}{0/2: 2484}{0/2: 2356}");

        // PITCH_BEND_SENSITIVITY increments centesimally.
        d.register_handler(RPN::PITCH_BEND_SENSITIVITY,
                           0x0001,
                           xRPN_logger);
        log.clear();
        select_RPN(d, 0, RPN::PITCH_BEND_SENSITIVITY);
        pv = ParameterValue(3, 99);
        dispatch_cc(d, 0, ControllerNumber::DATA_ENTRY_LSB, pv.lsb());
        dispatch_cc(d, 0, ControllerNumber::DATA_ENTRY_MSB, pv.msb());
        dispatch_cc(d, 0, ControllerNumber::DATA_INCREMENT, 0);
        dispatch_cc(d, 0, ControllerNumber::DATA_INCREMENT, 0);
        dispatch_cc(d, 0, ControllerNumber::DATA_DECREMENT, 0);
        dispatch_cc(d, 0, ControllerNumber::DATA_DECREMENT, 0);
        TS_ASSERT_EQUALS(log(),
                         "{0/0: 483}{0/0: 512}{0/0: 513}{0/0: 512}{0/0: 483}");
    }

    void test_dispatch_NRPN_message()
    {
        ParameterNumber pn(0x1234);
        ParameterValue pv(4321);
        Dispatcher d;
        not_omni(d);
        d.register_handler(NRPN(0x1234),
                           Dispatcher::ALL_TIMBRES,
                           xRPN_logger);
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            log.clear();

            // 1.  Set RPN LSB.
            dispatch_cc(d, chan, ControllerNumber::NRPN_LSB, pn.lsb());

            // 2.  Set RPN MSB.
            dispatch_cc(d, chan, ControllerNumber::NRPN_MSB, pn.msb());

            // 3.  Set data entry LSB.
            dispatch_cc(d, chan, ControllerNumber::DATA_ENTRY_LSB, pv.lsb());
            TS_ASSERT_EQUALS(log(), "");

            // 4.  Set data entry MSB.
            dispatch_cc(d, chan, ControllerNumber::DATA_ENTRY_MSB, pv.msb());

            // 5.  Increment data.
            dispatch_cc(d, chan, ControllerNumber::DATA_INCREMENT, 0);

            // 6.  Decrement data.
            dispatch_cc(d, chan, ControllerNumber::DATA_DECREMENT, 0);

            std::ostringstream expected;
            if (chan < MAX_TIMBRALITY) {
                expected << "{" << unsigned(chan) << "/4660: 4321}";
                expected << "{" << unsigned(chan) << "/4660: 4322}";
                expected << "{" << unsigned(chan) << "/4660: 4321}";
            }
            TS_ASSERT_EQUALS(log(), expected.str());
        }
    }

    void test_dispatch_channel_mode_message()
    {
        Dispatcher d;
        not_omni(d);
        d.register_handler(ChannelModeNumber::ALL_SOUND_OFF,
                           Dispatcher::ALL_TIMBRES,
                           small_logger);
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            std::uint8_t s =
                std::uint8_t(StatusByte::SELECT_CHANNEL_MODE) | chan;
            std::uint8_t cn = std::uint8_t(ChannelModeNumber::ALL_SOUND_OFF);
            std::uint8_t cv = 0;
            std::ostringstream expected;
            if (chan < MAX_TIMBRALITY)
                expected << std::hex
                         << std::uppercase
                         << '['
                         << unsigned(s)
                         << ' '
                         << unsigned(cn)
                         << ' '
                         << unsigned(cv)
                         << ']';
            log.clear();
            d.dispatch_message(SmallMessage(s, cn, cv));
            TS_ASSERT_EQUALS(log(), expected.str());
        }
    }

    void test_dispatch_system_common_message()
    {
        Dispatcher d;
        d.register_handler(StatusByte::SONG_SELECT, small_logger);
        std::uint8_t s = std::uint8_t(StatusByte::SONG_SELECT);
        std::uint8_t d1 = 0x12;

        log.clear();
        d.dispatch_message(SmallMessage(s, d1));
        TS_ASSERT_EQUALS(log(), "[F3 12]");
    }

    void test_dispatch_system_real_time_message()
    {
        Dispatcher d;
        d.register_handler(StatusByte::STOP, small_logger);

        log.clear();
        d.dispatch_message(SmallMessage(std::uint8_t(StatusByte::STOP)));
        TS_ASSERT_EQUALS(log(), "[FC]");
    }

    // void test_dispatch_sysex_message()
    // {
    //     //Dispatcher d;
    //     //d.register_handler(SysexID::NON_COMMERCIAL, log_sysex);
    //     SysexMessage msg;
    //     msg.append(StatusByte::SYSTEM_EXCLUSIVE);
    //     msg.append(SysexID::NON_COMMERCIAL);
    //     msg.append(SysexDeviceID::ALL_CALL);
    //     msg.append(1);
    //     msg.append(StatusByte::EOX);
    //
    //     log.clear();
    //     log_sysex(msg);
    //     //d.dispatch_message(msg);
    //     TS_TRACE(log());
    // }

};
