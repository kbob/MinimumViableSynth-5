#ifndef MIDI_DISPATCHER_included
#define MIDI_DISPATCHER_included

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>

#include "synth/midi/defs.h"
#include "synth/midi/messages.h"
#include "synth/midi/param.h"
#include "synth/util/fixed-map.h"
#include "synth/util/fixed-vector.h"
#include "synth/util/function.h"

namespace midi {

    class Dispatcher {

        typedef std::uint16_t c_mask;
        typedef std::uint16_t t_mask;
        static_assert(MAX_TIMBRES <= std::numeric_limits<t_mask>::digits,
                      "timbre mask too small");

    public:

        typedef function<void(const SmallMessage&)>  small_handler;
        typedef function<void(std::uint8_t channel,
                              const ParameterNumber&,
                              const ParameterValue&)> xRPN_handler;
        typedef function<void(const SysexMessage&)>  sysex_handler;
        typedef c_mask channel_mask;
        typedef t_mask timbre_mask;

        static const channel_mask ALL_CHANNELS = 0xFFFF;
        static const timbre_mask  ALL_TIMBRES = (1 << MAX_TIMBRES) - 1;

        Dispatcher();

        // Reset the xRPN state machines.
        void reset();

        // a packet of messages arrives simultaneously.
        // defer calling handlers until end of packet.
        void begin_packet();
        void end_packet();

        // Insert messages here.
        void dispatch_message(const SmallMessage& msg);
        void dispatch_message(const SysexMessage& msg);

        // Channel Message Handlers
        void register_handler(StatusByte,        c_mask, const small_handler&);
        void register_handler(ControllerNumber,  t_mask, const small_handler&);
        void register_handler(RPN,               t_mask, const  xRPN_handler&);
        void register_handler(NRPN,              t_mask, const  xRPN_handler&);
        void register_handler(ChannelModeNumber, t_mask, const small_handler&);

        // System Message Handlers
        void register_handler(StatusByte,                const small_handler&);
        void register_handler(const SysexID&,            const sysex_handler&);
        void register_handler(UniversalSysexNonRealTime, const sysex_handler&);
        void register_handler(UniversalSysexRealTime,    const sysex_handler&);

        // Channels and timbres are mapped many-to-many.  Use either
        // of these to set one row or column of the mapping.
        void set_channel_timbres(std::uint8_t channel, timbre_mask);
        void set_timbre_channels(std::uint8_t timbre, channel_mask);

    private:

        enum class xRPN_state : std::uint8_t {
            INACTIVE,
            RPN_ACTIVE,
            NRPN_ACTIVE,
        };

        struct xRPN_bundle {
            std::array<ParameterValue, CHANNEL_COUNT> values;
            std::array<xRPN_handler, MAX_TIMBRES> handlers;

            void broadcast(timbre_mask, std::uint8_t channel, ParameterNumber);
        };

        struct channel {
            timbre_mask           timbres;
            xRPN_state            state;
            ParameterNumber::byte rpn_msb;
            ParameterNumber::byte rpn_lsb;
            ParameterNumber::byte nrpn_msb;
            ParameterNumber::byte nrpn_lsb;

            channel()
            : timbres{ALL_TIMBRES},
              state{xRPN_state::INACTIVE},
              rpn_msb{ParameterNumber::NO_BYTE},
              rpn_lsb{ParameterNumber::NO_BYTE},
              nrpn_msb{ParameterNumber::NO_BYTE},
              nrpn_lsb{ParameterNumber::NO_BYTE}
            {}

            void reset();
        };

        struct timbre {
            channel_mask channels;
            std::array<small_handler, 128> cc_handlers;

            timbre()
            : channels{ALL_CHANNELS}    // start in omni mode.
            {}
        };

        void handle_cc(const SmallMessage&);
        void handle_data_entry_msb(const SmallMessage&);
        void handle_data_entry_lsb(const SmallMessage&);
        void handle_data_increment(const SmallMessage&);
        void handle_data_decrement(const SmallMessage&);
        void handle_rpn_msb(const SmallMessage&);
        void handle_rpn_lsb(const SmallMessage&);
        void handle_nrpn_msb(const SmallMessage&);
        void handle_nrpn_lsb(const SmallMessage&);

