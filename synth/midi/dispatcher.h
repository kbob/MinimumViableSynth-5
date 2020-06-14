#ifndef MIDI_DISPATCHER_included
#define MIDI_DISPATCHER_included

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>

#include "synth/core/config.h"
#include "synth/midi/defs.h"
#include "synth/midi/messages.h"
#include "synth/midi/param.h"
#include "synth/util/fixed-map.h"
#include "synth/util/fixed-vector.h"

namespace midi {

    class Dispatcher : public ::Config::Subsystem {

    public:

        typedef std::uint8_t timbre_index;
        static const timbre_index NO_TIMBRE = 0xFF;

        typedef std::function<void(const SmallMessage&)>  small_handler;
        typedef std::function<void(const ParameterValue&)> xRPN_handler;
        typedef std::function<void(const SysexMessage&)>  sysex_handler;
        typedef std::uint16_t channel_mask;
        static const channel_mask ALL_CHANNELS = 0xFFFF;

    private:

        class TimbreDispatcher {

        public:

            TimbreDispatcher();

            void reset();

            void register_handler(StatusByte,        small_handler);
            void register_handler(ControllerNumber,  small_handler);
            void register_handler(RPN,                xRPN_handler);
            void register_handler(NRPN,               xRPN_handler);
            void register_handler(ChannelModeNumber, small_handler);

            void handle_channel_message(const SmallMessage&);
            void handle_cc(const SmallMessage&);
            void handle_data_entry_msb(const SmallMessage&);
            void handle_data_entry_lsb(const SmallMessage&);
            void handle_data_increment(const SmallMessage&);
            void handle_data_decrement(const SmallMessage&);
            void handle_rpn_msb(const SmallMessage&);
            void handle_rpn_lsb(const SmallMessage&);
            void handle_nrpn_msb(const SmallMessage&);
            void handle_nrpn_lsb(const SmallMessage&);

        private:

            enum class xRPN_state {
                INACTIVE,
                RPN_ACTIVE,
                NRPN_ACTIVE,
            };

            struct xRPN_bundle {
                ParameterValue  value;
                xRPN_handler    handler;
            };

            xRPN_bundle *get_xRPN(ParameterNumber * = nullptr);

            xRPN_state m_state;
            ParameterNumber::byte m_rpn_msb;
            ParameterNumber::byte m_rpn_lsb;
            ParameterNumber::byte m_nrpn_msb;
            ParameterNumber::byte m_nrpn_lsb;

            std::array<small_handler, 7> m_status_byte_handlers;
            std::array<small_handler, 128> m_cc_handlers;
            std::array<xRPN_bundle, MAX_RPNS> m_rpns;
            fixed_map<ParameterNumber, xRPN_bundle, MAX_NRPNS> m_nrpns;

        };

    public:

        Dispatcher();

        void reset();

        // a packet of messages arrives simultaneously.
        // defer calling handlers until end of packet.
        void begin_packet();
        void end_packet();

        void dispatch_message(const SmallMessage& msg);
        void dispatch_message(const SysexMessage& msg);

        // Channel Message Handlers
        void register_handler(StatusByte,        channel_mask, small_handler);
        void register_handler(ControllerNumber,  channel_mask, small_handler);
        void register_handler(RPN,               channel_mask, xRPN_handler);
        void register_handler(NRPN,              channel_mask, xRPN_handler);
        void register_handler(ChannelModeNumber, channel_mask, small_handler);

        // System Message Handlers
        void register_handler(StatusByte,                      small_handler);
        void register_handler(const SysexID&,                  sysex_handler);
        void register_handler(UniversalSysexNonRealTime,       sysex_handler);
        void register_handler(UniversalSysexRealTime,          sysex_handler);

        timbre_index channel_to_timbre(std::uint8_t channel);

    private:

        small_handler m_status_byte_handlers[128];
        std::array<std::uint8_t, CHANNEL_COUNT> m_channel_to_timbre;
        std::array<TimbreDispatcher, MAX_TIMBRALITY> m_timbre_dispatch;
        // ??? SYSEX ???

    };


