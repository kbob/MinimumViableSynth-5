#ifndef MIDI_DISPATCHER_included
#define MIDI_DISPATCHER_included

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

#include "synth/midi/defs.h"
#include "synth/midi/layering.h"
#include "synth/midi/messages.h"
#include "synth/midi/param.h"
#include "synth/util/fixed-map.h"
#include "synth/util/function.h"

namespace midi {

    class Dispatcher {

        typedef Layering::channel_index c_index;
        typedef Layering::timbre_index  t_index;
        typedef Layering::channel_mask  c_mask;
        typedef Layering::timbre_mask   t_mask;

        static const t_mask ALL_TIMBRES = (1 << MAX_TIMBRES) - 1;

    public:

        typedef function<void(const SmallMessage&)>  small_handler;
        typedef function<void(std::uint8_t channel,
                              const ParameterNumber&,
                              const ParameterValue&)> xRPN_handler;
        typedef function<void(const SysexMessage&)>  sysex_handler;

        Dispatcher();

        const Layering *layering() const;
        void attach_layering(const Layering&);

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

    private:

        enum class xRPN_state : std::uint8_t {
            INACTIVE,
            RPN_ACTIVE,
            NRPN_ACTIVE,
        };

        struct xRPN_bundle {
            std::array<ParameterValue, CHANNEL_COUNT> values;
            std::array<xRPN_handler, MAX_TIMBRES> handlers;

            void broadcast(t_mask, c_index channel, ParameterNumber);
        };

        struct channel {
            xRPN_state            state;
            ParameterNumber::byte rpn_msb;
            ParameterNumber::byte rpn_lsb;
            ParameterNumber::byte nrpn_msb;
            ParameterNumber::byte nrpn_lsb;

            channel()
            : state{xRPN_state::INACTIVE},
              rpn_msb{ParameterNumber::NO_BYTE},
              rpn_lsb{ParameterNumber::NO_BYTE},
              nrpn_msb{ParameterNumber::NO_BYTE},
              nrpn_lsb{ParameterNumber::NO_BYTE}
            {}

            void reset();
        };

        struct timbre {
            std::array<small_handler, 128> cc_handlers;
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

        const Layering *m_layering;
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
    broadcast(t_mask timbres, c_index channel, ParameterNumber number)
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
    : m_layering{nullptr}
    {
        register_handler(StatusByte::CONTROL_CHANGE,
                         Layering::ALL_CHANNELS,
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

    inline auto
    Dispatcher::
    layering() const
    -> const Layering *
    {
        return m_layering;
    }

    inline void
    Dispatcher::
    attach_layering(const Layering& l)
    {
        m_layering = &l;
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
    register_handler(StatusByte s, c_mask m, const small_handler& h)
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
    register_handler(ControllerNumber cc, t_mask m, const small_handler& h)
    {
        auto index = static_cast<size_t>(cc);
        assert(index < CC_COUNT);
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                m_timbres[ti].cc_handlers[index] = h;
    }

    inline void
    Dispatcher::
    register_handler(RPN rpn, t_mask m, const xRPN_handler& h)
    {
        auto index = static_cast<size_t>(rpn);
        auto& handlers = m_rpns[index].handlers;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                handlers[ti] = h;
    }

    inline void
    Dispatcher::
    register_handler(NRPN nrpn, t_mask m, const xRPN_handler& h)
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
                     t_mask m,
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

    inline void
    Dispatcher::
    handle_cc(const SmallMessage& msg)
    {
        assert(msg.status() == StatusByte::CONTROL_CHANGE);
        auto channel = msg.channel();
        auto cc = msg.control_number();
        auto timbres = m_layering->channel_timbres(channel);
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
            auto timbres = m_layering->channel_timbres(channel);
            xrpn->broadcast(timbres, channel, pn);
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
            auto timbres = m_layering->channel_timbres(channel);
            xrpn->broadcast(timbres, channel, pn);
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
                auto timbres = m_layering->channel_timbres(channel);
                xrpn->broadcast(timbres, channel, pn);
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
                auto timbres = m_layering->channel_timbres(channel);
                xrpn->broadcast(timbres, channel, pn);
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

    inline auto
    Dispatcher::
    get_xRPN(std::uint8_t channel, ParameterNumber *pnp)
    -> xRPN_bundle *
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
