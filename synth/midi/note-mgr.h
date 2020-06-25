#ifndef NOTE_MGR_included
#define NOTE_MGR_included

#include <array>
#include <bitset>
#include <cstdint>

#include "synth/core/assigners.h"
#include "synth/core/synth.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/layering.h"
#include "synth/util/fixed-queue.h"
#include "synth/util/function.h"

class note_manager_unit_test;

namespace midi {

    // -- NoteManager - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
    //
    // There can be more than one timbre per channel.
    // There can be more than one channel per timbre.
    // There can be more than one note per <channel, note, timbre>.
    // There can be more than one voice per <channel, note, timbre>.

    class NoteManager {

    public:

        typedef Layering::channel_index channel_index;
        typedef std::uint8_t            voice_index;
        typedef std::uint8_t            timbre_index;

        // Each channel can be polyphonic or monophonic.
        enum class Mode : std::uint8_t {
            POLY,
            MONO,
        };

        // NoteManager emits these events.
        typedef function<void(std::uint8_t)> portamento_note_handler;
        typedef function<void(std::uint8_t)> note_number_handler;
        typedef function<void(std::uint16_t)> attack_velocity_handler;
        typedef function<void(std::uint8_t)> poly_pressure_handler;
        typedef function<void(std::uint8_t)> release_velocity_handler;

        NoteManager();

        // A note manager needs a Dispatcher, a Synth, and an Assigner
        // to function.
        void attach_dispatcher(Dispatcher *);
        void attach_synth(::Synth *);
        void attach_assigner(::Assigner *);

        // Voice controls can register handler for these events.
        void register_portamento_note_handler (voice_index,
                                               portamento_note_handler);
        void register_note_number_handler     (voice_index,
                                               note_number_handler);
        void register_attack_velocity_handler (voice_index,
                                               attack_velocity_handler);
        void register_poly_pressure_handler   (voice_index,
                                               poly_pressure_handler);
        void register_release_velocity_handler(voice_index,
                                               release_velocity_handler);

        // Channel mode control
        Mode channel_mode(channel_index) const;
        void channel_mode(channel_index, Mode);

        // Channel reset messages.
        void all_sound_off();
        void reset_all_controllers();
        void all_notes_off();
        void all_notes_off(channel_index);

        void render();

    private:

        // operations on voices
        //   - allocate/start
        //   - release
        //   - deallocate
        //   - steal
        //   - pass message to (note off, poly key pressure)

        typedef std::uint8_t            note_number;
        typedef std::bitset<NOTE_COUNT> note_mask;
        typedef Layering::timbre_mask   timbre_mask;

        static const voice_index   NO_VOICE      =   ~0;
        static const channel_index NO_CHANNEL    = 0xFF;
        static const note_number   NO_NOTE       = 0xFF;
        static const std::uint8_t  NO_VELOCITY   = 0xFF;
        static const std::uint8_t  NO_PORTAMENTO = 0xFF;

        // note_start_info: state needed to start a new note
        struct note_start_info {
            timbre_mask              timbres;
            std::uint8_t             note_number;
            std::uint16_t            attack_velocity;
            std::uint8_t             portamento_note;
        };

        // per-channel data
        struct channel_data {
            Mode                     mode;
            std::uint8_t             velocity_lsb;
            std::uint8_t             portamento_note;
            bool                     is_sustaining;
            std::uint8_t             mono_note;
            note_mask                notes_on;
            note_mask                notes_sustaining;
            note_mask                notes_sostenuto;

            channel_data()
            : mode{Mode::POLY},
              velocity_lsb{NO_VELOCITY},
              portamento_note{NO_NOTE},
              is_sustaining{false},
              mono_note{NO_NOTE}
            {}

            bool note_should_sound(note_number ni)
            {
                return !(notes_on[ni] ||
                         notes_sustaining[ni] ||
                         notes_sostenuto[ni]);
            }
        };

