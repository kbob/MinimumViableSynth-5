#include "messages.h"

#include <cxxtest/TestSuite.h>

using midi::StatusByte;
using midi::SmallMessage;
using midi::SysexMessage;

class messages_unit_test : public CxxTest::TestSuite {

public:

    std::uint8_t U(StatusByte s)
    {
        return static_cast<std::uint8_t>(s);
    }

    StatusByte S(std::uint8_t u)
    {
        return static_cast<StatusByte>(u);
    }

    void test_instantiate()
    {
        (void)midi::SmallMessage();
        (void)midi::SysexMessage();
    }

    void test_default_bytes()
    {
        SmallMessage m;
        TS_ASSERT_EQUALS(m.status_byte, SmallMessage::NO_STATUS);
        TS_ASSERT_EQUALS(m.data_byte_1, SmallMessage::NO_DATA);
        TS_ASSERT_EQUALS(m.data_byte_2, SmallMessage::NO_DATA);
    }

    void test_one_byte()
    {
        SmallMessage m(U(StatusByte::START));
        TS_ASSERT_EQUALS(m.status_byte, U(StatusByte::START));
        TS_ASSERT_EQUALS(m.data_byte_1, SmallMessage::NO_DATA);
        TS_ASSERT_EQUALS(m.data_byte_2, SmallMessage::NO_DATA);
    }

    void test_two_bytes()
    {
        SmallMessage m(U(StatusByte::PROGRAM_CHANGE), 1);
        TS_ASSERT_EQUALS(m.status_byte, U(StatusByte::PROGRAM_CHANGE));
        TS_ASSERT_EQUALS(m.data_byte_1, 1);
        TS_ASSERT_EQUALS(m.data_byte_2, SmallMessage::NO_DATA);
    }

    void test_three_bytes()
    {
        SmallMessage m(U(StatusByte::NOTE_OFF), 2, 3);
        TS_ASSERT_EQUALS(m.status_byte, U(StatusByte::NOTE_OFF));
        TS_ASSERT_EQUALS(m.data_byte_1, 2);
        TS_ASSERT_EQUALS(m.data_byte_2, 3);
    }

    void test_clear()
    {
        SmallMessage m(U(StatusByte::NOTE_OFF) | 1, 4, 5);
        TS_ASSERT_EQUALS(m.status_byte, 0x81);
        TS_ASSERT_EQUALS(m.data_byte_1, 4);
        TS_ASSERT_EQUALS(m.data_byte_2, 5);
        m.clear();
        TS_ASSERT_EQUALS(m.status_byte, SmallMessage::NO_STATUS);
        TS_ASSERT_EQUALS(m.data_byte_1, SmallMessage::NO_DATA);
        TS_ASSERT_EQUALS(m.data_byte_2, SmallMessage::NO_DATA);

        m = SmallMessage(U(StatusByte::NOTE_OFF) | 2, 6, 7);
        TS_ASSERT_EQUALS(m.status_byte, 0x82);
        TS_ASSERT_EQUALS(m.data_byte_1, 6);
        TS_ASSERT_EQUALS(m.data_byte_2, 7);
        m.clear_data();
        TS_ASSERT_EQUALS(m.status_byte, 0x82);
        TS_ASSERT_EQUALS(m.data_byte_1, SmallMessage::NO_DATA);
        TS_ASSERT_EQUALS(m.data_byte_2, SmallMessage::NO_DATA);
    }

