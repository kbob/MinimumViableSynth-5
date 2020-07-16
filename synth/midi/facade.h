#ifndef MIDI_FACADE_included
#define MIDI_FACADE_included

#include "synth/midi/config.h"
#include "synth/midi/dispatcher.h"
#include "synth/midi/layering.h"
#include "synth/midi/mode-mgr.h"
#include "synth/midi/note-mgr.h"
#include "synth/midi/parser.h"
#include "synth/midi/timbre-mgr.h"
#include "synth/midi/sizes.h"

class Assigner;
class Synth;

namespace midi {

    // Define convenience types here so clients can access them
    // with just `midi::` qualification.

    using small_handler = Dispatcher::small_handler;
    using channel_index = Layering::channel_index;
    using channel_mask = Layering::channel_mask;
    using interface_index = size_t;

    class Facade {

    public:

        // Construction
        Facade(size_t polyphony, size_t timbrality);
        Facade& attach(::Synth&);
        Facade& attach(::Assigner&);
        void finalize();

        // Configuration -- register as a subsystem of ::Config.
        Config& configurator();

        // Parameters
        const size_t polyphony;
        const size_t timbrality;

        // Interface Configuration
        size_t interface_count() const;
        bool interface_is_input(interface_index) const;
        bool interface_is_output(interface_index) const;
        void interface_is_input(interface_index, bool);
        void interface_is_output(interface_index, bool);

        // Mode Queries
        bool is_omni() const;
        bool is_mono() const;
        bool channel_is_mono(channel_index) const;
        bool is_multi() const;
        channel_mask mode4_active_channels() const;
        bool channel_is_legato(channel_index) const;

        // Mode Control
        void multi_mode(bool);
        void channel_legato_mode(channel_index, bool);

        // Input
        void process_byte(interface_index, char);
        void process_bytes(interface_index, const char *, size_t);
        void process_message(interface_index, const char *, size_t);

        // Output - TBD

    private:

        struct interface_data {
            bool is_input;
            bool is_output;
            Parser parser;

            interface_data()
            : is_input{false},
              is_output{false}
            {}
        };

        bool          m_finalized;
        ::Synth      *m_synth;
        ::Assigner   *m_assigner;
        Layering      m_layering;
        Dispatcher    m_dispatcher;
        NoteManager   m_note_mgr;
        TimbreManager m_timbre_mgr;
        ModeManager   m_mode_mgr;
        Config        m_config;
        std::array<interface_data, MAX_INTERFACES> m_interfaces;

    };


    // -- Construction - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline
    Facade::
    Facade(size_t poly, size_t timb)
    : polyphony{poly},
      timbrality{timb},
      m_finalized{false},
      m_synth{nullptr},
      m_assigner{nullptr},
      m_layering{timb},
      m_config{m_dispatcher, m_note_mgr, m_timbre_mgr}
    {
        m_dispatcher.attach_layering(m_layering);
        m_note_mgr.attach_dispatcher(m_dispatcher);
        m_timbre_mgr.attach_dispatcher(m_dispatcher);
    }

    inline Facade&
    Facade::attach(::Synth& s)
    {
        assert(!m_finalized);
        m_synth = &s;
        m_note_mgr.attach_synth(s);
        return *this;
    }

    inline Facade&
    Facade::attach(::Assigner& a)
    {
        assert(!m_finalized);
        m_assigner = &a;
        m_note_mgr.attach_assigner(a);
        return *this;
    }

    inline void
    Facade::finalize()
    {
        assert(m_synth);
        assert(m_assigner);
        m_finalized = true;
    }


    // -- Configurator - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline auto
    Facade::configurator()
    -> Config&
    {
        assert(m_finalized);
        return m_config;
    }


    // -- Interface Configuration -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline auto
    Facade::interface_count() const
    -> size_t
    {
        assert(m_finalized);
        return m_interfaces.size();
    }

    inline auto
    Facade::interface_is_input(interface_index index) const
    -> bool
    {
        assert(m_finalized);
        return index < m_interfaces.size() && m_interfaces[index].is_input;
    }

    inline auto
    Facade::interface_is_output(interface_index index) const
    -> bool
    {
        assert(m_finalized);
        return index < m_interfaces.size() && m_interfaces[index].is_output;
    }

    inline void
    Facade::
    interface_is_input(interface_index index, bool enable)
    {
        assert(m_finalized);
        assert(index < m_interfaces.size());
        auto& iface = m_interfaces[index];
        if (enable && !iface.is_input)
            iface.parser.reset();
        iface.is_input = enable;
    }

    inline void
    Facade::
    interface_is_output(interface_index index, bool enable)
    {
        assert(!"output not implemented");
        assert(m_finalized);
        assert(index < m_interfaces.size());
        auto& iface = m_interfaces[index];
        iface.is_output = enable;
    }


    // -- Mode Queries - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline auto
    Facade::
    is_omni() const
    -> bool
    {
        assert(m_finalized);
        return m_mode_mgr.is_omni();
    }

    inline auto
    Facade::
    is_mono() const
    -> bool
    {
        assert(m_finalized);
        return m_mode_mgr.is_mono();
    }

    inline auto
    Facade::
    channel_is_mono(channel_index ci) const
    -> bool
    {
        assert(m_finalized);
        return m_mode_mgr.is_mono(ci);
    }

    inline auto
    Facade::
    is_multi() const
    -> bool
    {
        assert(m_finalized);
        return m_mode_mgr.is_multi();
    }

    inline auto
    Facade::
    mode4_active_channels() const
    -> channel_mask
    {
        assert(m_finalized);
        return m_mode_mgr.mode4_active_channels();
    }

    inline auto
    Facade::
    channel_is_legato(channel_index ci) const
    -> bool
    {
        assert(m_finalized);
        return m_note_mgr.channel_legato(ci);
    }


    // -- Mode Control - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline void
    Facade::
    multi_mode(bool enable)
    {
        assert(m_finalized);
        m_mode_mgr.multi_mode(enable);
    }

    inline void
    Facade::
    channel_legato_mode(channel_index ci, bool enable)
    {
        assert(m_finalized);
        m_note_mgr.channel_legato(ci, enable);
    }


    // -- Input -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- - //

    inline void
    Facade::
    process_byte(interface_index ii, char byte)
    {
        assert(m_finalized);
        assert(ii < MAX_INTERFACES);
        auto& iface = m_interfaces[ii];
        assert(iface.is_input);
        iface.parser.process_byte(byte);
    }

    inline void
    Facade::
    process_bytes(interface_index ii, const char *data, size_t size)
    {
        assert(m_finalized);
        assert(ii < MAX_INTERFACES);
        auto& iface = m_interfaces[ii];
        assert(iface.is_input);
        iface.parser.process_bytes(data, size);
    }

    inline void
    Facade::
    process_message(interface_index ii, const char *data, size_t size)
    {
        assert(m_finalized);
        assert(ii < MAX_INTERFACES);
        auto& iface = m_interfaces[ii];
        assert(iface.is_input);
        iface.parser.process_message(data, size);
    }

}

#endif /* !MIDI_FACADE_included */