        // per-voice data
        struct voice_data {
            std::uint8_t             channel;
            std::uint8_t             note;
            portamento_note_handler  portamento_note_handler;
            note_number_handler      note_number_handler;
            attack_velocity_handler  attack_velocity_handler;
            poly_pressure_handler    poly_pressure_handler;
            release_velocity_handler release_velocity_handler;

            voice_data()
            : channel{NO_CHANNEL},
              note{NO_NOTE}
            {}
        };

        // per-timbre info
        struct timbre_data {
            voice_index              mono_voice;

            timbre_data()
            : mono_voice{NO_VOICE}
            {}
        };

        // NoteManager consumes these MIDI messages.
        void handle_note_on_message(const SmallMessage&);
        void handle_note_off_message(const SmallMessage&);
        void handle_poly_pressure_message(const SmallMessage&);
        void handle_damper_pedal_message(const SmallMessage&);
        void handle_sostenuto_message(const SmallMessage&);
        void handle_high_res_velocity_message(const SmallMessage&);
        void handle_portamento_control_message(const SmallMessage&);
        // void handle_all_notes_off_message(const SmallMessage&);

        // Auxilliary types needed to construct handlers
        template <void (NoteManager::*M)(const SmallMessage&)>
            using small_binding =
                Dispatcher::small_handler::binding<NoteManager, M>;
        using note_on_binding =
            small_binding<&NoteManager::handle_note_on_message>;
        using note_off_binding =
            small_binding<&NoteManager::handle_note_off_message>;
        using poly_pressure_binding =
            small_binding<&NoteManager::handle_poly_pressure_message>;
        using damper_pedal_binding =
            small_binding<&NoteManager::handle_damper_pedal_message>;
        using sostenuto_binding =
            small_binding<&NoteManager::handle_sostenuto_message>;
        using high_res_velocity_binding =
            small_binding<&NoteManager::handle_high_res_velocity_message>;
        using portamento_control_binding =
            small_binding<&NoteManager::handle_portamento_control_message>;
        // using all_notes_off_binding =
        //     small_binding<&NoteManager::handle_all_notes_off_message>;

        // Voice control
        void start_note(Voice *, timbre_index, const note_start_info&);
        void retrigger_note(Voice *, std::uint16_t velocity);
        void release_note(channel_index,
                          note_number,
                          std::uint8_t velocity = 0);

        // Member Variables
        ::Synth                                 *m_synth;
        ::Assigner                              *m_assigner;
        Dispatcher                              *m_dispatcher;
        const Layering                          *m_layering;
        std::array<channel_data, CHANNEL_COUNT>  m_channels;
        fixed_vector<voice_data, MAX_VOICES>     m_voices;
        fixed_vector<timbre_data, MAX_TIMBRES>   m_timbres;
        fixed_queue<note_start_info, MAX_VOICES> m_pending_notes;
        fixed_queue<Voice *, MAX_VOICES>         m_killed_voices;

        static_assert(MAX_VOICES < std::numeric_limits<voice_index>::max(),
                      "voice_index too small");

        friend class ::note_manager_unit_test;

    };


    // -- NoteManager - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline
    NoteManager::
    NoteManager()
    : m_synth{nullptr},
      m_assigner{nullptr},
      m_dispatcher{nullptr},
      m_layering{nullptr}
    {}

