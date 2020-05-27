#ifndef MIDI_MESSAGES_included
#define MIDI_MESSAGES_included

#include <cassert>
#include <cstdint>

#include "synth/midi/config.h"
#include "synth/midi/defs.h"

namespace midi {

    struct SmallMessage {
        std::uint8_t status_byte;
        std::uint8_t data_byte_1;
        std::uint8_t data_byte_2;

        static const std::uint8_t NO_STATUS = 0x00;
        static const std::uint8_t NO_DATA = 0xFF;

        SmallMessage()
        : status_byte{NO_STATUS},
          data_byte_1{NO_DATA},
          data_byte_2{NO_DATA}
        {}

        SmallMessage(std::uint8_t s)
        : status_byte{s},
          data_byte_1{NO_DATA},
          data_byte_2{NO_DATA}
        {
            assert(is_system_real_time_message() ||
                   status() == StatusByte::TUNE_REQUEST);
        }

        SmallMessage(std::uint8_t s, std::uint8_t d1)
        : status_byte{s},
          data_byte_1{d1},
          data_byte_2{NO_DATA}
        {
            assert(status() == StatusByte::PROGRAM_CHANGE ||
                   status() == StatusByte::CHANNEL_PRESSURE ||
                   status() == StatusByte::MTC_QUARTER_FRAME ||
                   status() == StatusByte::SONG_SELECT);
            assert(!(d1 & 0x80));
        }

        SmallMessage(std::uint8_t s, std::uint8_t d1, std::uint8_t d2)
        : status_byte{s},
          data_byte_1{d1},
          data_byte_2{d2}
        {
            assert(status() == StatusByte::NOTE_OFF ||
                   status() == StatusByte::NOTE_ON ||
                   status() == StatusByte::POLY_KEY_PRESSURE ||
                   status() == StatusByte::CONTROL_CHANGE ||
                   status() == StatusByte::PITCH_BEND ||
                   status() == StatusByte::SELECT_CHANNEL_MODE ||
                   status() == StatusByte::SONG_POSITION);
            assert(!(d1 & 0x80));
            assert(!(d2 & 0x80));
        }

        void clear()
        {
            status_byte = NO_STATUS;
            data_byte_1 = NO_DATA;
            data_byte_2 = NO_DATA;
        }

        void clear_data()
        {
            data_byte_1 = NO_DATA;
            data_byte_2 = NO_DATA;
        }

        // The MIDI message hierarchy [detail p. 42]
        //
        // Message
        //   Channel Message
        //     Channel Voice Message
        //       Note Off, Note On, Poly Key Pressure, Control Change,
        //       Program Change, Channel Pressure, Pitch Bend
        //     Channel Mode Message
        //       All Sound Off, Reset All Controllers, Local Control,
        //       All Notes Off, Omni Off, Omni On, Mono On, Poly On
        //   System Message
        //     System Common Message
        //       MIDI Time Code Quarter Frame, Song Position Pointer,
        //       Song Select, Tune Request, EOX
        //     System Real Time Message
        //       Timing Clock, Start, Continue, Stop, Active Sensing,
        //       System Reset
        //     System Exclusive Message
        //        Universal System Exclusive Message Non Real Time
        //          Sample Dump Header, Sample Dump Packet, Sample Dump
        //          Request, MIDI Time Code, Sample Dump Extensions,
        //          General Information, File Dump, MIDI Tuning
        //          Standard, General MIDI, End of File, Wait, Cancel,
        //          NAK, ACK
        //        Universal System Exclusive Message Real Time
        //          MIDI Time Code, MIDI Show Control, Notation
        //          Information, Device Control, Real Time MTC Coding,
        //          MIDI Machine Control Commands, MIDI Machine Control
        //          Responses, MIDI Tuning Standard

        bool is_channel_message() const
        {
            return (status_byte & 0x80) && (status_byte & 0xF0) != 0xF0;
        }

        bool is_channel_voice_message() const
        {
            if (!is_channel_message())
                return false;
            if (status() != StatusByte::SELECT_CHANNEL_MODE)
                return true;
            return control_number() < 120;
        }

        bool is_channel_mode_message() const
        {
            return is_channel_message() &&
                   status() == StatusByte::SELECT_CHANNEL_MODE &&
                   control_number() >= 120;
        }

        bool is_system_message() const
        {
            return status_byte >= 0xF0;
        }

        bool is_system_exclusive_message() const
        {
            return false;       // sysex messages have their own class.
        }

        bool is_system_common_message() const
        {
            return (status_byte & 0xF8) == 0xF0;
        }

        bool is_system_real_time_message() const
        {
            return status_byte >= 0xF8;
        }

