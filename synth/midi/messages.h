#ifndef MIDI_MESSAGES_included
#define MIDI_MESSAGES_included

#include <cassert>
#include <cstdint>

#include "synth/midi/defs.h"

namespace midi {

    struct SmallMessage {
        std::uint8_t status_byte;
        std::uint8_t data_byte_1;
        std::uint8_t data_byte_2;

        SmallMessage()
        : status_byte{0},
          data_byte_1{0xFF},
          data_byte_2{0xFF}
        {}

        SmallMessage(std::uint8_t s)
        : status_byte{s},
          data_byte_1{0xFF},
          data_byte_2{0xFF}
        {
            assert(s & 0x80);
        }

        SmallMessage(std::uint8_t s, std::uint8_t d1)
        : status_byte{s},
          data_byte_1{d1},
          data_byte_2{0xFF}
        {
            assert(s & 0x80);
            assert(!(d1 & 0x80));
        }

        SmallMessage(std::uint8_t s, std::uint8_t d1, std::uint8_t d2)
        : status_byte{s},
          data_byte_1{d1},
          data_byte_2{d2}
        {
            assert(s & 0x80);
            assert(!(d1 & 0x80));
            assert(!(d2 & 0x80));
        }

        // return high nybble of channel messages, whole status byte of
        // of system messages.
        StatusByte status() const
        {
            std::uint8_t s = status_byte;
            assert(s & 0x80);
            if ((s & 0xF0) != 0xF0)
                s &= 0xF0;
            return static_cast<StatusByte>(s);
        }

        uint8_t channel() const
        {
            assert(is_channel_message());
            return status_byte & 0x0F;
        }

        // It might be nice to implement these.
        bool is_channel_message() const;
        bool is_channel_voice_message() const;
        bool is_channel_mode_message() const;
        bool is_system_message() const;
        bool is_system_exclusive_message() const;
        bool is_system_common_message() const;
        bool is_system_real_time_message() const;

        std::uint8_t note() const
        {
            assert(status() == StatusByte::NOTE_OFF ||
                   status() == StatusByte::NOTE_ON ||
                   status() == StatusByte::POLY_KEY_PRESSURE);
            return data_byte_1;
        }

        std::uint8_t velocity() const
        {
            assert(status() == StatusByte::NOTE_OFF ||
                   status() == StatusByte::NOTE_ON);
            return data_byte_2;
        }

        std::uint8_t poly_pressure() const
        {
            assert(status() == StatusByte::POLY_KEY_PRESSURE);
            return data_byte_2;
        }

        std::uint8_t control_number() const
        {
            assert(status() == StatusByte::CONTROL_CHANGE);
            return data_byte_1;
        }

        std::uint8_t control_value() const
        {
            assert(status() == StatusByte::CONTROL_CHANGE);
            return data_byte_2;
        }

        std::uint8_t program_number() const
        {
            assert(status() == StatusByte::PROGRAM_CHANGE);
            return data_byte_1;
        }

        std::uint8_t channel_pressure() const
        {
            assert(status() == StatusByte::CHANNEL_PRESSURE);
            return data_byte_1;
        }

        std::int16_t bend() const
        {
            assert(status() == StatusByte::PITCH_BEND);
            return std::int16_t(data_byte_1 << 7 | data_byte_2) - 8192;
        }

        std::uint8_t local_control() const
        {
            return control_value();
        }

        std::uint16_t song_position() const
        {
            return data_byte_2 << 7 | data_byte_1;
        }

        std::uint8_t song_number() const
        {
            return data_byte_1;
        }

    };

    struct SysexMessage;        // TBD

}

#endif /* !MIDI_MESSAGES_included */