    inline void
    NoteManager::
    attach_dispatcher(Dispatcher *d)
    {
        m_dispatcher = d;
        m_layering = d->layering();
        if (m_synth)
            assert(m_layering->timbrality == m_synth->timbrality);
        auto all_timbres = m_layering->all_timbres;
        assert(m_layering);
        d->register_handler(StatusByte::NOTE_ON,
                            Layering::ALL_CHANNELS,
                            note_on_binding(this));
        d->register_handler(StatusByte::NOTE_OFF,
                            Layering::ALL_CHANNELS,
                            note_off_binding(this));
        d->register_handler(StatusByte::POLY_KEY_PRESSURE,
                            Layering::ALL_CHANNELS,
                            poly_pressure_binding(this));
        d->register_handler(ControllerNumber::DAMPER_PEDAL,
                            all_timbres,
                            damper_pedal_binding(this));
        d->register_handler(ControllerNumber::SOSTENUTO,
                            all_timbres,
                            sostenuto_binding(this));
        d->register_handler(ControllerNumber::HIGH_RESOLUTION_VELOCITY_PREFIX,
                            all_timbres,
                            high_res_velocity_binding(this));
        d->register_handler(ControllerNumber::PORTAMENTO_CONTROL,
                            all_timbres,
                            portamento_control_binding(this));
        // d->register_handler(ChannelModeNumber::ALL_NOTES_OFF,
        //                     all_timbres,
        //                     all_notes_off_binding(this));
    }

    inline void
    NoteManager::
    attach_synth(Synth *s)
    {
        if (m_layering)
            assert(m_layering->timbrality == s->timbrality);
        m_synth = s;
        m_voices.resize(s->polyphony);
        m_timbres.resize(s->timbrality);
    }

    inline void
    NoteManager::
    attach_assigner(Assigner *a)
    {
        m_assigner = a;
    }

    inline void
    NoteManager::
    register_portamento_note_handler(voice_index vi, portamento_note_handler h)
    {
        if (vi >= m_voices.size())
            m_voices.resize(vi + 1);
        m_voices[vi].portamento_note_handler = h;
    }

    inline void
    NoteManager::
    register_note_number_handler(voice_index vi, note_number_handler h)
    {
        if (vi >= m_voices.size())
            m_voices.resize(vi + 1);
        m_voices[vi].note_number_handler = h;
    }

    inline void
    NoteManager::
    register_attack_velocity_handler(voice_index vi, attack_velocity_handler h)
    {
        if (vi >= m_voices.size())
            m_voices.resize(vi + 1);
        m_voices[vi].attack_velocity_handler = h;
    }

    inline void
    NoteManager::
    register_poly_pressure_handler(voice_index vi, poly_pressure_handler h)
    {
        if (vi >= m_voices.size())
            m_voices.resize(vi + 1);
        m_voices[vi].poly_pressure_handler = h;
    }

    inline void
    NoteManager::
    register_release_velocity_handler(voice_index vi,
                                      release_velocity_handler h)
    {
        if (vi >= m_voices.size())
            m_voices.resize(vi + 1);
        m_voices[vi].release_velocity_handler = h;
    }

    inline auto
    NoteManager::
    channel_mode(channel_index ci) const
    -> Mode
    {
        return m_channels[ci].mode;
    }

    inline void
    NoteManager::
    channel_mode(channel_index ci, Mode m)
    {
        auto& current_mode = m_channels[ci].mode;
        if(current_mode != m) {
            all_notes_off(ci);
            current_mode = m;
        }
    }

    inline void
    NoteManager::
    all_sound_off()
    {
        for (auto& v: m_synth->voices())
            if (v.state() != Voice::State::IDLE &&
                v.state() != Voice::State::STOPPING)
                v.kill_note();
    }

    inline void
    NoteManager::
    reset_all_controllers()
    {
        assert(!"write me!");
        // for all voices
        // for all controls,
        // if it's a MIDI control,
        // reset it.
        // also reset all channels' portamento note, velocity lsb,
        // and poly pressure.
        // Should sustain and sostenuto be reset?
    }

    inline void
    NoteManager::
    all_notes_off()
    {
        for (channel_index ci = 0; ci < CHANNEL_COUNT; ci++)
            all_notes_off(ci);
    }

