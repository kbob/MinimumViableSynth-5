#ifndef MIDI_LAYERING_included
#define MIDI_LAYERING_included

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

#include "synth/midi/defs.h"
#include "synth/core/sizes.h"

namespace midi {

    class Layering {

    public:

        // // Maybe I should make some self-sizing integer types?
        //
        // typedef size_type<CHANNEL_COUNT> channel_index;
        // typedef size_type<MAX_TIMBRES> timbre_index;
        // typedef mask_type<CHANNEL_COUNT> channel_mask;
        // typedef mask_type<MAX_TIMBRES> timbre_mask;
        typedef std::uint8_t channel_index;
        typedef std::uint8_t timbre_index;
        typedef std::uint16_t channel_mask;
        typedef std::uint8_t timbre_mask;

        static const channel_index NO_CHANNEL   = ~0;
        static const timbre_index  NO_TIMBRE    = ~0;
        static const channel_mask  ALL_CHANNELS = (1 << CHANNEL_COUNT) - 1;

        const timbre_index timbrality;
        const timbre_mask all_timbres;

        Layering(size_t timbrality);

        void omni_mode();
        void poly_mode(channel_index);
        void mono_mode(channel_mask);
        void multi_mode();

        // Getters
        timbre_mask channel_timbres(channel_index) const;
        channel_mask timbre_channels(timbre_index) const;

        // Setters
        void channel_timbres(channel_index, timbre_mask);
        void timbre_channels(timbre_index, channel_mask);

    private:

        std::array<timbre_mask, CHANNEL_COUNT> m_channel_timbres;
        std::array<channel_mask, MAX_TIMBRES>  m_timbre_channels;

        static_assert(
            std::numeric_limits<channel_index>::max() > CHANNEL_COUNT,
            "channel_index too small"
        );
        static_assert(
            std::numeric_limits<timbre_index>::max() > MAX_TIMBRES,
            "timbre_index too small");
        static_assert(
            std::numeric_limits<channel_mask>::digits >= CHANNEL_COUNT,
            "channel_mask too small"
        );
        static_assert(
            std::numeric_limits<timbre_mask>::digits >= MAX_TIMBRES,
            "timbre_mask too small"
        );

    };

    inline
    Layering::
    Layering(size_t timb)
    : timbrality{static_cast<timbre_index>(timb)},
      all_timbres{static_cast<timbre_mask>((1 << timb) - 1)}
    {
        assert(timbrality <= MAX_TIMBRES);
        omni_mode();
    }

    inline void
    Layering::
    omni_mode()
    {
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            m_channel_timbres[ci] = 1 << 0;
        // init all timbres here because the constructor calls us.
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            m_timbre_channels[ti] = (ti == 0) ? ALL_CHANNELS : 0;
    }

    inline void
    Layering::
    poly_mode(channel_index chan)
    {
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            m_channel_timbres[ci] = (ci == chan) ? (1 << 0) : 0;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            m_timbre_channels[ti] = (ti == 0) ? 1 << chan : 0;
    }

    inline void
    Layering::
    mono_mode(channel_mask enabled_channels)
    {
        assert(enabled_channels);
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            m_channel_timbres[ci] = (enabled_channels & 1 << ci) ? 1 : 0;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            m_timbre_channels[ti] = ti ? 0 : enabled_channels;
    }

    inline void
    Layering::
    multi_mode()
    {
        m_channel_timbres = {0};
        m_timbre_channels = {0};
        for (size_t ci = 0; ci < CHANNEL_COUNT && ci < timbrality; ci++)
            m_channel_timbres[ci] = 1 << ci;
        for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
            m_timbre_channels[ti] = (ti < timbrality) ? 1 << ti : 0;
    }

    inline auto
    Layering::
    channel_timbres(channel_index ci) const
    -> timbre_mask
    {
        assert(!(m_channel_timbres[ci] & ~all_timbres));
        return m_channel_timbres[ci];
    }

    inline auto
    Layering::
    timbre_channels(timbre_index ti) const
    -> channel_mask
    {
        return m_timbre_channels[ti];
    }

    inline void
    Layering::
    channel_timbres(channel_index ci, timbre_mask tm)
    {
        assert(!(tm & ~all_timbres));
        m_channel_timbres[ci] = tm;
        for (size_t ti = 0; ti < timbrality; ti++) {
            if (tm & (1 << ti))
                m_timbre_channels[ti] |= 1 << ci;
            else
                m_timbre_channels[ti] &= ~(1 << ci);
        }
    }

    inline void
    Layering::
    timbre_channels(timbre_index ti, channel_mask cm)
    {
        assert(ti < timbrality || cm == 0x0000);
        m_timbre_channels[ti] = cm;
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++) {
            if (cm & (1 << ci))
                m_channel_timbres[ci] |= 1 << ti;
            else
                m_channel_timbres[ci] &= ~(1 << ti);
        }
    }

}

#endif /* !MIDI_LAYERING_included */