        // return high nybble of channel messages, whole status byte of
        // system messages.
        StatusByte status() const
        {
            std::uint8_t s = status_byte;
            assert(s & 0x80);
            if (is_channel_message())
                s &= 0xF0;
            return static_cast<StatusByte>(s);
        }

        uint8_t channel() const
        {
            assert(is_channel_message());
            return status_byte & 0x0F;
        }

        std::uint8_t note_number() const
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
            return std::int16_t(data_byte_2 << 7 | data_byte_1) - 8192;
        }

        std::uint16_t song_position() const
        {
            assert(status() == StatusByte::SONG_POSITION);
            return data_byte_2 << 7 | data_byte_1;
        }

        std::uint8_t song_number() const
        {
            assert(status() == StatusByte::SONG_SELECT);
            return data_byte_1;
        }

    };

    struct SysexID {
        std::uint8_t data[3];

        static const std::uint8_t NO_VALUE = 0xFF;

        SysexID()
        : data{NO_VALUE, NO_VALUE, NO_VALUE}
        {}

        SysexID(std::uint8_t d0)
        : data{d0, NO_VALUE, NO_VALUE}
        {
            assert(d0 != 0 && !(d0 & 0x80));
        }

        SysexID(std::uint8_t d0, std::uint8_t d1, std::uint8_t d2)
        : data{d0, d1, d2}
        {
            assert(d0 == 0);
            assert(!(d1 & 0x80));
            assert(!(d2 & 0x80));
            assert(d1 || d2);
        }

        size_t size() const
        {
            assert(data[0] != NO_VALUE);
            return (data[0] == 0x00) ? 3 : 1;
        }

        static const SysexID NON_COMMERCIAL;
        static const SysexID UNIVERSAL;
        static const SysexID UNIVERSAL_REAL_TIME;

    };

    const SysexID SysexID::NON_COMMERCIAL(0x7D);
    const SysexID SysexID::UNIVERSAL(0x7E);
    const SysexID SysexID::UNIVERSAL_REAL_TIME(0x7F);

    bool operator == (const SysexID& a, const SysexID& b)
    {
        if (a.data[0] != b.data[0])
            return false;
        if (a.data[0] != 0x00)
            return true;
        return a.data[1] == b.data[1] && a.data[2] == b.data[2];
    }

    bool operator != (const SysexID& a, const SysexID& b)
    {
        return !(a == b);
    }

    enum class SysexDeviceID {
        ALL_CALL = 0x7F,
    };

    struct SysexMessage {
        size_t size;
        std::uint8_t data[MAX_SYSEX_SIZE];

        static const std::uint8_t NO_DATA = 0xFF;

        SysexMessage()
        {
            clear();
        }

        SysexID id() const
        {
            if (size >= 3 && data[1] != 0x00)
                return SysexID(data[1]);
            if (size >= 5 && data[1] == 0x00)
                return SysexID(data[1], data[2], data[3]);
            return SysexID();
        }

        SysexDeviceID device_id() const
        {
            if (size >= 4 && data[1] != 0x00)
                return static_cast<SysexDeviceID>(data[2]);
            if (size >= 6 && data[1] == 0x00)
                return static_cast<SysexDeviceID>(data[4]);
            return static_cast<SysexDeviceID>(NO_DATA);
        }

        const std::uint8_t *payload() const
        {
            size_t offset = 2 + id().size();
            assert(offset < size - 1);
                return data + offset;
        }

        size_t payload_size() const
        {
            size_t offset = 2 + id().size();
            // std::cout << "offset = " << offset << std::endl;
            // std::cout << "size = " << size << std::endl;
            if (offset < size - 1)
                return size - offset - 1;
            return 0;
        }

        void clear()
        {
            for (size_t i = 0; i < MAX_SYSEX_SIZE; i++)
                data[i] = NO_DATA;
            size = 0;
        }

        void append(std::uint8_t byte)
        {
            assert(size < MAX_SYSEX_SIZE);
            data[size++] = byte;
        }

        void append(StatusByte s)
        {
            assert((s == StatusByte::SYSTEM_EXCLUSIVE && size == 0) ||
                   (s == StatusByte::EOX && size > 0));
            if (size < MAX_SYSEX_SIZE)
                data[size++] = static_cast<std::uint8_t>(s);
        }

        void append(const SysexID& s)
        {
            assert(size == 1);
            size_t id_size = s.size();
            if (size + id_size <= MAX_SYSEX_SIZE) {
                for (size_t i = 0; i < id_size; i++)
                    data[size++] = s.data[i];
            }
        }

        void append(SysexDeviceID d)
        {
            assert(size == 2 || size == 4);
            if (size < MAX_SYSEX_SIZE)
                data[size++] = static_cast<std::uint8_t>(d);
        }

    };

}

#endif /* !MIDI_MESSAGES_included */
