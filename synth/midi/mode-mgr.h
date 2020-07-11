#ifndef MIDI_MODE_MGR_included
#define MIDI_MODE_MGR_included

#include <algorithm>
#include <cassert>
#include <cstdint>

#include "synth/midi/defs.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/layering.h"
#include "synth/midi/note-mgr.h"

// The mode manager handles MIDI modes.
// Modes are Omni/not Omni, Mono/Poly, and Multi.

namespace midi {

    // -- ModeManager - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
    //
    // The ModeManager manages channel modes.
    //
    // The MIDI spec defines four modes: omni poly, omni mono, poly, and
    // mono.  They are also known as Mode 1 through Mode 4,
    // respectively.  We add a fifth mode, Multitimbral Mode.
    //
    // One MIDI channel is designated as the Basic Channel.  It is
    // channel 1 by default.  The four classic modes, 1 through 4,
    // are selected by channel mode messages on the basic channel.
    //
    // In Mode 1, Omni Poly Mode, the first timbre plays polyphonically
    // using all available voices. The other timbres are inactive.
    // The voices are controlled by voice messages from all channels.
    //
    // In Mode 2, Omni Mono Mode, the first timbre plays a single
    // monophonic voice.  The other timbres are inactive.  The voice is
    // controlled by voice messages from all channels.
    //
    // In Mode 3, (non-omni) Poly Mode, the synth only responds to voice
    // messages on the basic channel.  The channel is mapped to the
    // first timbre through the Layering object.  The other timbres
    // are inactive.
    //
    // In Mode 4, (non-omni) Mono Mode, the synth responds to voice
    // messages on channels Basic Channel through Basic Channel + N.
    // The channels are all mapped to the first timbre.  The other
    // timbres are inactive.
    //
    // The parameter N is specified in the MIDI Mono Mode On message.
    // If Basic Channel + N is more than 16, the acrive channel numbers
    // wrap around.
    //
    // When Multitimbral Mode is enabled, it overrides modes 1-4.
    // In Multitimbral Mode, voice messages are processed on all
    // channels, and they control timbres according to the Layering.
    // Each channel has its own mono/poly setting, determined by
    // the Mono Mode On and Poly Mode On MIDI messages
    // The Basic Channel is not used in multi mode.
    //
    // There is not yet a way to enable/disable multi mode through
    // MIDI messages.
    //
    //   XXX Need to handle Legato Footswitch CC here.
    //
    //
    // Thanks to http://www.philrees.co.uk/articles/midimode.htm
    // for clarifying my understanding of these modes.

    class ModeManager {

        typedef Layering::channel_index c_index;
        typedef Layering::channel_mask  c_mask;

    public:

        typedef c_index channel_index;

        ModeManager();

        void attach_dispatcher(Dispatcher&);
        void attach_layering(Layering&);
        void attach_note_manager(NoteManager&);

        channel_index basic_channel() const;
        void basic_channel(channel_index);

        ChannelMode mode() const;
        bool is_omni() const;
        bool is_mono() const;
        bool is_mono(channel_index) const;
        bool is_multi() const;
        c_mask mode4_active_channels() const;

        void multi_mode(bool);

    private:

        void update(ChannelMode prev_mode);

        void handle_omni_off_message(const SmallMessage&);
        void handle_omni_on_message(const SmallMessage&);
        void handle_mono_on_message(const SmallMessage&);
        void handle_poly_on_message(const SmallMessage&);

        template <void (ModeManager::*M)(const SmallMessage&)>
            using small_binding =
                Dispatcher::small_handler::binding<ModeManager, M>;
        using omni_off_binding =
            small_binding<&ModeManager::handle_omni_off_message>;
        using omni_on_binding =
            small_binding<&ModeManager::handle_omni_on_message>;
        using mono_on_binding =
            small_binding<&ModeManager::handle_mono_on_message>;
        using poly_on_binding =
            small_binding<&ModeManager::handle_poly_on_message>;

        struct classic_state {
            c_index     basic_channel;
            c_index     mode4_channel_count;
            // ChannelMode mode;
            bool        is_omni;
            bool        is_mono;

            classic_state()
            : basic_channel{0},
              mode4_channel_count{CHANNEL_COUNT},
              is_omni{true},
              is_mono{false}
            {}
        };

        struct multi_state {
            c_mask      mono_channels;

            multi_state()
            : mono_channels{0}
            {}
        };

        Dispatcher     *m_dispatcher;
        Layering       *m_layering;
        NoteManager    *m_note_manager;
        bool            m_is_multi;
        classic_state   m_classic;
        multi_state     m_multi;

    };

    inline ModeManager::ModeManager()
    : m_dispatcher{nullptr},
      m_layering{nullptr},
      m_note_manager{nullptr},
      m_is_multi{false}
    {}

    inline void
    ModeManager::
    attach_dispatcher(Dispatcher& d)
    {
        m_dispatcher = &d;
        assert(d.layering());
        if (m_layering)
            assert(m_layering == d.layering());
        auto all_timbres = d.layering()->all_timbres;
        d.register_handler(ChannelModeNumber::OMNI_MODE_OFF,
                           all_timbres,
                           omni_off_binding(this));
        d.register_handler(ChannelModeNumber::OMNI_MODE_ON,
                           all_timbres,
                           omni_on_binding(this));
        d.register_handler(ChannelModeNumber::MONO_MODE_ON,
                           all_timbres,
                           mono_on_binding(this));
        d.register_handler(ChannelModeNumber::POLY_MODE_ON,
                           all_timbres,
                           poly_on_binding(this));
    }

