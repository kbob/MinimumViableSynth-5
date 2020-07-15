#ifndef MIDI_CONFIG_included
#define MIDI_CONFIG_included

#include "synth/core/config.h"
#include "synth/midi/defs.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/note-mgr.h"
#include "synth/midi/timbre-mgr.h"

class Timbre;
class Voice;

namespace midi {

    class Config : public ::Config::Subsystem {

    public:

        Config(Dispatcher&, NoteManager&, TimbreManager&);

        void pre_configure(::Timbre&) const override;
        void post_configure(::Timbre&) const override;

        void pre_configure(::Voice&) const override;
        void post_configure(::Voice&) const override;

        // handlers called from Dispatcher correspond to MIDI messages.
        void register_handler(ControllerNumber,
                              const Dispatcher::small_handler&);
        void register_handler(RPN, const Dispatcher::xRPN_handler&);
        void register_handler(NRPN, const Dispatcher::xRPN_handler&);

        // handlers called from NoteManager correspond to note events.
        void register_portamento_note_handler(
            const NoteManager::portamento_note_handler&
        );
        void register_note_number_handler(
            const NoteManager::note_number_handler&
        );
        void register_attack_velocity_handler(
            const NoteManager::attack_velocity_handler&
        );
        void register_poly_pressure_handler(
            const NoteManager::poly_pressure_handler&
        );
        void register_release_velocity_handler(
            const NoteManager::release_velocity_handler&
        );

        // handlers called from TimbreManager correspond to channel events.
        void register_channel_pressure_handler(
            const TimbreManager::channel_pressure_handler&
        );
        void register_pitch_bend_handler(
            const TimbreManager::pitch_bend_handler&
        );

    private:

        Dispatcher&    m_dispatcher;
        NoteManager&   m_note_mgr;
        TimbreManager& m_timbre_mgr;
        mutable bool   m_in_timbre;
        mutable bool   m_in_voice;
        mutable size_t m_timbre_index;
        mutable size_t m_voice_index;

    };

    inline
    Config::Config(Dispatcher& d, NoteManager& n_mgr, TimbreManager& t_mgr)
    : m_dispatcher{d},
      m_note_mgr{n_mgr},
      m_timbre_mgr{t_mgr},
      m_in_timbre{false},
      m_in_voice{false},
      m_timbre_index{0},
      m_voice_index{0}
    {}

    inline void
    Config::
    pre_configure(::Timbre&) const
    {
        assert(m_timbre_index < MAX_TIMBRES);
        m_in_timbre = true;
    }

    inline void
    Config::
    post_configure(::Timbre&) const
    {
        m_in_timbre = false;
        m_timbre_index++;
    }

    inline void
    Config::
    pre_configure(::Voice&) const
    {
        assert(m_voice_index < MAX_VOICES);
        m_in_voice = true;
    }

    inline void
    Config::
    post_configure(::Voice&) const
    {
        m_in_voice = false;
        m_voice_index++;
    }

    inline void
    Config::
    register_handler(ControllerNumber cc, const Dispatcher::small_handler& h)
    {
        assert(m_in_timbre);
        m_dispatcher.register_handler(cc, 1 << m_timbre_index, h);
    }

    inline void
    Config::
    register_handler(RPN rpn, const Dispatcher::xRPN_handler& h)
    {
        assert(m_in_timbre);
        m_dispatcher.register_handler(rpn, 1 << m_timbre_index, h);
    }

    inline void
    Config::
    register_handler(NRPN nrpn, const Dispatcher::xRPN_handler& h)
    {
        assert(m_in_timbre);
        m_dispatcher.register_handler(nrpn, 1 << m_timbre_index, h);
    }

    inline void
    Config::
    register_portamento_note_handler(
            const NoteManager::portamento_note_handler& h
        )
    {
        assert(m_in_voice);
        m_note_mgr.register_portamento_note_handler(1 << m_voice_index, h);
    }

    inline void
    Config::
    register_note_number_handler(
            const NoteManager::note_number_handler& h
        )
    {
        assert(m_in_voice);
        m_note_mgr.register_note_number_handler(1 << m_voice_index, h);
    }

    inline void
    Config::
    register_attack_velocity_handler(
            const NoteManager::attack_velocity_handler& h
        )
    {
        assert(m_in_voice);
        m_note_mgr.register_attack_velocity_handler(1 << m_voice_index, h);
    }

    inline void
    Config::
    register_poly_pressure_handler(
            const NoteManager::poly_pressure_handler& h
        )
    {
        assert(m_in_voice);
        m_note_mgr.register_poly_pressure_handler(1 << m_voice_index, h);
    }

    inline void
    Config::
    register_release_velocity_handler(
            const NoteManager::release_velocity_handler& h
        )
    {
        assert(m_in_voice);
        m_note_mgr.register_release_velocity_handler(1 << m_voice_index, h);
    }

    inline void
    Config::
    register_channel_pressure_handler(
            const TimbreManager::channel_pressure_handler& h
        )
    {
        assert(m_in_timbre);
        m_timbre_mgr.register_channel_pressure_handler(1 << m_timbre_index, h);
    }

    inline void
    Config::
    register_pitch_bend_handler(
            const TimbreManager::pitch_bend_handler& h
        )
    {
        assert(m_in_timbre);
        m_timbre_mgr.register_pitch_bend_handler(1 << m_timbre_index, h);
    }

}

#endif /* !MIDI_CONFIG_included */