    // -- TimbreDispatcher -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline
    Dispatcher::TimbreDispatcher::
    TimbreDispatcher()
    : m_state{xRPN_state::INACTIVE},
      m_rpn_msb{ParameterNumber::NO_BYTE},
      m_rpn_lsb{ParameterNumber::NO_BYTE},
      m_nrpn_msb{ParameterNumber::NO_BYTE},
      m_nrpn_lsb{ParameterNumber::NO_BYTE}
    {
        auto data_entry_msb_handler = [this] (const SmallMessage& msg)
        {
            handle_data_entry_msb(msg);
        };
        auto data_entry_lsb_handler = [this] (const SmallMessage& msg)
        {
            handle_data_entry_lsb(msg);
        };
        auto data_increment_handler = [this] (const SmallMessage& msg)
        {
            handle_data_increment(msg);
        };
        auto data_decrement_handler = [this] (const SmallMessage& msg)
        {
            handle_data_decrement(msg);
        };
        auto nrpn_lsb_handler = [this] (const SmallMessage& msg)
        {
            handle_nrpn_lsb(msg);
        };
        auto nrpn_msb_handler = [this] (const SmallMessage& msg)
        {
            handle_nrpn_msb(msg);
        };
        auto rpn_lsb_handler = [this] (const SmallMessage& msg)
        {
            handle_rpn_lsb(msg);
        };
        auto rpn_msb_handler = [this] (const SmallMessage& msg)
        {
            handle_rpn_msb(msg);
        };

        typedef ControllerNumber CC;
        register_handler(CC::DATA_ENTRY_MSB, data_entry_msb_handler);
        register_handler(CC::DATA_ENTRY_LSB, data_entry_lsb_handler);
        register_handler(CC::DATA_INCREMENT, data_increment_handler);
        register_handler(CC::DATA_DECREMENT, data_decrement_handler),
        register_handler(CC::NRPN_LSB,       nrpn_lsb_handler);
        register_handler(CC::NRPN_MSB,       nrpn_msb_handler);
        register_handler(CC::RPN_LSB,        rpn_lsb_handler);
        register_handler(CC::RPN_MSB,        rpn_msb_handler);
    }