    inline void
    ModeManager::
    attach_layering(Layering& l)
    {
        m_layering = &l;
        if (m_dispatcher && m_dispatcher->layering())
            assert(&l == m_dispatcher->layering());
    }

    inline void
    ModeManager::
    attach_note_manager(NoteManager& nm)
    {
        m_note_manager = &nm;
    }

    inline auto
    ModeManager::
    basic_channel() const
    -> channel_index
    {
        return m_classic.basic_channel;
    }

    inline void
    ModeManager::
    basic_channel(channel_index ci)
    {
        assert(ci < CHANNEL_COUNT);
        m_classic.basic_channel = ci;
        update(mode());
    }

    inline auto
    ModeManager::
    mode() const
    -> ChannelMode
    {
        if (m_is_multi)
            return ChannelMode::MULTI;
        else {
            if (m_classic.is_omni && !m_classic.is_mono)
                return ChannelMode::OMNI_POLY;
            if (m_classic.is_omni && m_classic.is_mono)
                return ChannelMode::OMNI_MONO;
            if (!m_classic.is_omni && !m_classic.is_mono)
                return ChannelMode::POLY;
            if (!m_classic.is_omni && m_classic.is_mono)
                return ChannelMode::MONO;
        }
        assert(!"can't happen");
        return ChannelMode::OMNI_POLY;
    }

    inline auto
    ModeManager::
    is_omni() const
    -> bool
    {
        return !m_is_multi && m_classic.is_omni;
    }

    inline auto
    ModeManager::
    is_mono() const
    -> bool
    {
        return !m_is_multi && m_classic.is_mono;
    }

    inline auto
    ModeManager::
    is_multi() const
    -> bool
    {
        return m_is_multi;
    }

    inline auto
    ModeManager::
    mode4_active_channels() const
    -> c_mask
    {
        std::uint32_t m = (1 << m_classic.mode4_channel_count) - 1;
        m <<= m_classic.basic_channel;
        m = (m & Layering::ALL_CHANNELS) | (m >> CHANNEL_COUNT);
        return m;
    }

    inline void
    ModeManager::
    multi_mode(bool m)
    {
        auto prev_mode = mode();
        m_is_multi = m;
        update(prev_mode);
    }

    inline void
    ModeManager::
    handle_omni_off_message(const SmallMessage& msg)
    {
        if (msg.channel() != m_classic.basic_channel)
            return;
        auto prev_mode = mode();
        m_classic.is_omni = false;

        if (mode() != prev_mode)
            update(prev_mode);
    }

    inline void
    ModeManager::
    handle_omni_on_message(const SmallMessage& msg)
    {
        if (msg.channel() != m_classic.basic_channel)
            return;
        auto prev_mode = mode();
        m_classic.is_omni = true;

        if (mode() != prev_mode)
            update(prev_mode);
    }

    inline void
    ModeManager::
    handle_mono_on_message(const SmallMessage& msg)
    {
        auto prev_mode = mode();
        auto prev_mode4_channel_count = m_classic.mode4_channel_count;
        auto prev_mono_channels = m_multi.mono_channels;
        if (msg.channel() == m_classic.basic_channel) {
            m_classic.is_mono = true;
            auto m = msg.control_value();
            if (m == 0)
                m = CHANNEL_COUNT;
            m_classic.mode4_channel_count = m;
        }
        m_multi.mono_channels |= 1 << msg.channel();

        if (mode() != prev_mode)
            update(prev_mode);
        else if (is_multi()) {
            if (m_multi.mono_channels != prev_mono_channels)
                update(prev_mode);
        } else {
            if (m_classic.mode4_channel_count != prev_mode4_channel_count)
                update(prev_mode);
        }
    }

    inline void
    ModeManager::
    handle_poly_on_message(const SmallMessage& msg)
    {
        auto prev_mode = mode();
        auto prev_mono_channels = m_multi.mono_channels;
        if (msg.channel() == m_classic.basic_channel)
            m_classic.is_mono = false;
        m_multi.mono_channels &= ~(1 << msg.channel());
        if (mode() != prev_mode)
            update(prev_mode);
        else if (m_multi.mono_channels != prev_mono_channels)
            update(prev_mode);
    }

    inline void
    ModeManager::
    update(ChannelMode prev_mode)
    {
        assert(m_layering);
        assert(m_note_manager);
        switch (mode()) {

        case ChannelMode::OMNI_POLY:
            m_layering->omni_mode();
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                m_note_manager->channel_mode(ci, NoteManager::Mode::POLY);
            break;

        case ChannelMode::OMNI_MONO:
            m_layering->omni_mode();
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                m_note_manager->channel_mode(ci, NoteManager::Mode::MONO);
            break;

        case ChannelMode::POLY:
            {
                auto bc = m_classic.basic_channel;
                m_layering->poly_mode(bc);
                m_note_manager->channel_mode(bc, NoteManager::Mode::POLY);
            }
            break;

        case ChannelMode::MONO:
            m_layering->mono_mode(mode4_active_channels());
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                m_note_manager->channel_mode(ci, NoteManager::Mode::MONO);
            break;

        case ChannelMode::MULTI:
            if (prev_mode != ChannelMode::MULTI)
                m_layering->multi_mode();
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++) {
                auto mode = m_multi.mono_channels & (1 << ci)
                    ? NoteManager::Mode::MONO
                    : NoteManager::Mode::POLY;
                m_note_manager->channel_mode(ci, mode);
            }
            break;
        }
    }

}

#endif /* !MIDI_MODE_MGR_included */