        xRPN_bundle *get_xRPN(std::uint8_t channel,
                              ParameterNumber * = nullptr);

        template <void (Dispatcher::*M)(const SmallMessage&)>
            using small_binding =
                small_handler::binding<Dispatcher, M>;

        using cc_binding =
            small_binding<&Dispatcher::handle_cc>;
        using data_entry_msb_binding =
            small_binding<&Dispatcher::handle_data_entry_msb>;
        using data_entry_lsb_binding =
            small_binding<&Dispatcher::handle_data_entry_lsb>;
        using data_increment_binding =
            small_binding<&Dispatcher::handle_data_increment>;
        using data_decrement_binding =
            small_binding<&Dispatcher::handle_data_decrement>;
        using rpn_msb_binding =
            small_binding<&Dispatcher::handle_rpn_msb>;
        using rpn_lsb_binding =
            small_binding<&Dispatcher::handle_rpn_lsb>;
        using nrpn_msb_binding =
            small_binding<&Dispatcher::handle_nrpn_msb>;
        using nrpn_lsb_binding =
            small_binding<&Dispatcher::handle_nrpn_lsb>;

        static const std::uint8_t NO_TIMBRE = 0xFF;

        std::array<small_handler, 128> m_status_byte_handlers;
        std::array<xRPN_bundle, MAX_RPNS> m_rpns;
        fixed_map<ParameterNumber, xRPN_bundle, MAX_NRPNS> m_nrpns;

        std::array<channel, CHANNEL_COUNT> m_channels;
        std::array<timbre, MAX_TIMBRES> m_timbres;

        // ??? SYSEX ???

    };


    // -- Dispatcher::xRPN_bundle - -- -- -- -- -- -- -- -- -- -- -- -- //

