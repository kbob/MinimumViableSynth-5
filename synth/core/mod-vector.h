#ifndef MOD_VECTOR_included
#define MOD_VECTOR_included

#include "synth/core/modules.h"
#include "synth/util/bijection.h"
#include "synth/util/noalloc.h"

class mod_vector {

public:

    static const size_t MAX_MODULES = 16;
    static const size_t MAX_PORTS = 16;

private:

    typedef fixed_vector<ModuleBase *, MAX_MODULES> vec;

public:

    typedef typename vec::const_iterator const_iterator;
    typedef bijection<Port *, MAX_PORTS> port_vector;

    size_t            size()       const { return m_modules.size();  }
    const_iterator    begin()      const { return m_modules.begin(); }
    const_iterator    end()        const { return m_modules.end();   }
    const ModuleBase *at(size_t i) const { return m_modules.at(i);   }

    void push_back(ModuleBase *m)        { m_modules.push_back(m);   }

    size_t index(const ModuleBase *m) const
    {
        auto it = std::find(m_modules.begin(), m_modules.end(), m);
        if (it == m_modules.end())
            throw std::domain_error("mod_vector");
        return it - m_modules.begin();
    }

    const port_vector ports() const
    {
        port_vector ports;
        for (const auto *m: m_modules)
            for (auto *p: m->ports())
                ports.push_back(p);
        return ports;
    }

private:

    vec m_modules;

    friend class ModVectorUnitTest;
};

#endif /* !MOD_VECTOR_included */
