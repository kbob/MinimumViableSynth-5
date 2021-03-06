#ifndef MIDI_PARAM_included
#define MIDI_PARAM_included

#include <cassert>
#include <cstdint>

#include "synth/midi/defs.h"

namespace midi {

    class ParameterNumber {

    public:

        typedef std::uint16_t number_type;
        typedef std::uint8_t byte;

        static const number_type MAX = 128 * 256 - 1;
        static const number_type NO_NUMBER =
            std::numeric_limits<number_type>::max();
        static const number_type NO_BYTE =
            std::numeric_limits<byte>::max();

        // Constructors
        ParameterNumber()
        : m_key{NO_NUMBER}
        {}
        ParameterNumber(number_type key)
        : m_key{key}
        {
            assert(is_valid());
        }
        ParameterNumber(RPN rpn)
        : m_key{static_cast<number_type>(rpn)}
        {
            assert(is_valid());
        }
        ParameterNumber(NRPN nrpn)
        : m_key{static_cast<number_type>(nrpn)}
        {
            assert(is_valid());
        }
        ParameterNumber(byte msb, byte lsb)
        : m_key{static_cast<number_type>(msb << 8 | lsb)}
        {
            assert(is_valid());
        }

        // Predicates
        bool is_valid() const
        {
            return !(m_key & ~0x7F7F);
        }

        // Accessors
        number_type number() const
        {
            assert(is_valid());
            return m_key;
        }
        byte msb() const
        {
            assert(is_valid());
            return m_key >> 8;
        }
        byte lsb() const
        {
            assert(is_valid());
            return m_key;
        }

        // Relational Operators
        friend bool
        operator == (const ParameterNumber& a, const ParameterNumber& b)
        {
            return a.m_key == b.m_key;
        }

        friend bool
        operator < (const ParameterNumber& a, const ParameterNumber& b)
        {
            return a.m_key < b.m_key;
        }

    private:

        number_type m_key;

    };

    bool operator != (const ParameterNumber& a, const ParameterNumber& b)
    {
        return !(a == b);
    }

    struct ParameterValue {

    public:

        typedef std::uint16_t value_type;
        typedef std::uint8_t byte;

        static const value_type MAX = 128 * 128 - 1;
        static const value_type NO_VALUE = 0xFF80;
        static const value_type NO_BYTE = std::numeric_limits<byte>::max();

        // Constructors
        ParameterValue() : m_value{NO_VALUE} {}
        ParameterValue(value_type val) : m_value(val) { assert(val <= MAX); }
        ParameterValue(byte msb, byte lsb)
        : m_value{static_cast<value_type>(msb << 7 | lsb)}
        {}

        // Predicates
        bool is_valid() const
        {
            return !(m_value & 0xC000);
        }
        bool is_valid_centesimal() const
        {
            return is_valid() && lsb() < 100;
        }

        // Accessors
        value_type value() const
        {
            assert(is_valid());
            return m_value;
        }
        byte msb() const
        {
            return m_value >> 7;
        }
        byte lsb() const
        {
            return m_value & 0x7F;
        }

        // Operations
        void increment_value()
        {
            if (is_valid() && m_value < MAX)
                m_value++;
        }
        void decrement_value()
        {
            if (is_valid() && m_value > 0)
                --m_value;
        }
        void increment_msb()
        {
            if (is_valid()) {
                if (m_value + 128 > MAX)
                    m_value = MAX;
                else
                    m_value += 128;
            }
        }
        void decrement_msb()
        {
            if (is_valid()) {
                if (m_value < 128)
                    m_value = 0;
                else
                    m_value -= 128;
            }
        }
        void increment_centesimally()
        {
            if (is_valid_centesimal()) {
                if (lsb() == 99) {
                    if (msb() < 127)
                        m_value += 128 - 99;    // inc msb, reset lsb
                } else
                    m_value++;
            }
        }
        void decrement_centesimally()
        {
            if (is_valid_centesimal()) {
                if (lsb() == 0) {
                    if (msb() != 0)
                        m_value -= 128 - 99;    // dec msb, set lsb = 99
                } else
                    --m_value;
            }
        }
        void set_msb(byte msb)
        {
            assert(!(msb & 0x80));
            m_value = (m_value & 0x007F) | msb << 7;
        }
        void set_lsb(byte lsb)
        {
            assert(!(lsb & 0x80));
            m_value = (m_value & 0xFF80) | lsb;
        }

    private:

        value_type m_value;

    };

}

#endif /* !MIDI_PARAM_included */
