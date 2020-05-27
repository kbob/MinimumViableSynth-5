#include "parser.h"

#include <string>
#include <sstream>

#include <cxxtest/TestSuite.h>

using midi::Parser;
using midi::SmallMessage;
using midi::SysexMessage;
using midi::StatusByte;
using midi::SysexID;
using midi::SysexDeviceID;

class parser_unit_test : public CxxTest::TestSuite {

public:

    static SmallMessage last;
    static SysexMessage last_sysex;

    static class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator () () { return ss.str(); }
    } log;

    static void log_msg(const SmallMessage& msg)
    {
        last = msg;
        auto f = log.ss.flags();
        log.ss << '['
               << std::hex << std::uppercase
               << "0x" << unsigned(msg.status_byte);
        if (msg.data_byte_1 != SmallMessage::NO_DATA) {
            log.ss << ' '
                   << std::dec
                   << unsigned(msg.data_byte_1);
            if (msg.data_byte_2 != SmallMessage::NO_DATA)
                log.ss << ' '
                       << unsigned(msg.data_byte_2);
        }
        log.ss << ']';
        log.ss.flags(f);
    }

    static void log_sysex_msg(const SysexMessage& msg)
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

    void test_instantiate()
    {
        (void)midi::Parser();
    }

    void test_logging()
    {
        log.clear();
        log.ss << "foo " << 42;
        TS_ASSERT_EQUALS(log.ss.str(), "foo 42");
        TS_ASSERT_EQUALS(log(), "foo 42");
    }

    void test_process_message()
    {
        Parser p;
        p.register_handler(log_msg);

        // one-byte messages, both system common and system real time
        log.clear();
        p.process_message("\xF6", 1);   // Tune Request
        p.process_message("\xFA", 1);   // Start
        TS_ASSERT_EQUALS(log(), "[0xF6][0xFA]");

        // two-byte messages, both channel and system
        log.clear();
        p.process_message("\xC4\5", 2); // Program Change, channel 4, program 2
        p.process_message("\xF3\6", 2); // Song Select, song 6
        TS_ASSERT_EQUALS(log(), "[0xC4 5][0xF3 6]");

        // three-byte messages, both channel and system
        log.clear();
        uint16_t pos = 1234;
        char pos_msg[3] = {char(0xF2)};
        pos_msg[1] = pos & 0x7F;
        pos_msg[2] = pos >> 7;
        p.process_message("\x96\7\10", 3);  // Note On, channel 6, note 7 vel. 8
        p.process_message(pos_msg, 3);      // Song Position, pos. 1234
        TS_ASSERT_EQUALS(log(), "[0x96 7 8][0xF2 82 9]");
    }

    void test_malformed_messages()
    {
        Parser p;
        p.register_handler(log_msg);

        log.clear();

        // no status byte
        TS_ASSERT_THROWS(p.process_message("123", 3), std::runtime_error);
        // 1 byte message w/ 3 byte channel msg status
        TS_ASSERT_THROWS(p.process_message("\x80", 1), std::runtime_error);
        // 1 byte message w/ sysex status
        TS_ASSERT_THROWS(p.process_message("\xF0", 1), std::runtime_error);
        // 1 byte message w/ 2 byte status
        TS_ASSERT_THROWS(p.process_message("\xCC", 1), std::runtime_error);

        // 2 byte message w/ 3 byte status
        TS_ASSERT_THROWS(p.process_message("\x80\1", 2), std::runtime_error);
        // 2 byte message w/ two status bytes
        TS_ASSERT_THROWS(p.process_message("\xC0\x80", 2), std::runtime_error);

        // 3 byte message w/ 1 byte status
        TS_ASSERT_THROWS(p.process_message("\xFC\1\2", 3), std::runtime_error);
        // 3 byte message w/ two status bytes
        TS_ASSERT_THROWS(p.process_message("\x99\x88g", 3), std::runtime_error);
        TS_ASSERT_THROWS(p.process_message("\x99h\xAA", 3), std::runtime_error);

        TS_ASSERT_EQUALS(log(), "");
    }

    void test_process_sysex_message()
    {
        Parser p;
        p.register_handler(log_msg);
        p.register_handler(log_sysex_msg);

        unsigned char m[] = {0xF0, 0x7D, 0x7F, 42, 43, 0xF7};
        log.clear();
        p.process_message((const char *)m, 6);
        TS_ASSERT_EQUALS(log(), "<F0 7D 7F 2A 2B F7>");

        TS_ASSERT_EQUALS(last_sysex.size, 6);
        TS_ASSERT_EQUALS(last_sysex.id(), SysexID::NON_COMMERCIAL);
        TS_ASSERT_EQUALS(last_sysex.device_id(), SysexDeviceID::ALL_CALL);
        TS_ASSERT_EQUALS(last_sysex.payload_size(), 2);
        TS_ASSERT_EQUALS(last_sysex.payload()[0], 42);
        TS_ASSERT_EQUALS(last_sysex.payload()[1], 43);
    }

    void test_process_bytes()
    {
        struct mcase {
            //const char *in;
            const std::uint8_t in[8];
            const char *out;
        };
        static const mcase cases[] = {

            // two two-byte channel messages w/ running status
            { { 0xC1, 1, 2 },                   "[0xC1 1][0xC1 2]" },

            // three three-byte channel messages w/ running status
            { { 0x82, 3, 4, 5, 6, 7, 8 },    "[0x82 3 4][0x82 5 6][0x82 7 8]" },

            // one-byte system common messages
            { { 1, 0xF6, 2, 3, 0xF6, 0xF6, 4 }, "[0xF6][0xF6][0xF6]" },

            // two-byte system common message, no running status
            { { 0xF1, 1, 2 },                   "[0xF1 1]" },

            // three-byte system common message, no running status
            { { 0xF2, 1, 2, 3, 4 },             "[0xF2 1 2]" },

            // three-byte channel message w/ real time messages
            { { 0x93, 0xF8, 1, 0xFA, 2, 0xFB }, "[0xF8][0xFA][0x93 1 2][0xFB]"},

            // three-byte channel message w/ undefined real time messages
            { { 0x93, 0xF9, 1, 0xFD, 2, 0xF9 }, "[0x93 1 2]" },

            // three-byte channel message interrupted by system common message
            { { 0x93, 0xF6, 1, 2, 3, 4 },       "[0xF6]" },

            // sysex message
            { { 0xF0, 1, 2, 3, 4, 5, 0xF7 },    "<F0 1 2 3 4 5 F7>" },

            // unterminated sysex
            { { 0xF0, 1, 2, 0xF1, 3 },          "[0xF1 3]" },

            // unmatched EOX
            { { 0x80, 1, 2, 0xF7, 3, 0xc4, 5 }, "[0x80 1 2][0xC4 5]" },

        };
        static const size_t case_count = (&cases)[1] - cases;

        Parser p;
        p.register_handler(log_msg);
        p.register_handler(log_sysex_msg);
        for (size_t i = 0; i < case_count; i++) {
            const mcase& c = cases[i];
            const char *in = reinterpret_cast<const char *>(&c.in[0]);
            log.clear();
            p.process_bytes(in, strlen(in));
            TS_ASSERT_EQUALS(log(), c.out);
        }
    }

    void test_reset()
    {
        Parser p;
        p.register_handler(log_msg);

        log.clear();
        p.process_bytes("\x90\x3c\x40\1", 4);
        p.reset();
        p.process_bytes("\2\x80\x3c\0", 4);
        TS_ASSERT_EQUALS(log(), "[0x90 60 64][0x80 60 0]")
    }

    void test_process_oversize_sysex()
    {
        Parser p;
        p.register_handler(log_msg);
        p.register_handler(log_sysex_msg);
        log.clear();
        p.process_byte('\xF0');
        for (size_t i = 0; i < midi::MAX_SYSEX_SIZE - 1; i++)
            p.process_byte(i & 0x7F);
        p.process_bytes("\xF7\1\x83\2\3", 5);
        TS_ASSERT_EQUALS(log(), "[0x83 2 3]");
    }

    void test_fuzz()
    {
        // succeeds if it doesn't crash.
        Parser p;
        srandom(123);
        for (int i = 0; i < 1000000; i++)
            p.process_byte((char)random());
    }

};

parser_unit_test::Logger parser_unit_test::log;
SmallMessage parser_unit_test::last;
SysexMessage parser_unit_test::last_sysex;
