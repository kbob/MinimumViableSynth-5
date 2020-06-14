#ifndef MIDI_ROUTER_included
#define MIDI_ROUTER_included

#include <limits>

#include "synth/core/assigners.h"
#include "synth/core/config.h"
#include "synth/core/controls.h"
#include "synth/core/synth.h"
#include "synth/core/voice.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/messages.h"
#include "synth/util/fixed-queue.h"

namespace midi {

    // XXX channel to timbre is a many to many mapping.
    //     Layered voices map one channel to several timbres.
    //     Omni mode maps all channels to several timbres.
    //     The global control maps to all timbres.
    // So I dunno.
    // Can Dispatcher be extended to handle voices?



    class Router : public ::Config::Subsystem {

    public:

        typedef std::function<void(std::uint8_t)>  portamento_note_handler;
        typedef std::function<void(std::uint8_t)>  note_number_handler;
        typedef std::function<void(std::uint16_t)> attack_velocity_handler;
        typedef std::function<void(std::uint8_t)>  key_pressure_handler;
        typedef std::function<void(std::uint8_t)>  release_velocity_handler;

        Router();

        void attach_source(Dispatcher&);

        void register_portamento_note_handler(portamento_note_handler);
        void register_note_number_handler(note_number_handler);
        void register_attack_velocity_handler(attack_velocity_handler);
        void register_key_pressure_handler(key_pressure_handler);
        void register_release_velocity_handler(release_velocity_handler);

        void render();

        // ::Config::Subsystem interface
        void pre_configure(Timbre&) const override;
        void pre_configure(Voice&) const override;
        void post_configure(Synth&) const override;

    private:

        typedef Dispatcher::timbre_index timbre_index;
        typedef std::uint8_t             voice_index;
        typedef std::uint16_t            timbre_note_index;

        static const std::uint8_t NO_VELOCITY = 0xFF;
        static const std::uint8_t NO_NOTE     = 0xFF;
        static const timbre_index NO_TIMBRE   = Dispatcher::NO_TIMBRE;
        static const voice_index  NO_VOICE    =
            std::numeric_limits<voice_index>::max();
        static const timbre_note_index NO_TNI =
            std::numeric_limits<timbre_note_index>::max();

        struct note_info {
            timbre_index  timbre;
            std::uint8_t  portamento_note;
            std::uint8_t  number;
            std::uint16_t attack_velocity;
        };

        static const size_t TIMBRE_NOTE_COUNT = MAX_TIMBRES * NOTE_COUNT;
        static_assert(MAX_TIMBRES <= std::numeric_limits<timbre_index>::max(),
                      "timbre_index too small");
        static_assert(MAX_VOICES <= std::numeric_limits<voice_index>::max(),
                      "voice_index too small");
        static_assert(TIMBRE_NOTE_COUNT <=
                      std::numeric_limits<timbre_note_index>::max(),
                      "timbre_note_index too small");

        typedef fixed_vector<Timbre *, MAX_TIMBRES>   timbre_vector;
        typedef fixed_vector<Voice *, MAX_VOICES>    voice_vector;
        typedef Universe<timbre_vector, MAX_TIMBRES> timbre_verse;
        typedef Universe<voice_vector, MAX_VOICES>   voice_verse;

        template <class Handler>
            using voice_handlers = fixed_map<voice_index, Handler, MAX_VOICES>;

        ::Synth *m_synth;
        ::Assigner *m_assigner;
        Dispatcher *m_dispatcher;
        std::uint8_t m_velocity_lsb;
        std::uint8_t m_portamento_note;
        std::uint8_t m_is_sustaining;
        std::array<voice_index, TIMBRE_NOTE_COUNT> m_timbre_note_to_voice;
        std::array<timbre_note_index, MAX_VOICES> m_voice_to_tni;

        std::bitset<TIMBRE_NOTE_COUNT> m_notes_on;
        std::bitset<TIMBRE_NOTE_COUNT> m_notes_sustaining;
        std::bitset<TIMBRE_NOTE_COUNT> m_notes_sostenuto;
        std::bitset<TIMBRE_NOTE_COUNT> m_notes_voiced;  // Do we need this?
        mutable timbre_vector m_timbre_vec;
        mutable voice_vector m_voice_vec;
        mutable deferred<timbre_verse> m_timbres;
        mutable deferred<voice_verse> m_voices;
        fixed_queue<note_info, MAX_VOICES>       m_pending_notes;
        fixed_queue<voice_index, MAX_VOICES>     m_pending_voices;
        voice_handlers<portamento_note_handler>  m_portamento_note_handlers;
        voice_handlers<note_number_handler>      m_note_number_handlers;
        voice_handlers<attack_velocity_handler>  m_attack_velocity_handlers;
        voice_handlers<key_pressure_handler>     m_key_pressure_handlers;
        voice_handlers<release_velocity_handler> m_release_velocity_handlers;

