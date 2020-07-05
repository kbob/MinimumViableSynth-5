#ifndef MIDI_NOTE_MGR_included
#define MIDI_NOTE_MGR_included

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>

#include "synth/core/assigners.h"
#include "synth/core/synth.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/layering.h"
#include "synth/util/bits.h"
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
    //
    // Is monophonic a property of a channel or a timbre?

    // Gadzooks!
    //  - poly vs mono
    //  - legato vs staccato
    //  - voice preallocated vs available vs unavailable
    //  - previous note sounding vs. releasing
    //  - channel vs timbre
    //  - poly key pressure
    //  - damper pedal and sostenuto
    //  - portamento control
    //  - legato footswitch

    // The note manager handles legato and portamento.  Each is
    // conceptually simple but has some special cases.
    //
    // Legato is the concept that successive notes in a phrase do not
    // need to retrigger envelopes.  Instead, the voice playing the
    // old note changes its pitch to the new note.
    //
    //  - Each channel has a toggle, enable_legato.  When it is off,
    //    legato transitions are disabled on that channel.
    //
    //  - When a channel is in mono mode, and a note is sounding when
    //    a new note is played, a legato transition is made.
    //
    //    In MIDI terms, if a Note On message arrives before the
    //    previous note's Note Off, than it is legato.  Or, if a Note
    //    Off message message arrives and an older note is still on,
    //    then the voice will make a legato transition back to the
    //    older note.  The Damper Pedal and Sostenuto controllers also
    //    affect whether the older note is sounding.
    //
    //  - When a channel is in poly mode, if a Portamento Control
    //    message precedes a Note On, and the note in the PC message
    //    is currently sounding, then that note's voices make a legato
    //    transition to the new note.
    //
    //  - There is also a Legato Footswitch controller.  The Legato
    //    Footswitch puts a channel into mono mode and enables legato.
    //    Then legato processing is handled as described above.  The
    //    Legato Footswitch controller is actually handled in the
    //    mode manager; it is just mentioned here for completeness.
    //
    // Portamento, aka glide, is starting a note by sliding into its
    // pitch from a different note's pitch.
    //
    //  - Each channel has a Portamento Time controller.  When the
    //    portamento time is zero, portamento may be enabled, but
    //    it has no audible effect.
    //
    //  - When a starting note has been specified by a Portamento
    //    Control message, portamento is enabled.  If the starting
    //    note is currently sounding, that note's voices will be
    //    reused.  Otherwise,
    //
    //  - When a channel is in mono mode, and a legato transition
    //    occurs (see above), the transition uses portamento.
    //
    //  - When a channel is in mono mode, and enable_legato is off,
    //    every note starts with portamento from the last note.
    //
    // A note may start with legato, portamento, both, or neither.


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
        void attach_dispatcher(Dispatcher&);
        void attach_synth(::Synth&);
        void attach_assigner(::Assigner&);

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

        // Channel reset messages
        void all_sound_off();
        void reset_all_controllers();
        void all_notes_off();
        void all_notes_off(channel_index);

        // Render function
        void render(size_t);

    private:

        typedef std::uint8_t            note_number;
        typedef std::bitset<NOTE_COUNT> note_mask;
        typedef Layering::timbre_mask   timbre_mask;

        static const voice_index        NO_VOICE      =   ~0;
        static const channel_index      NO_CHANNEL    = 0xFF;
        static const note_number        NO_NOTE       = 0xFF;
        static const std::uint8_t       NO_VELOCITY   = 0xFF;
        static const std::uint8_t       NO_PORTAMENTO = 0xFF;

        // note_start_info: state needed to start a new note
        struct note_start_info {
            channel_index               channel;
            timbre_mask                 timbres;
            note_number                 src_note;
            bool                        is_legato;
            note_number                 portamento_note;
            note_number                 note;
            std::uint16_t               attack_velocity;
        };

        // per-channel data
        struct channel_data {
            Mode                        mode;
            std::uint8_t                velocity_lsb;
            std::uint8_t                portamento_note;
            bool                        is_sustaining;
            bool                        legato_enabled;
            std::uint8_t                mono_note;
            voice_index                 mono_voices[MAX_TIMBRES];
            std::uint16_t               mono_attack_velocity;
            note_mask                   notes_on;
            note_mask                   notes_sustaining;
            note_mask                   notes_sostenuto;

            channel_data()
            : mode{Mode::POLY},
              velocity_lsb{NO_VELOCITY},
              portamento_note{NO_NOTE},
              is_sustaining{false},
              legato_enabled{true},
              mono_note{NO_NOTE},
              mono_voices{0}
            {}

            // Should this note be sounding, either because it received
            // a Note On without Note Off, or it is being held by the
            // Damper Pedal or Sostenuto control?
            //
            // This does not consider whether the note maps to any
            // timbres or has been assigned any voices.
            bool note_should_sound(note_number ni)
            {
                return (notes_on[ni] ||
                        notes_sustaining[ni] ||
                        notes_sostenuto[ni]);
            }

            // note_mask sounding_notes()
            // {
            //     return notes_on | notes_sustaining | notes_sostenuto;
            // }

            // Should the newest note reuse any existing note's voices?
            note_number src_note()
            {
                switch (mode) {

                case Mode::MONO:
                    return mono_note;

                case Mode::POLY:
                    return portamento_note;
                }
            }

            // Should the newest note use portamento?  And if so, from
            // what note?
            note_number portamento_src()
            {
                if (portamento_note != NO_NOTE)
                    return portamento_note;
                if (mode == Mode::MONO)
                    return mono_note;
                return NO_NOTE;
            }

            // Should the newest note play legato?
            bool is_legato()
            {
                if (!legato_enabled)
                    return false;
                auto src = src_note();
                return src != NO_NOTE && note_should_sound(src);
            }

        };

        // per-voice data
        struct voice_data {
            std::uint8_t                channel;
            std::uint8_t                note;
            portamento_note_handler     portamento_note_handler;
            note_number_handler         note_number_handler;
            attack_velocity_handler     attack_velocity_handler;
            poly_pressure_handler       poly_pressure_handler;
            release_velocity_handler    release_velocity_handler;

            voice_data()
            : channel{NO_CHANNEL},
              note{NO_NOTE}
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

        // Voice control
        void enqueue_note(const note_start_info&);
        void start_note(Voice *, timbre_index, const note_start_info&);
        void change_note(voice_index, const note_start_info&);
        void release_note(channel_index,
                          note_number,
                          std::uint8_t velocity = 0);
        void kill_note(Voice *);

        voice_index find_existing_voice(channel_index,
                                        timbre_index,
                                        note_number);
        bool resume_earlier_note(channel_index, note_start_info&);

        // Member Variables
        ::Synth                                 *m_synth;
        ::Assigner                              *m_assigner;
        Dispatcher                              *m_dispatcher;
        const Layering                          *m_layering;
        std::array<channel_data, CHANNEL_COUNT>  m_channels;
        fixed_vector<voice_data, MAX_VOICES>     m_voices;
        fixed_queue<note_start_info, MAX_VOICES> m_pending_notes;
        fixed_queue<Voice *, MAX_VOICES>         m_killed_voices;

        static_assert(MAX_VOICES < std::numeric_limits<voice_index>::max(),
                      "voice_index too small");

        static_assert(MAX_TIMBRES < std::numeric_limits<timbre_index>::max(),
                      "timbre_index too small");

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


    // -- Attach Things -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline void
    NoteManager::
    attach_dispatcher(Dispatcher& d)
    {
        assert(!m_dispatcher);
        m_dispatcher = &d;
        m_layering = d.layering();
        assert(m_layering);
        if (m_synth)
            assert(m_layering->timbrality == m_synth->timbrality);
        auto all_timbres = m_layering->all_timbres;
        d.register_handler(StatusByte::NOTE_ON,
                           Layering::ALL_CHANNELS,
                           note_on_binding(this));
        d.register_handler(StatusByte::NOTE_OFF,
                           Layering::ALL_CHANNELS,
                           note_off_binding(this));
        d.register_handler(StatusByte::POLY_KEY_PRESSURE,
                           Layering::ALL_CHANNELS,
                           poly_pressure_binding(this));
        d.register_handler(ControllerNumber::DAMPER_PEDAL,
                           all_timbres,
                           damper_pedal_binding(this));
        d.register_handler(ControllerNumber::SOSTENUTO,
                           all_timbres,
                           sostenuto_binding(this));
        d.register_handler(ControllerNumber::HIGH_RESOLUTION_VELOCITY_PREFIX,
                           all_timbres,
                           high_res_velocity_binding(this));
        d.register_handler(ControllerNumber::PORTAMENTO_CONTROL,
                           all_timbres,
                           portamento_control_binding(this));
    }

    inline void
    NoteManager::
    attach_synth(Synth& s)
    {
        assert(!m_synth);
        if (m_layering)
            assert(m_layering->timbrality == s.timbrality);
        m_synth = &s;
        m_voices.resize(s.polyphony);
    }

    inline void
    NoteManager::
    attach_assigner(Assigner& a)
    {
        assert(!m_assigner);
        m_assigner = &a;
    }


    // -- Registrars -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline void
    NoteManager::
    register_portamento_note_handler(voice_index vi, portamento_note_handler h)
    {
        assert(!m_voices[vi].portamento_note_handler);
        m_voices[vi].portamento_note_handler = h;
    }

    inline void
    NoteManager::
    register_note_number_handler(voice_index vi, note_number_handler h)
    {
        assert(!m_voices[vi].note_number_handler);
        m_voices[vi].note_number_handler = h;
    }

    inline void
    NoteManager::
    register_attack_velocity_handler(voice_index vi, attack_velocity_handler h)
    {
        assert(!m_voices[vi].attack_velocity_handler);
        m_voices[vi].attack_velocity_handler = h;
    }

    inline void
    NoteManager::
    register_poly_pressure_handler(voice_index vi, poly_pressure_handler h)
    {
        assert(!m_voices[vi].poly_pressure_handler);
        m_voices[vi].poly_pressure_handler = h;
    }

    inline void
    NoteManager::
    register_release_velocity_handler(voice_index vi,
                                      release_velocity_handler h)
    {
        assert(!m_voices[vi].release_velocity_handler);
        m_voices[vi].release_velocity_handler = h;
    }


// -- Channel Mode Control - -- -- -- -- -- -- -- -- -- -- -- -- -- //

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


    // -- Channel Reset Message  -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline void
    NoteManager::
    all_sound_off()
    {
        for (auto& voice: m_synth->voices())
            if (voice.state() != Voice::State::IDLE &&
                voice.state() != Voice::State::STOPPING)
                //voice->kill_note();
                kill_note(&voice);
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
        // Should sustain and sostenuto be reset?  Yes.
    }

    inline void
    NoteManager::
    all_notes_off()
    {
        while (!m_pending_notes.empty())
            m_pending_notes.pop();
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

        // Clear this channel's notes from the pending note queue.
        // We do this by popping everything and re-enqueuing notes
        // for other channels.
        size_t qsize = m_pending_notes.size();
        for (size_t i = 0; i < qsize; i++) {
            note_start_info nsi = m_pending_notes.front();
            m_pending_notes.pop();
            if (nsi.channel != ci)
                m_pending_notes.push(nsi);
        }
    }


    // -- Render Function  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    inline void
    NoteManager::
    render(size_t)
    {
        // Find killed voices that have finally died and reuse them.
        while (!m_killed_voices.empty()) {
            Voice *voice = m_killed_voices.front();
            if (voice->state() != Voice::State::IDLE)
                break;
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


    // -- MIDI Message Handlers- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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
        auto p_note = chan.portamento_src();
        chan.portamento_note = NO_PORTAMENTO;
        if (chan.mode == Mode::MONO)
            chan.mono_attack_velocity = vel;

// std::cout << "\nNM::handle_note_on\n";
// std::cout << "  note = " << int(note) << '\n';
// std::cout << "  src_note => " << int(chan.src_note()) << '\n';
// std::cout << "  mono_note = " << int(chan.mono_note) << '\n';
// std::cout << "  is_legato() => "
//           << std::boolalpha << chan.is_legato() << '\n';

        note_start_info info;
        info.channel         = ci;
        info.timbres         = timbres;
        info.src_note        = chan.src_note();
        info.is_legato       = chan.is_legato();
        info.portamento_note = p_note;
        info.note            = note;
        info.attack_velocity = vel;

        enqueue_note(info);
    }

    inline void
    NoteManager::
    handle_note_off_message(const SmallMessage& msg)
    {
        // N.B., status may either be NOTE_OFF or NOTE_ON,
        // because NOTE_ON message w/ velocity 0 means note off.
        channel_index ci = msg.channel();
        note_number note = msg.note_number();
        std::uint8_t vel = msg.velocity();

        auto& chan = m_channels[ci];

        switch (chan.mode) {

        case Mode::POLY:
            chan.notes_on.reset(note);
            for (auto& v_data: m_voices)
                if (v_data.channel == ci && v_data.note == note)
                    release_note(ci, note, vel);
            break;

        case Mode::MONO:
// std::cout << "\nNM::handle_note_off\n";

            // Collect info for resuming earlier note.
            note_start_info resume_info = {};
            resume_info.src_note = chan.src_note();
            resume_info.is_legato = chan.is_legato();
            resume_info.portamento_note = chan.portamento_src();

            chan.notes_on.reset(note);

            if (!chan.note_should_sound(note) &&
                note == chan.mono_note) {
                if (!resume_earlier_note(ci, resume_info)) {
// std::cout << "  releasing note " << int(note) << '\n';
                // linear search, yuck!
                    for (auto& v_data: m_voices)
// {
//  std::cout << "  v: channel = "
//            << int(ci)
//            << ", note = "
//            << int(v_data.note)
//            << '\n';
                        if (v_data.channel == ci && v_data.note == note)
                            release_note(ci, note, vel);
// }
                }
            }

            break;
        }

        // // Collect info for resuming earlier note.
        // note_start_info resume_info = {};
        // resume_info.src_note = chan.src_note();
        // resume_info.is_legato = chan.is_legato();
        // resume_info.portamento_note = chan.portamento_src();
        //
        // chan.notes_on.reset(note);
        //
        // if (!chan.note_should_sound(note)) {
        //     if (chan.mote == MONO && )
        //     // linear search, yuck!
        //     for (auto& v_data: m_voices)
        //         if (v_data.channel == ci && v_data.note == note)
        //             release_note(ci, note, vel);
        //     resume_earlier_note(ci, resume_info);
        // }
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
            bool did_release = false;
            for (auto& v_data: m_voices) {
                if (v_data.channel == ci) {
                    auto note = v_data.note;
                    if (!chan.note_should_sound(note)) {
                        release_note(ci, note);
                        did_release = true;
                    }
                }
            }
            // if (did_release)
            //     resume_earlier_note(ci);
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
            bool did_release = false;
            for (auto& v_data: m_voices) {
                if (v_data.channel == ci) {
                    auto note = v_data.note;
                    if (!chan.note_should_sound(note)) {
                        release_note(ci, note);
                        did_release = true;
                    }
                }
            }
            // if (did_release)
            //     resume_earlier_note(ci);
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


    // -- Voice Control -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    // Called from Note On. Note Off, Damper Pedal release, and
    // Sostenuto release.
    inline void
    NoteManager::
    enqueue_note(const note_start_info& info)
    {
// std::cout << "\nenqueue_note\n";
// std::cout << "  info.note = " << int(info.note) << '\n';
        auto ci = info.channel;
        auto& chan = m_channels[ci];
        auto timbrality = m_layering->timbrality;
        // if (chan.mode == Mode::MONO)
        //     chan.mono_note = info.note;
        for (size_t ti = 0; ti < timbrality; ti++) {
            if (info.timbres & (1 << ti)) {
                auto existing_vi = find_existing_voice(ci, ti, info.src_note);
                if (existing_vi != NO_VOICE) {
                    change_note(existing_vi, info);
                } else if (auto *voice = m_assigner->assign_idle_voice()) {
                    start_note(voice, ti, info);
                } else if (auto *voice = m_assigner->choose_voice_to_steal()) {
                    kill_note(voice);
                    //voice->kill_note();
                    m_pending_notes.push(info);
                } else {
                    // replace oldest pending notes.
                    auto n = count_bits(info.timbres);
                    while (n > 0 && !m_pending_notes.empty()) {
                        n -= count_bits(m_pending_notes.front().timbres);
                        m_pending_notes.pop();
                    }
                    m_pending_notes.push(info);
                }
            }
        }
        if (chan.mode == Mode::MONO)
            chan.mono_note = info.note;
    }

    inline void
    NoteManager::
    start_note(Voice *voice, timbre_index ti, const note_start_info& info)
    {
// std::cout << "\nNM::start_note\n";
        voice_index vi = voice - m_synth->voices().data();
        assert(0 <= vi && vi < MAX_VOICES);
        auto& v_data = m_voices[vi];
        Timbre& timbre = m_synth->timbres()[ti];

        if (voice->timbre() != &timbre)
            m_synth->attach_voice_to_timbre(timbre, *voice);
        v_data.channel = info.channel;
        v_data.note = info.note;
        if (auto& h = v_data.note_number_handler)
            h(info.note);
        if (auto& h = v_data.attack_velocity_handler)
            h(info.attack_velocity);
        if (info.portamento_note != NO_NOTE)
            if (auto& h = v_data.portamento_note_handler)
                h(info.portamento_note);
        voice->start_note();
    }

    inline void
    NoteManager::
    change_note(voice_index vi, const note_start_info& info)
    {
// std::cout << "\nNM::change_note\n";
// std::cout << "  info.note = " << int(info.note) << '\n';
        assert(vi < m_synth->polyphony);
        auto& v_data = m_voices[vi];
        assert(v_data.channel == info.channel);
        v_data.note = info.note;

        if (auto& h = v_data.note_number_handler)
            h(info.note);
        if (!info.is_legato)
            if (auto& h = v_data.attack_velocity_handler)
                h(info.attack_velocity);
        if (info.portamento_note != NO_NOTE)
            if (auto& h = v_data.portamento_note_handler)
                h(info.portamento_note);
    }

    inline void
    NoteManager::
    release_note(channel_index ci, note_number note, std::uint8_t velocity)
    {
        for (size_t vi = 0; vi < m_synth->polyphony; vi++) {
            auto& v_data = m_voices[vi];
            if (v_data.channel == ci && v_data.note == note) {
                auto *voice = &m_synth->voices()[vi];
                if (voice->state() == Voice::State::SOUNDING) {
                    if (auto& h = v_data.release_velocity_handler)
                        h(velocity);
                    voice->release_note();
                }
            }
        }
    }

    inline void
    NoteManager::
    kill_note(Voice *voice)
    {
        auto vi = voice - m_synth->voices().data();
        auto& v_data = m_voices[vi];
        voice->kill_note();
        m_killed_voices.push(voice);
        v_data.channel = NO_CHANNEL;
        v_data.note = NO_NOTE;
    }

    inline auto
    NoteManager::
    find_existing_voice(channel_index ci,
                        timbre_index ti,
                        note_number note)
    -> voice_index
    {
        auto& chan = m_channels[ci];
        switch (chan.mode) {

        case Mode::MONO:
// std::cout << "\nNM::find_existing_voice\n";
// std::cout << "  ci = "
//           << int(ci)
//           << '\n';
// std::cout << "  note = "
//           << int(note)
//           << '\n';
// std::cout << "  mono_note = " << int(chan.mono_note) << '\n';
// std::cout << std::endl;
            if (chan.mono_note != NO_NOTE) {
                assert(note == NO_NOTE || note == chan.mono_note);
                return chan.mono_voices[ti];
            }
            break;

        case Mode::POLY:
            for (size_t vi = 0; vi < m_synth->polyphony; vi++) {
                auto& v_data = m_voices[vi];
                if (v_data.channel == ci && v_data.note == note) {
                    auto& voice = m_synth->voices()[vi];
                    auto ti2 = voice.timbre() - m_synth->timbres().data();
                    if (ti2 == ti)
                        return vi;
                }
                // XXX ensure there is only one voice that matches.
            }
            break;
        }
        return NO_VOICE;
    }

    inline auto
    NoteManager::
    resume_earlier_note(channel_index ci, note_start_info& resume_info)
    -> bool
    {
        auto& chan = m_channels[ci];
        assert(chan.mode == Mode::MONO);

        if (chan.notes_on.none() &&
            chan.notes_sustaining.none() &&
            chan.notes_sostenuto.none())
        {
            return false;
        }

        for (size_t note = NOTE_COUNT; note-- > 0; ) {
            if (chan.note_should_sound(note)) {
                resume_info.channel = ci;
                resume_info.timbres = m_layering->channel_timbres(ci);
                // src_note, is_legato, and portamento_note
                /// are already initialized.
                resume_info.note = note;
                resume_info.attack_velocity = chan.mono_attack_velocity;
                enqueue_note(resume_info);
                return true;
            }
        }
        return false;
    }

    // XXX how do we keep the assigner from stealing the mono voices?
    //     need some concept of protected voice?
    //     Answer: we don't  steal the mono voice.  If mono timbre A
    //     is not playing, let poly timbre B use the voice.

}

#endif /* !MIDI_NOTE_MGR_included */
