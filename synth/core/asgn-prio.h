#ifndef ASGN_PRIO_included
#define ASGN_PRIO_included

#include <functional>
#include <limits>

#include "synth/core/assigners.h"
#include "synth/core/sizes.h"
#include "synth/core/synth.h"
#include "synth/core/voice.h"
#include "synth/util/fixed-queue.h"
#include "synth/util/universe.h"

class PriorityAssigner : public Assigner {

public:

    typedef std::function<int(const Voice&)> prioritizer;

    PriorityAssigner(Synth& synth, const prioritizer& f)
    : m_synth{synth},
      m_prio{f},
      m_all_voices{synth.voices()},
      m_free_voices{m_all_voices.none}
    {}

    Voice *assign_idle_voice() override
    {
        if (!m_free_voices.any()) {
            for (size_t i = 0; i < m_synth.polyphony; i++)
                if (m_synth.voices()[i].state() == Voice::State::IDLE)
                    m_free_voices.set(i);
            m_free_voice_rotor = m_free_voices.indices().begin();
        }
        if (m_free_voices.any()) {
            size_t i = *m_free_voice_rotor++;
            m_free_voices.reset(i);
            return &m_synth.voices()[i];
        }
        return nullptr;
    }

    Voice *choose_voice_to_steal() override
    {
        int min_pri = std::numeric_limits<int>::max();
        Voice *target = nullptr;
        for (auto& v: m_synth.voices()) {
            if (v.state() != Voice::State::STOPPING) {
                int pri = m_prio(v);
                if (min_pri > pri) {
                    min_pri = pri;
                    target = &v;
                }
            }
        }
        return target;
    }

private:

    typedef Universe<Synth::voice_vector, MAX_POLYPHONY> voice_verse;
    typedef voice_verse::subset_type voice_set;

    Synth& m_synth;
    prioritizer m_prio;

    voice_verse m_all_voices;
    voice_set m_free_voices;

    voice_set::index_iterator m_free_voice_rotor;

};

#endif /* !ASGN_PRIO_included */