        void handle_note_on_message(const SmallMessage&);
        void handle_note_off_message(const SmallMessage&);
        void handle_poly_key_pressure_message(const SmallMessage&);
        void handle_damper_pedal_message(const SmallMessage&);
        void handle_sostenuto_message(const SmallMessage&);
        void handle_high_res_velocity_message(const SmallMessage&);
        void handle_portamento_control_message(const SmallMessage&);
        void handle_all_notes_off_message(const SmallMessage&);

        bool should_sound(timbre_note_index);
        void start_note(Voice *, const note_info&);
        void release_note(timbre_note_index);

    };


    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
    class NoteControl : public ::ControlType<NoteControl> {
    public:
        void configure(const ::Config& cfg)
        {
            auto *router = cfg.get<Router>();
            router->register_note_number_handler([this] (std::uint8_t n) {
                m_note = n;
            });
        }
    private:
        std::uint8_t m_note;
    };
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


    inline
    Router::
    Router()
    : m_synth{nullptr},
      m_assigner{nullptr},
      m_dispatcher{nullptr},
      m_velocity_lsb{NO_VELOCITY},
      m_portamento_note{NO_NOTE},
      m_is_sustaining{false}
    {}

    inline void
    Router::
    attach_source(Dispatcher& disp)
    {
        m_dispatcher = &disp;
        disp.register_handler(StatusByte::NOTE_ON,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_note_on_message(msg);
                              });
        disp.register_handler(StatusByte::NOTE_OFF,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_note_off_message(msg);
                              });
        disp.register_handler(StatusByte::POLY_KEY_PRESSURE,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_poly_key_pressure_message(msg);
                              });
        disp.register_handler(ControllerNumber::DAMPER_PEDAL,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_damper_pedal_message(msg);
                              });
        disp.register_handler(ControllerNumber::SOSTENUTO,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_sostenuto_message(msg);
                              });
        disp.register_handler(ControllerNumber::HIGH_RESOLUTION_VELOCITY_PREFIX,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_high_res_velocity_message(msg);
                              });
        disp.register_handler(ControllerNumber::PORTAMENTO_CONTROL,
                              Dispatcher::ALL_CHANNELS,
                              [this] (const SmallMessage& msg) {
                                  handle_portamento_control_message(msg);
                              });
    }

    inline void
    Router::
    register_portamento_note_handler(portamento_note_handler h)
    {
        assert(m_voice_vec.size() > 0);
            m_portamento_note_handlers[m_voice_vec.size() - 1] = h;
    }

    inline void
    Router::
    register_note_number_handler(note_number_handler h)
    {
        assert(m_voice_vec.size() > 0);
        m_note_number_handlers[m_voice_vec.size() - 1] = h;
    }

    inline void
    Router::
    register_attack_velocity_handler(attack_velocity_handler h)
    {
        assert(m_voice_vec.size() > 0);
        m_attack_velocity_handlers[m_voice_vec.size() - 1] = h;
    }

    inline void
    Router::
    register_key_pressure_handler(key_pressure_handler h)
    {
        assert(m_voice_vec.size() > 0);
        m_key_pressure_handlers[m_voice_vec.size() - 1] = h;
    }

    inline void
    Router::
    register_release_velocity_handler(release_velocity_handler h)
    {
        assert(m_voice_vec.size() > 0);
        m_release_velocity_handlers[m_voice_vec.size() - 1] = h;
    }

    inline void
    Router::
    render()
    {
        if (m_pending_voices.empty())
            return;
        auto vi = m_pending_voices.front();
        Voice *voice = m_voices->operator [] (vi);
        if (voice->state() != Voice::State::IDLE)
            return;
        auto tni = m_voice_to_tni[vi];
        m_timbre_note_to_voice[tni] = NO_VOICE;
        m_voice_to_tni[vi] = NO_TNI;
        while (m_pending_notes.size() > m_pending_voices.size())
            m_pending_notes.pop();
        start_note(m_voices->operator [] (vi), m_pending_notes.front());
        m_pending_voices.pop();
        m_pending_notes.pop();
    }

    inline void
    Router::
    pre_configure(Timbre& v) const
    {
        m_timbre_vec.push_back(&v);
    }

    inline void
    Router::
    pre_configure(Voice& v) const
    {
        m_voice_vec.push_back(&v);
    }

    inline void
    Router::
    post_configure(Synth&) const
    {
        m_timbres.construct(m_timbre_vec);
        m_voices.construct(m_voice_vec);
    }

    inline void
    Router::
    handle_note_on_message(const SmallMessage& msg)
    {
        if (msg.velocity() == 0) {
            // This is a note off event.
            SmallMessage note_off(msg);
            auto s = msg.status_byte;
            s ^= static_cast<std::uint8_t>(StatusByte::NOTE_ON);
            s ^= static_cast<std::uint8_t>(StatusByte::NOTE_OFF);
            note_off.status_byte = s;
            assert(note_off.status() == StatusByte::NOTE_OFF);
            assert(note_off.channel() == msg.channel());
            handle_note_off_message(note_off);
            return;
        }

        auto ti = m_dispatcher->channel_to_timbre(msg.channel());
        auto ni = msg.note_number();
        auto tni = ti * NOTE_COUNT + ni;
        m_notes_on.set(tni);
        if (m_is_sustaining)
            m_notes_sustaining.set(tni);

        note_info info;
        info.timbre = ti;
        info.number = msg.note_number();
        std::uint16_t v = msg.velocity();
        if (m_velocity_lsb != NO_VELOCITY) {
            info.attack_velocity = v << 7 | m_velocity_lsb;
            m_velocity_lsb = NO_VELOCITY;
        } else
            info.attack_velocity = v << 7 | v;
        info.portamento_note = m_portamento_note;
        m_portamento_note = NO_NOTE;

        Voice *voice = m_assigner->assign_idle_voice();
        if (voice) {
            start_note(voice, info);
        } else {
            voice = m_assigner->choose_voice_to_steal();
            if (voice) {
                voice->kill_note();
                m_pending_voices.push(m_voices->index(voice));
            }
            if (m_pending_notes.full())
                m_pending_notes.pop();  // discard oldest
            m_pending_notes.push(info);
        }
    }

    inline bool
    Router::
    should_sound(timbre_note_index tni)
    {
        return m_notes_on[tni] ||
               m_notes_sustaining[tni] ||
               m_notes_sostenuto[tni];
    }

    inline void
    Router::
    handle_note_off_message(const SmallMessage& msg)
    {
        timbre_index ti = m_dispatcher->channel_to_timbre(msg.channel());
        if (ti == Dispatcher::NO_TIMBRE)
            return;

        auto ni = msg.note_number();
        size_t tni = ti * NOTE_COUNT + ni; // tni: timbre+note index
        m_notes_on.reset(tni);

        auto vi = m_timbre_note_to_voice[tni];
        if (vi == NO_VOICE)
            return;

        if (auto& h = m_release_velocity_handlers[vi])
            h(msg.velocity());

        if (!should_sound(tni))
            release_note(tni);
    }

    inline void
    Router::
    handle_all_notes_off_message(const SmallMessage& msg)
    {
        timbre_index ti = m_dispatcher->channel_to_timbre(msg.channel());
        if (ti == Dispatcher::NO_TIMBRE)
            return;

        auto notes_on = m_notes_on;
        m_notes_on.reset();
        for (size_t ni = 0; ni < NOTE_COUNT; ni++) {
            auto tni = ti * NOTE_COUNT + ni;
            if (notes_on[tni] && !should_sound(tni)) {
                auto vi = m_timbre_note_to_voice[tni];
                if (vi == NO_VOICE)
                    continue;

                if (auto& h = m_release_velocity_handlers[vi])
                    h(msg.velocity());
                release_note(tni);
            }
        }
    }

    inline void
    Router::
    start_note(Voice *voice, const note_info& note)
    {
        Timbre *timbre = m_timbres->operator [] (note.timbre);
        m_synth->attach_voice_to_timbre(*timbre, *voice);
        auto vi = m_voices->index(voice);
        if (auto& h = m_note_number_handlers[vi])
            h(note.number);
        if (auto& h = m_attack_velocity_handlers[vi])
            h(note.attack_velocity);
        if (note.portamento_note != NO_NOTE)
            if (auto& h = m_portamento_note_handlers[vi])
                h(note.portamento_note);
    }

    inline void
    Router::
    release_note(timbre_note_index tni)
    {
        auto vi = m_timbre_note_to_voice[tni];
        if (vi == NO_VOICE)
            return;
        auto voice = m_voices->operator [] (vi);
        voice->release_note();
    }

}

#endif /* MIDI_ROUTER_included */
