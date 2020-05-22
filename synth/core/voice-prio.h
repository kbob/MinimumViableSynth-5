#ifndef VOICE_PRIO_included
#define VOICE_PRIO_included

#include <functional>
#include <limits>

#include "synth/core/config.h"
#include "synth/core/synth.h"
#include "synth/core/voice.h"
#include "synth/core/voice-alloc.h"
#include "synth/util/fixed-queue.h"
#include "synth/util/universe.h"

class PriorityAllocator : public VoiceAllocator {

public:

    typedef std::function<int(const Voice&)> prioritizer;

    PriorityAllocator(Synth& synth, prioritizer& f)
    : m_synth{synth},
      m_prio{f},
      m_all_voices{synth.voices()},
      m_free_voices{m_all_voices.none}
    {}

    Voice *allocate_voice() override
    {
        if (!m_pending.empty()) {
            if (Voice *avail = allocate_stolen_voice())
                return avail;
        } else {
            if (Voice *avail = allocate_idle_voice())
                return avail;
        }

        // neither idle nore pending voice available -- steal a voice.
        steal_voice();
        return nullptr;
    }

private:

    typedef Universe<Synth::voice_vector, MAX_POLYPHONY> voice_verse;
    typedef voice_verse::subset_type voice_set;
    typedef fixed_queue<Voice *, MAX_POLYPHONY> voice_queue;

    Synth& m_synth;
    prioritizer& m_prio;

    voice_verse m_all_voices;
    voice_set m_free_voices;

    // XXX `Subset<C, N>::index_iterator` should be public.
    //     (It should also be `index_iterator`, not `index_iter`.)
    typedef decltype(m_free_voices.indices().begin()) vi_iterator;
    vi_iterator m_free_voice_rotor;

    voice_queue m_pending;

    Voice *allocate_stolen_voice()
    {
        if (m_pending.empty())
            return nullptr;
        Voice *avail = m_pending.front();
        if (avail->state() != Voice::State::IDLE)
            return nullptr;
        m_pending.pop();
        return avail;
    }

    Voice *allocate_idle_voice()
    {
        if (!m_free_voices.any()) {
            for (size_t i = 0; i < m_synth.polyphony; i++)
                if (m_synth.voices()[i].state() == Voice::State::IDLE)
                    m_free_voices.set(i);
            m_free_voice_rotor = m_free_voices.indices().begin();
            // XXX Should have a rotor to speed up finding next idle
            //     voice.  But Subset<T>::index_iter is private.
        }
        if (m_free_voices.any()) {
            size_t i = *m_free_voice_rotor++;
            m_free_voices.reset(i);
            return &m_synth.voices()[i];
        }
        return nullptr;
    }

    void steal_voice()
    {
        int min_pri = std::numeric_limits<int>::max();
        Voice *stolen = nullptr;
        for (auto& v: m_synth.voices()) {
            if (v.state() != Voice::State::STOPPING) {
                int pri = m_prio(v);
                if (min_pri > pri) {
                    min_pri = pri;
                    stolen = &v;
                }
            }
        }
        if (stolen) {
            stolen->kill_note();
            m_pending.push(stolen);
        }
    }

};

#endif /* !VOICE_PRIO_included */
