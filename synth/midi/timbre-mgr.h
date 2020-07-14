#ifndef MIDI_TIMBRE_MGR_included
#define MIDI_TIMBRE_MGR_included

#include <cstdint>

#include "synth/midi/dispatcher.h"
#include "synth/midi/layering.h"
#include "synth/midi/sizes.h"

namespace midi {

    class TimbreManager {

        using t_mask = Layering::timbre_mask;

    public:

        using timbre_mask = t_mask;
        using small_handler = Dispatcher::small_handler;
        typedef std::uint8_t                          channel_pressure_type;
        typedef std::int16_t                          pitch_bend_type;
        typedef function<void(channel_pressure_type)> channel_pressure_handler;
        typedef function<void(pitch_bend_type)>       pitch_bend_handler;

        TimbreManager();
        void attach_dispatcher(Dispatcher&);

        void
        register_channel_pressure_handler(t_mask, channel_pressure_handler);
        void
        register_pitch_bend_handler(t_mask, pitch_bend_handler);

    private:

        struct handlers {
            channel_pressure_handler channel_pressure;
            pitch_bend_handler       pitch_bend;
        };

        void handle_channel_pressure_message(const SmallMessage&);
        void handle_pitch_bend_message(const SmallMessage&);

        Dispatcher *m_dispatcher;
        std::array<handlers, MAX_TIMBRES> m_handlers;

    };

    inline
    TimbreManager::
    TimbreManager()
    : m_dispatcher{nullptr}
    {}

    inline void
    TimbreManager::
    attach_dispatcher(Dispatcher& d)
    {
        assert(!m_dispatcher);
        m_dispatcher = &d;

        auto cp_handler =
            small_handler::binding<
                TimbreManager,
                &TimbreManager::handle_channel_pressure_message
            >(this);
        auto pb_handler =
            small_handler::binding<
                TimbreManager,
                &TimbreManager::handle_pitch_bend_message
            >(this);

        d.register_handler(StatusByte::CHANNEL_PRESSURE,
                           Layering::ALL_CHANNELS,
                           cp_handler);
        d.register_handler(StatusByte::PITCH_BEND,
                           Layering::ALL_CHANNELS,
                           pb_handler);
    }

    inline void
    TimbreManager::
    register_channel_pressure_handler(t_mask m, channel_pressure_handler h)
    {
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                m_handlers[ti].channel_pressure = h;
    }

    inline void
    TimbreManager::
    register_pitch_bend_handler(t_mask m, pitch_bend_handler h)
    {
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            if (m & (1 << ti))
                m_handlers[ti].pitch_bend = h;
    }

    inline void
    TimbreManager::
    handle_channel_pressure_message(const SmallMessage& msg)
    {
        assert(msg.status() == StatusByte::CHANNEL_PRESSURE);
        auto channel = msg.channel();
        auto pressure = msg.channel_pressure();

        assert(m_dispatcher);
        assert(m_dispatcher->layering());
        auto layering = m_dispatcher->layering();
        auto timbres = layering->channel_timbres(channel);
        auto timbrality = layering->timbrality;
        for (size_t ti = 0; ti < timbrality; ti++)
            if (timbres & (1 << ti))
                if (auto& h = m_handlers[ti].channel_pressure)
                    h(pressure);
    }

    inline void
    TimbreManager::
    handle_pitch_bend_message(const SmallMessage& msg)
    {
        assert(msg.status() == StatusByte::PITCH_BEND);
        auto channel = msg.channel();
        auto bend = msg.bend();

        assert(m_dispatcher);
        assert(m_dispatcher->layering());
        auto layering = m_dispatcher->layering();
        auto timbres = layering->channel_timbres(channel);
        auto timbrality = layering->timbrality;

        for (size_t ti = 0; ti < timbrality; ti++)
            if (timbres & (1 << ti))
                if (auto& h = m_handlers[ti].pitch_bend)
                    h(bend);
    }

}

#endif /* MIDI_TIMBRE_MGR_included */