    inline void
    Dispatcher::xRPN_bundle::
    broadcast(timbre_mask timbres,
              std::uint8_t channel,
              ParameterNumber number)
    {
        auto value = values[channel];
        if (value.is_valid()) {
            for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
                if (timbres & (1 << ti)) {
                    auto& handler = handlers[ti];
                    if (handler)
                        handler(channel, number, value);
                }
            }
        }
    }


    // -- Dispatcher -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline
    Dispatcher::
    Dispatcher()
    {
        register_handler(StatusByte::CONTROL_CHANGE,
                         ALL_CHANNELS,
                         cc_binding(this));
        register_handler(ControllerNumber::DATA_ENTRY_MSB,
                         ALL_TIMBRES,
                         data_entry_msb_binding(this));
        register_handler(ControllerNumber::DATA_ENTRY_MSB,
                         ALL_TIMBRES,
                         data_entry_msb_binding(this));
        register_handler(ControllerNumber::DATA_ENTRY_LSB,
                         ALL_TIMBRES,
                         data_entry_lsb_binding(this));
        register_handler(ControllerNumber::DATA_INCREMENT,
                         ALL_TIMBRES,
                         data_increment_binding(this));
        register_handler(ControllerNumber::DATA_DECREMENT,
                         ALL_TIMBRES,
                         data_decrement_binding(this));
        register_handler(ControllerNumber::RPN_MSB,
                         ALL_TIMBRES,
                         rpn_msb_binding(this));
        register_handler(ControllerNumber::RPN_LSB,
                         ALL_TIMBRES,
                         rpn_lsb_binding(this));
        register_handler(ControllerNumber::NRPN_MSB,
                         ALL_TIMBRES,
                         nrpn_msb_binding(this));
        register_handler(ControllerNumber::NRPN_LSB,
                         ALL_TIMBRES,
                         nrpn_lsb_binding(this));
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
        for (auto& chan: m_channels)
            chan.reset();
    }

    inline void
    Dispatcher::
    dispatch_message(const SmallMessage& msg)
    {
        assert(msg);
        size_t index = msg.status_byte & 0x7F;
        auto& handler = m_status_byte_handlers[index];
        if (handler)
            handler(msg);
    }

    // inline void
    // Dispatcher::
    // dispatch_message(const SysexMessage&)
    // {
    //     assert(!"write me!");
    // }

    inline void
    Dispatcher::
    register_handler(StatusByte s, channel_mask m, const small_handler& h)
    {
        for (size_t chan = 0; chan < CHANNEL_COUNT; chan++) {
            if (m & (1 << chan)) {
                size_t index = (static_cast<size_t>(s) + chan) & 0x7F;
                auto& pos = m_status_byte_handlers[index];
                assert(!pos);
                pos = h;
            }
        }
    }

    inline void
    Dispatcher::
    register_handler(ControllerNumber cc, timbre_mask m, const small_handler& h)
    {
        auto index = static_cast<size_t>(cc);
        assert(index < CC_COUNT);
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                m_timbres[ti].cc_handlers[index] = h;
    }

    inline void
    Dispatcher::
    register_handler(RPN rpn, timbre_mask m, const xRPN_handler& h)
    {
        auto index = static_cast<size_t>(rpn);
        auto& handlers = m_rpns[index].handlers;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                handlers[ti] = h;
    }

    inline void
    Dispatcher::
    register_handler(NRPN nrpn, timbre_mask m, const xRPN_handler& h)
    {
        auto index = static_cast<size_t>(nrpn);
        auto& handlers = m_nrpns[index].handlers;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                handlers[ti] = h;
    }

    inline void
    Dispatcher::
    register_handler(ChannelModeNumber mode,
                     timbre_mask m,
                     const small_handler& h)
    {
        auto index = static_cast<size_t>(mode);
        assert(index >= CC_COUNT && index < m_timbres[0].cc_handlers.size());
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                m_timbres[ti].cc_handlers[index] = h;
    }

    inline void
    Dispatcher::
    register_handler(StatusByte s, const small_handler& h)
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

    inline void
    Dispatcher::set_channel_timbres(std::uint8_t channel, timbre_mask mask)
    {
        m_channels[channel].timbres = mask;
        for (size_t i = 0, bit = 1; i < m_timbres.size(); i++, bit <<= 1) {
            if (mask & (1 << i))
                m_timbres[i].channels |= bit;
            else
                m_timbres[i].channels &= ~bit;
        }
    }

    inline void
    Dispatcher::set_timbre_channels(std::uint8_t timbre, channel_mask mask)
    {
        m_timbres[timbre].channels = mask;
        timbre_mask bit = 1 << timbre;
        for (size_t i = 0; i < m_channels.size(); i++)
            if (mask & (1 << i))
                m_channels[i].timbres |= bit;
            else
                m_channels[i].timbres &= ~bit;
    }

    inline void
    Dispatcher::
    handle_cc(const SmallMessage& msg)
    {
        assert(msg.status() == StatusByte::CONTROL_CHANGE);
        auto chan = msg.channel();
        auto cc = msg.control_number();
        auto timbres = m_channels[chan].timbres;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
            if (timbres & ( 1 << ti)) {
                auto& handler = m_timbres[ti].cc_handlers[cc];
                if (handler)
                    handler(msg);
            }
        }
    }

    inline void
    Dispatcher::
    handle_data_entry_msb(const SmallMessage& msg)
    {
        auto channel = msg.channel();
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(channel, &pn)) {
            auto& value = xrpn->values[channel];
            value.set_msb(msg.control_value());
            xrpn->broadcast(m_channels[channel].timbres, channel, pn);
        }
    }

    inline void
    Dispatcher::
    handle_data_entry_lsb(const SmallMessage& msg)
    {
        auto channel = msg.channel();
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(channel, &pn)) {
            auto& value = xrpn->values[channel];
            value.set_lsb(msg.control_value());
            xrpn->broadcast(m_channels[channel].timbres, channel, pn);
        }
    }

    // According to MIDI RP-018:
    //  - ignore value byte
    //  - unless otherwise specified, inc/dec the LSB.
    //  - RPN 0000 Pitch Bend Sensitivity: roll over MSB at 100 cents.
    //      (centesimal)
    //  - RPNs 0002, 0003, and 0004: inc/dec the MSB.

    inline void
    Dispatcher::
    handle_data_increment(const SmallMessage& msg)
    {
        auto channel = msg.channel();
        auto& chan = m_channels[channel];
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(channel, &pn)) {
            auto& value = xrpn->values[channel];
            if (value.is_valid()) {
                if (chan.state == xRPN_state::RPN_ACTIVE) {
                    switch (RPN(pn.number())) {

                    case RPN::PITCH_BEND_SENSITIVITY:
                        value.increment_centesimally();
                        break;

                    case RPN::COARSE_TUNING:
                    case RPN::TUNING_PROGRAM_SELECT:
                    case RPN::TUNING_BANK_SELECT:
                        value.increment_msb();
                        break;

                    default:
                        value.increment_value();
                        break;
                    }
                } else {
                    value.increment_value();    // all NRPNs inc the LSB.
                }
                xrpn->broadcast(m_channels[channel].timbres, channel, pn);
            }
        }
    }

    inline void
    Dispatcher::
    handle_data_decrement(const SmallMessage& msg)
    {
        auto channel = msg.channel();
        auto& chan = m_channels[channel];
        ParameterNumber pn;
        if (auto *xrpn = get_xRPN(channel, &pn)) {
            auto& value = xrpn->values[channel];
            if (value.is_valid()) {
                if (chan.state == xRPN_state::RPN_ACTIVE) {
                    switch (RPN(pn.number())) {

                    case RPN::PITCH_BEND_SENSITIVITY:
                        value.decrement_centesimally();
                        break;

                    case RPN::COARSE_TUNING:
                    case RPN::TUNING_PROGRAM_SELECT:
                    case RPN::TUNING_BANK_SELECT:
                        value.decrement_msb();
                        break;

                    default:
                        value.decrement_value();
                        break;
                    }
                } else {
                    value.decrement_value();
                }
                xrpn->broadcast(m_channels[channel].timbres, channel, pn);
            }
        }
    }

    inline void
    Dispatcher::
    handle_rpn_msb(const SmallMessage& msg)
    {
        auto& chan = m_channels[msg.channel()];
        chan.rpn_msb = msg.control_value();
        chan.state = xRPN_state::RPN_ACTIVE;
    }

    inline void
    Dispatcher::
    handle_rpn_lsb(const SmallMessage& msg)
    {
        auto& chan = m_channels[msg.channel()];
        chan.rpn_lsb = msg.control_value();
        chan.state = xRPN_state::RPN_ACTIVE;
    }

    inline void
    Dispatcher::
    handle_nrpn_msb(const SmallMessage& msg)
    {
        auto& chan = m_channels[msg.channel()];
        chan.nrpn_msb = msg.control_value();
        chan.state = xRPN_state::NRPN_ACTIVE;
    }

    inline void
    Dispatcher::
    handle_nrpn_lsb(const SmallMessage& msg)
    {
        auto& chan = m_channels[msg.channel()];
        chan.nrpn_lsb = msg.control_value();
        chan.state = xRPN_state::NRPN_ACTIVE;
    }

    inline
    Dispatcher::xRPN_bundle *
    Dispatcher::
    get_xRPN(std::uint8_t channel, ParameterNumber *pnp)
    {
        auto& chan = m_channels[channel];
        if (chan.state == xRPN_state::RPN_ACTIVE) {
            auto nmsb = chan.rpn_msb, nlsb = chan.rpn_lsb;
            if (nmsb == ParameterNumber::NO_BYTE ||
                nlsb == ParameterNumber::NO_BYTE)
                return nullptr;
            auto pn = ParameterNumber(nmsb, nlsb);
            if (pn.number() >= MAX_RPNS)
                return nullptr;
            if (pnp)
                *pnp = pn;
            return &m_rpns[pn.number()];
        } else if (chan.state == xRPN_state::NRPN_ACTIVE) {
            auto msb = chan.nrpn_msb, lsb = chan.nrpn_lsb;
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

}

#endif /* !MIDI_DISPATCHER_included */