    inline void
    NoteManager::
    all_notes_off(channel_index ci)
    {
        auto& chan = m_channels[ci];
        auto notes_on = chan.notes_on;
        chan.notes_on.reset();
        for (size_t ni = 0; ni < notes_on.size(); ni++)
            if (notes_on[ni] && !chan.note_should_sound(ni))
                release_note(ci, ni);
        // XXX also clear the pending note queue.
    }

    inline void
    NoteManager::
    render()
    {
        // Find killed voices that have finally died and reuse them.
        while (!m_killed_voices.empty()) {
            Voice *voice = m_killed_voices.front();
            if (voice->state() != Voice::State::IDLE)
                break;
            // XXX clear timbre.mono_voice if applicable.
            m_killed_voices.pop();
            if (m_pending_notes.empty())
                continue;

            // We have an idle voice and a pending note.
            // Start the note on the voice.
            auto& note_info = m_pending_notes.front();
            for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
                if (note_info.timbres & (1 << ti)) {
                    start_note(voice, ti, note_info);
                    note_info.timbres &= ~(1 << ti);
                    if (!note_info.timbres)
                        m_pending_notes.pop();
                }
            }
        }
    }

    inline void
    NoteManager::
    handle_note_on_message(const SmallMessage& msg)
    {
        if (msg.velocity() == 0) {
            // This is actually a note off event.
            return handle_note_off_message(msg);
        }

        std::uint8_t ci = msg.channel();
        std::uint8_t note = msg.note_number();
        std::uint8_t v = msg.velocity();

        auto timbres = m_layering->channel_timbres(ci);

        auto& chan = m_channels[ci];
        chan.notes_on.set(note);
        if (chan.is_sustaining)
            chan.notes_sustaining.set(note);
        std::uint16_t vel;
        if (chan.velocity_lsb == NO_VELOCITY)
            vel = v << 7 | v;
        else {
            vel = v << 7 | chan.velocity_lsb;
            chan.velocity_lsb = NO_VELOCITY;
        }
        auto port_note = chan.portamento_note;
        chan.portamento_note = NO_PORTAMENTO;


        note_start_info info;
        info.timbres = timbres;
        info.note_number = note;
        info.attack_velocity = vel;
        info.portamento_note = port_note;

        switch (chan.mode) {

        case Mode::POLY:
            // XXX really need a way to iterate through a
            //     channel's timbres.
            //     Now that Layering is a nice abstraction,
            //     maybe I could swap it out for one that
            //     iterates channel timbres faster.
            for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
                if (timbres & (1 << ti)) {
                    Voice *voice = m_assigner->assign_idle_voice();
                    if (voice) {
                        start_note(voice, ti, info);
                        info.timbres &= ~(1 << ti);
                    } else
                        break;
                }
            }
            if (info.timbres) {
                for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
                    Voice *voice = m_assigner->choose_voice_to_steal();
                    if (voice) {
                        voice->kill_note();
                        m_killed_voices.push(voice);
                    }
                }
                if (m_pending_notes.full())
                    m_pending_notes.pop();  // discard oldest
                m_pending_notes.push(info);
            }
            break;

        case Mode::MONO:
            for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
                if (timbres & (1 << ti)) {
                    auto& timbre = m_timbres[ti];
                    if (!timbre.mono_voice) {

                    }
                    // ... m_timbres[ti].mono_voice ...
                    // if mono_voice is non-null,
                    //    allocate it and push to pending.
                    // else
                    //     change mono_voice's portamento note,
                    //     attack velocity,
                    //     note number
                    timbres &= ~(1 << ti);
                }
            }
            break;
        }
    }

    inline void
    NoteManager::
    handle_note_off_message(const SmallMessage& msg)
    {
        // N.B., status may either be NOTE_OFF or NOTE_ON,
        // because NOTE_ON messages w/ velocity 0 mean note off.
        channel_index ci = msg.channel();
        note_number note = msg.note_number();
        std::uint8_t vel = msg.velocity();

        auto& chan = m_channels[ci];
        chan.notes_on.reset(note);

        // linear search, yuck!
        if (!chan.note_should_sound(note))
            for (auto& v_data: m_voices)
                if (v_data.channel == ci && v_data.note == note)
                    release_note(ci, note, vel);
    }

    inline void
    NoteManager::
    handle_poly_pressure_message(const SmallMessage& msg)
    {
        channel_index ci = msg.channel();
        note_number note = msg.note_number();
        auto pressure = msg.poly_pressure();
        for (auto& v_data: m_voices)
            if (v_data.channel == ci && v_data.note == note)
                if (auto& h = v_data.poly_pressure_handler)
                    h(pressure);

    }

    inline void
    NoteManager::
    handle_damper_pedal_message(const SmallMessage& msg)
    {
        channel_index ci = msg.channel();
        bool is_sustaining = msg.switch_value();

        auto& chan = m_channels[ci];
        bool was_sustaining = chan.is_sustaining;

        if (is_sustaining && !was_sustaining) {
            chan.notes_sustaining = chan.notes_on;
            chan.is_sustaining = true;
        } else if (was_sustaining && !is_sustaining) {
            chan.notes_sustaining.reset();
            for (auto& v_data: m_voices) {
                if (v_data.channel == ci) {
                    auto note = v_data.note;
                    if (!chan.note_should_sound(note))
                        release_note(ci, note);
                }
            }
        }
    }

    inline void
    NoteManager::
    handle_sostenuto_message(const SmallMessage& msg)
    {
        channel_index ci = msg.channel();
        bool is_sostenuto = msg.switch_value();

        auto& chan = m_channels[ci];

        if (is_sostenuto) {
            chan.notes_sostenuto = chan.notes_on;
        } else {
            chan.notes_sostenuto.reset();
            for (auto& v_data: m_voices) {
                if (v_data.channel == ci) {
                    auto note = v_data.note;
                    if (!chan.note_should_sound(note))
                        release_note(ci, note);
                }
            }
        }
    }

    inline void
    NoteManager::
    handle_high_res_velocity_message(const SmallMessage& msg)
    {
        channel_index ci = msg.channel();
        auto& chan = m_channels[ci];
        chan.velocity_lsb = msg.control_value();
    }

    inline void
    NoteManager::
    handle_portamento_control_message(const SmallMessage& msg)
    {
        channel_index ci = msg.channel();
        auto& chan = m_channels[ci];
        chan.portamento_note = msg.control_value();
    }

    // inline void
    // NoteManager::
    // handle_all_notes_off_message(const SmallMessage& msg)
    // {
    //     all_notes_off(msg.channel());
    // }

    inline void
    NoteManager::
    start_note(Voice *voice, timbre_index ti, const note_start_info& info)
    {
        voice_index vi = m_synth->voices().data() - voice;
        assert(0 <= vi && vi < MAX_VOICES);
        auto& v_data = m_voices[vi];
        Timbre& timbre = m_synth->timbres()[ti];

        m_synth->attach_voice_to_timbre(timbre, *voice);
        if (auto& h = v_data.note_number_handler)
            h(info.note_number);
        if (auto& h = v_data.attack_velocity_handler)
            h(info.attack_velocity);
        if (info.portamento_note != NO_NOTE)
            if (auto& h = v_data.portamento_note_handler)
                h(info.portamento_note);
        voice->start_note();
    }

    inline void
    NoteManager::
    retrigger_note(Voice *, std::uint16_t velocity)
    {
        (void)velocity;
    }

    inline void
    NoteManager::
    release_note(channel_index, note_number, std::uint8_t velocity)
    {
        (void)velocity;
    }


    // XXX how do we keep the assigner from stealing the mono voices?
    //     need some concept of protected voice?
    //     Answer: we don't  steal the mono voice.  If mono timbre A
    //     is not playing, let poly timbre B use the voice.

}

#endif /* !NOTE_MGR_included */