    inline void
    Dispatcher::TimbreDispatcher::
    reset()
    {
        m_state = xRPN_state::INACTIVE;
        m_rpn_msb = m_rpn_lsb = ParameterNumber::NO_BYTE;
        m_nrpn_msb = m_nrpn_lsb = ParameterNumber::NO_BYTE;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    register_handler(StatusByte s, small_handler h)
    {
        assert((static_cast<std::uint8_t>(s) & 0x8F) == 0x80);
        assert((static_cast<std::uint8_t>(s) & 0xF0) != 0xF0);
        size_t index = static_cast<size_t>(s) >> 4 & 0x7;
        m_status_byte_handlers[index] = h;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    register_handler(ControllerNumber cc, small_handler h)
    {
        m_cc_handlers[static_cast<size_t>(cc)] = h;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    register_handler(RPN rpn, xRPN_handler h)
    {
        auto index = static_cast<size_t>(rpn);
        assert(index < MAX_RPNS);
        m_rpns[index].handler = h;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    register_handler(NRPN nrpn, xRPN_handler h)
    {
        auto index = ParameterNumber(nrpn);
        assert(!m_nrpns.count(index));
        m_nrpns[index].handler = h;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    register_handler(ChannelModeNumber mode, small_handler h)
    {
        m_cc_handlers[static_cast<size_t>(mode)] = h;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_channel_message(const SmallMessage& msg)
    {
        assert(msg.is_channel_message());
        auto s = msg.status_byte;
        auto& handler = m_status_byte_handlers[s >> 4 & 0x07];
        if (handler)
            handler(msg);
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_cc(const SmallMessage& msg)
    {
        assert(msg.status() == StatusByte::CONTROL_CHANGE);
        auto cc = msg.control_number();
        auto& handler = m_cc_handlers[cc];
        if (handler)
            handler(msg);
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_data_entry_msb(const SmallMessage& msg)
    {
        if (auto *xrpn = get_xRPN()) {
            xrpn->value.set_msb(msg.control_value());
            if (xrpn->value.is_valid())
                xrpn->handler(xrpn->value);
        }
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_data_entry_lsb(const SmallMessage& msg)
    {
        if (auto *xrpn = get_xRPN()) {
            xrpn->value.set_lsb(msg.control_value());
            if (xrpn->value.is_valid())
                xrpn->handler(xrpn->value);
        }
    }

    // According to MIDI RP-018:
    //  - ignore value byte
    //  - unless otherwise specified, inc/dec the LSB.
    //  - RPN 0000 Pitch Bend Sensitivity: roll over MSB at 100 cents.
    //      (centesimal)
    //  - RPNs 0002, 0003, and 0004: inc/dec the MSB.

    inline void
    Dispatcher::TimbreDispatcher::
    handle_data_increment(const SmallMessage&)
    {
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(&pn)) {
            if (xrpn->value.is_valid()) {
                if (m_state == xRPN_state::RPN_ACTIVE) {
                    switch (RPN(pn.number())) {

                    case RPN::PITCH_BEND_SENSITIVITY:
                        xrpn->value.increment_centesimally();
                        break;

                    case RPN::COARSE_TUNING:
                    case RPN::TUNING_PROGRAM_SELECT:
                    case RPN::TUNING_BANK_SELECT:
                        xrpn->value.increment_msb();
                        break;

                    default:
                        xrpn->value.increment_value();
                        break;
                    }
                } else {
                    xrpn->value.increment_value();
                }
                xrpn->handler(xrpn->value);
            }
        }
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_data_decrement(const SmallMessage&)
    {
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(&pn)) {
            if (xrpn->value.is_valid()) {
                if (m_state == xRPN_state::RPN_ACTIVE) {
                    switch (RPN(pn.number())) {

                    case RPN::PITCH_BEND_SENSITIVITY:
                        xrpn->value.decrement_centesimally();
                        break;

                    case RPN::COARSE_TUNING:
                    case RPN::TUNING_PROGRAM_SELECT:
                    case RPN::TUNING_BANK_SELECT:
                        xrpn->value.decrement_msb();
                        break;

                    default:
                        xrpn->value.decrement_value();
                        break;
                    }
                } else {
                    xrpn->value.decrement_value();
                }
                xrpn->handler(xrpn->value);
            }
        }
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_rpn_msb(const SmallMessage& msg)
    {
        m_rpn_msb = msg.control_value();
        m_state = xRPN_state::RPN_ACTIVE;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_rpn_lsb(const SmallMessage& msg)
    {
        m_rpn_lsb = msg.control_value();
        m_state = xRPN_state::RPN_ACTIVE;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_nrpn_msb(const SmallMessage& msg)
    {
        m_nrpn_msb = msg.control_value();
        m_state = xRPN_state::NRPN_ACTIVE;
    }

    inline void
    Dispatcher::TimbreDispatcher::
    handle_nrpn_lsb(const SmallMessage& msg)
    {
        m_nrpn_lsb = msg.control_value();
        m_state = xRPN_state::NRPN_ACTIVE;
    }

    inline
    Dispatcher::TimbreDispatcher::xRPN_bundle *
    Dispatcher::TimbreDispatcher::
    get_xRPN(ParameterNumber *pnp)
    {
        if (m_state == xRPN_state::RPN_ACTIVE) {
            auto nmsb = m_rpn_msb, nlsb = m_rpn_lsb;
            if (nmsb == ParameterNumber::NO_BYTE ||
                nlsb == ParameterNumber::NO_BYTE)
                return nullptr;
            auto pn = ParameterNumber(nmsb, nlsb);
            if (pn.number() >= MAX_RPNS)
                return nullptr;
            if (pnp)
                *pnp = pn;
            return &m_rpns[pn.number()];
        } else if (m_state == xRPN_state::NRPN_ACTIVE) {
            auto msb = m_nrpn_msb, lsb = m_nrpn_lsb;
            if (msb == ParameterNumber::NO_BYTE ||
                lsb == ParameterNumber::NO_BYTE)
                return nullptr;
            auto pn = ParameterNumber(msb, lsb);
            auto it = m_nrpns.find(pn.number());
            if (it == m_nrpns.end())
                return nullptr;
            if (pnp)
                *pnp = pn;
            return &it->second;
        }
        return nullptr;
    }


    // -- Dispatcher -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline
    Dispatcher::Dispatcher()
    {
        // initially, we map timbres to channels linearly.
        for (size_t i = 0; i < m_timbre_dispatch.size(); i++)
            m_channel_to_timbre[i] = i;
        for (size_t i = m_timbre_dispatch.size();
                    i < m_channel_to_timbre.size();
                    i++)
            m_channel_to_timbre[i] = NO_TIMBRE;

        // forward CC messages to timbre's handler.
        auto dispatch_cc = [this] (const SmallMessage& msg)
        {
            assert(msg.status() == StatusByte::CONTROL_CHANGE);
            uint8_t index = m_channel_to_timbre[msg.channel()];
            if (index != NO_TIMBRE)
                m_timbre_dispatch[index].handle_cc(msg);
        };
#if 0
        register_handler(StatusByte::CONTROL_CHANGE, ALL_CHANNELS, dispatch_cc);
#else
        // forward all other channel messages to timbre's handler.
        auto dispatch_channel_msg = [this] (const SmallMessage& msg)
        {
            assert(msg.is_channel_message());
            uint8_t index = m_channel_to_timbre[msg.channel()];
            if (index != NO_TIMBRE)
                m_timbre_dispatch[index].handle_channel_message(msg);
        };
        for (std::uint8_t s = 0x80 & 0x7F; s < (0xF0 & 0x7F); s++) {
            if(s >= (0xB0 & 0x7F) && s < (0xC0 & 0x7F))
                m_status_byte_handlers[s] = dispatch_cc;
            else
                m_status_byte_handlers[s] = dispatch_channel_msg;
        }
#endif
    }

    // inline void
    // Dispatcher::
    // begin_packet()
    // {}
    //
    // inline void
    // Dispatcher::
    // end_packet()
    // {}

    inline void
    Dispatcher::
    reset()
    {
        for (auto& cd: m_timbre_dispatch)
            cd.reset();
    }

    inline void
    Dispatcher::
    dispatch_message(const SmallMessage& msg)
    {
        assert(msg.status_byte & 0x80);
        size_t index = msg.status_byte & 0x7F;
        auto& handler = m_status_byte_handlers[index];
        if (handler)
            handler(msg);
    }

    inline void
    Dispatcher::
    dispatch_message(const SysexMessage&)
    {
        assert(!"write me!");
    }

    inline void
    Dispatcher::
    register_handler(StatusByte s, channel_mask m, small_handler h)
    {
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
#if 0
                size_t index = (static_cast<size_t>(s) + chan) & 0x7F;
                auto& pos = m_status_byte_handlers[index];
                assert(!pos);
                pos = h;
#else
                auto timbre = m_channel_to_timbre[chan];
                if (timbre != NO_TIMBRE)
                    m_timbre_dispatch[timbre].register_handler(s, h);
#endif
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(ControllerNumber cc, channel_mask m, small_handler h)
    {
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
                auto timbre = m_channel_to_timbre[chan];
                if (timbre != NO_TIMBRE)
                    m_timbre_dispatch[timbre].register_handler(cc, h);
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(RPN rpn, channel_mask m, xRPN_handler h)
    {
        assert(unsigned(rpn) < MAX_RPNS);
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
                auto timbre = m_channel_to_timbre[chan];
                if (timbre != NO_TIMBRE)
                    m_timbre_dispatch[timbre].register_handler(rpn, h);
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(NRPN nrpn, channel_mask m, xRPN_handler h)
    {
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
                auto timbre = m_channel_to_timbre[chan];
                if (timbre != NO_TIMBRE)
                    m_timbre_dispatch[timbre].register_handler(nrpn, h);
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(ChannelModeNumber mode, channel_mask m, small_handler h)
    {
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
                auto timbre = m_channel_to_timbre[chan];
                if (timbre != NO_TIMBRE)
                    m_timbre_dispatch[timbre].register_handler(mode, h);
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(StatusByte s, small_handler h)
    {
        size_t index = static_cast<size_t>(s) & 0x7F;
        auto& pos = m_status_byte_handlers[index];
        assert(!pos);
        pos = h;
    }

    // inline void
    // Dispatcher::
    // register_handler(const SysexID&, sysex_handler)
    // {}
    //
    // inline void
    // Dispatcher::
    // register_handler(UniversalSysexNonRealTime, sysex_handler)
    // {}
    //
    // inline void
    // Dispatcher::
    // register_handler(UniversalSysexRealTime, sysex_handler)
    // {}

    inline
    Dispatcher::timbre_index
    Dispatcher::
    channel_to_timbre(std::uint8_t channel)
    {
        assert(channel < CHANNEL_COUNT);
        return m_channel_to_timbre[channel];
    }

}

#endif /* !MIDI_DISPATCHER_included */