    void test_channel_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            TS_ASSERT_EQUALS(m.is_channel_message(), 0x80 <= i && i < 0xF0);
        }
    }

    void test_channel_voice_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            m.data_byte_1 = 0;
            bool icvm = 0x80 <= i && i < 0xF0;
            TS_ASSERT_EQUALS(m.is_channel_voice_message(), icvm);
            m.data_byte_1 = 127;
            if ((i & 0xF0) == 0xB0)
                icvm = false;
            TS_ASSERT_EQUALS(m.is_channel_voice_message(), icvm);
        }
    }

    void test_channel_mode_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            m.data_byte_1 = 0;
            TS_ASSERT_EQUALS(m.is_channel_mode_message(), false);
            m.data_byte_1 = 126;
            bool icmm = (i & 0xF0) == 0xB0;
            TS_ASSERT_EQUALS(m.is_channel_mode_message(), icmm);
        }
    }

    void test_system_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            bool ism = i >= 0xF0;
            TS_ASSERT_EQUALS(m.is_system_message(), ism);
        }
    }

    void test_sysex_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            TS_ASSERT_EQUALS(m.is_system_exclusive_message(), false);
        }
    }

    void test_system_common_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            bool iscm = i >= 0xF0 && i < 0xF8;
            TS_ASSERT_EQUALS(m.is_system_common_message(), iscm);
        }
    }

    void test_system_real_time_predicate()
    {
        SmallMessage m;
        for (int i = 0; i < 256; i++) {
            m.status_byte = i;
            bool isrtm = i >= 0xF8;
            TS_ASSERT_EQUALS(m.is_system_real_time_message(), isrtm);
        }
    }

    void test_status()
    {
        SmallMessage m;
        for (int i = 128; i < 256; i++) {
            int s = (i < 0xF0) ? (i & 0xF0) : i;
            m.status_byte = i;
            TS_ASSERT_EQUALS(m.status(), S(s));
        }
    }

    void test_channel()
    {
        SmallMessage m;
        for (int i = 0x80; i < 0xF0; i++) {
            int chan = i & 0xF;
            m.status_byte = i;
            TS_ASSERT_EQUALS(m.channel(), chan);
        }
    }

    void test_note_number()
    {
        SmallMessage m(U(StatusByte::NOTE_ON) | 3, 8, 9);
        TS_ASSERT_EQUALS(m.note_number(), 8);
    }

    void test_velocity()
    {
        SmallMessage m(U(StatusByte::NOTE_ON) | 4, 10, 11);
        TS_ASSERT_EQUALS(m.velocity(), 11);
    }

    void test_poly_pressure()
    {
        SmallMessage m(U(StatusByte::POLY_KEY_PRESSURE) | 5, 12, 13);
        TS_ASSERT_EQUALS(m.poly_pressure(), 13);
    }

    void test_control_number()
    {
        SmallMessage m(U(StatusByte::CONTROL_CHANGE) | 6, 14, 15);
        TS_ASSERT_EQUALS(m.control_number(), 14);

        m = SmallMessage(U(StatusByte::SELECT_CHANNEL_MODE), 120, 16);
        TS_ASSERT_EQUALS(m.control_number(), 120);
    }

    void test_control_value()
    {
        SmallMessage m(U(StatusByte::CONTROL_CHANGE) | 7, 17, 18);
        TS_ASSERT_EQUALS(m.control_value(), 18);

        m = SmallMessage(U(StatusByte::SELECT_CHANNEL_MODE), 121, 19);
        TS_ASSERT_EQUALS(m.control_value(), 19);
    }

    void test_program_number()
    {
        SmallMessage m(U(StatusByte::PROGRAM_CHANGE) | 8, 20);
        TS_ASSERT_EQUALS(m.program_number(), 20);
    }

    void test_channel_pressure()
    {
        SmallMessage m(U(StatusByte::CHANNEL_PRESSURE) | 9, 21);
        TS_ASSERT_EQUALS(m.channel_pressure(), 21);
    }

    void test_bend()
    {
        SmallMessage m(U(StatusByte::PITCH_BEND), 22, 23);
        int bend = (23 << 7 | 22) - 8192;
        TS_ASSERT_EQUALS(m.bend(), bend);
    }

    void test_song_position()
    {
        SmallMessage m(U(StatusByte::SONG_POSITION), 24, 25);
        int pos = 25 << 7 | 24;
        TS_ASSERT_EQUALS(m.song_position(), pos);
    }

    void test_song_number()
    {
        SmallMessage m(U(StatusByte::SONG_SELECT), 26);
        TS_ASSERT_EQUALS(m.song_number(), 26);
    }

};
