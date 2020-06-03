#ifndef MIDI_PARSER_included
#define MIDI_PARSER_included

#include <cstdint>
#include <functional>
#include <stdexcept>

#include "synth/midi/config.h"
#include "synth/midi/defs.h"
#include "synth/midi/messages.h"

namespace midi {

    class Parser {

    public:

        typedef std::function<void(const SmallMessage&)> small_handler;
        typedef std::function<void(const SysexMessage&)> sysex_handler;

        Parser()
        : m_small_handler{nullptr},
          m_sysex_handler{nullptr},
          m_state{NO}
        {}

        void register_handler(small_handler h) { m_small_handler = h; }
        void register_handler(sysex_handler h) { m_sysex_handler = h; }

        void process_byte(char byte)
        {
            parse_byte(byte);
        }

        void process_bytes(const char *bytes, size_t count)
        {
            for (size_t i = 0; i < count; i++)
                parse_byte(bytes[i]);
        }

        // These messages do not use running status.
        // Throw `std::runtime_error` if message is not
        // legal MIDI.
        void process_message(const char *msg, size_t count)
        {
            assert(count > 0);
            if (!(msg[0] & 0x80)) {

        malformed:
                throw std::runtime_error("malformed message");
            }

            auto state = s_state_table[msg[0] & 0x7F];
            if (count == 1) {
                if (state != S1 && state != RT)
                    goto malformed;
                emit_msg(SmallMessage(msg[0]));
            } else if (count == 2) {
                if (state != C21 && state != S21)
                    goto malformed;
                if (msg[1] & 0x80)
                    goto malformed;
                emit_msg(SmallMessage(msg[0], msg[1]));
            } else if (count == 3) {
                if (state != C31 && state != S31)
                    goto malformed;
                if ((msg[1] & 0x80) || (msg[2] & 0x80))
                    goto malformed;
                emit_msg(SmallMessage(msg[0], msg[1], msg[2]));
            } else {
                auto e = static_cast<StatusByte>(msg[count - 1]);
                if (state != SX || e != StatusByte::EOX)
                    goto malformed;
                SysexMessage sysex;
                for (size_t i = 0; i < count; i++) {
                    if (i && i < count - 1 && msg[i] & 0x80)
                        goto malformed;
                    sysex.append(msg[i]);
                }
                emit_sysex_msg(sysex);
            }
            reset();            // can't interleave messages and bytes.
        }

        void reset()
        {
            m_state = NO;
            m_msg.clear();
            m_sysex_msg.clear();
        }

    private:
        enum State : std::uint8_t {
            NO,                 // no message
            C21,                // channel msg, 1 of 2 bytes
            C31,                // channel msg, 1 of 3 bytes
            C32,                // channel msg, 2 of 3 bytes
            SX,                 // sysex msg
            S1,                 // system msg, 1 byte
            S21,                // system msg, 1 of 2 bytes
            S31,                // system msg, 1 of 3 bytes
            S32,                // system msg, 2 of 3 bytes
            EX,                 // end sysex msg
            RT,                 // real time msg
            URT,                // undefined real time msg
        };

        small_handler m_small_handler;
        sysex_handler m_sysex_handler;
        State m_state;
        SmallMessage m_msg;
        SysexMessage m_sysex_msg;

        static const State s_state_table[128];

        void parse_byte(std::uint8_t c)
        {
            if (c & 0x80) {
                State prev_state = m_state;
                m_state = s_state_table[c & 0x7f];
                switch (m_state) {

                case RT:
                    emit_msg(SmallMessage(c));
                    // fall through...
                case URT:
                    m_state = prev_state;
                    return;

                case SX:
                    m_sysex_msg.clear();
                    m_sysex_msg.append(c);
                    break;

                case EX:
                    // if a sysex messsage is too long, the extra
                    // bytes are not accumulated, and the message
                    // is not emitted.
                    if (prev_state == SX && m_sysex_msg.size < MAX_SYSEX_SIZE) {
                        m_sysex_msg.append(c);
                        emit_sysex_msg(m_sysex_msg);
                    }
                    m_state = NO;
                    break;

                case S1:
                    m_msg.status_byte = c;
                    m_msg.clear_data();
                    emit_msg(m_msg);
                    m_msg.clear();
                    m_state = NO;
                    break;

                case C21:
                case C31:
                case S21:
                case S31:
                    m_msg.status_byte = c;
                    m_msg.clear_data();
                    break;

                case NO:
                    m_msg.clear();
                    break;

                default:        // C32 S32
                    assert(!"impossible parser state");
                }
            } else {
                switch (m_state) {

                case NO:
                    break;

                case C31:
                    m_msg.data_byte_1 = c;
                    m_state = C32;
                    break;

                case C32:
                    m_msg.data_byte_2 = c;
                    emit_msg(m_msg);
                    m_msg.clear_data();
                    m_state = C31;
                    break;

                case C21:
                    m_msg.data_byte_1 = c;
                    emit_msg(m_msg);
                    m_msg.clear_data();
                    // m_state = C21; (unchanged)
                    break;

                case SX:
                    if (m_sysex_msg.size < MAX_SYSEX_SIZE)
                        m_sysex_msg.append(c);
                    // m_state = SX; (unchanged)
                    break;

                case S31:
                    m_msg.data_byte_1 = c;
                    m_state = S32;
                    break;

                case S32:
                    m_msg.data_byte_2 = c;
                    emit_msg(m_msg);
                    m_msg.clear();
                    m_state = NO;
                    break;

                case S21:
                    m_msg.data_byte_1 = c;
                    emit_msg(m_msg);
                    m_msg.clear();
                    m_state = NO;
                    break;

                default:        // states RT URT EX S1
                    assert(!"impossible parser state");
                    break;
                }
            }
        }

        void emit_msg(const SmallMessage& msg)
        {
            if (m_small_handler)
                m_small_handler(msg);
        }

        void emit_sysex_msg(const SysexMessage& msg)
        {
            if (m_sysex_handler)
                m_sysex_handler(msg);
        }

    };

    const Parser::State Parser::s_state_table[128] = {

        C31, C31, C31, C31,     C31, C31, C31, C31,     // Note Off
        C31, C31, C31, C31,     C31, C31, C31, C31,

        C31, C31, C31, C31,     C31, C31, C31, C31,     // Note On
        C31, C31, C31, C31,     C31, C31, C31, C31,

        C31, C31, C31, C31,     C31, C31, C31, C31,     // Poly Key Pressure
        C31, C31, C31, C31,     C31, C31, C31, C31,

        C31, C31, C31, C31,     C31, C31, C31, C31,     // Control Change
        C31, C31, C31, C31,     C31, C31, C31, C31,

        C21, C21, C21, C21,     C21, C21, C21, C21,     // Program Change
        C21, C21, C21, C21,     C21, C21, C21, C21,

        C21, C21, C21, C21,     C21, C21, C21, C21,     // Channel Pressure
        C21, C21, C21, C21,     C21, C21, C21, C21,

        C31, C31, C31, C31,     C31, C31, C31, C31,     // Pitch Bend
        C31, C31, C31, C31,     C31, C31, C31, C31,

        SX,  S21, S31, S21,     NO,  NO,  S1,  EX,      // System Common
        RT,  URT, RT,  RT,      RT,  URT, RT,  RT,      // System Real Time

    };

}

#endif /* !MIDI_PARSER_included */
