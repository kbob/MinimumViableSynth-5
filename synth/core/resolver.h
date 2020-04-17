#ifndef RESOLVER_included
#define RESOLVER_included

class Resolver {

public:

    static const size_t MAX_MODULES = 16;
    static const size_t MAX_PORTS = 16;

    void add_module(Module *m)
    {
        m_mod_map.push_back(m);
        for (auto *p: m->ports())
            m_port_map.push_back(p);
    }

    Module *resolve_module(size_t index)
    {
        if (index < m_mod_map.size())
            return m_mod_map.at(index);
        return NULL;
    }

    Port *resolve_port(size_t index)
    {
        if (index < m_port_map.size())
            return m_port_map.at(index);
        return NULL;
    }

private:

    fixed_vector<Module *, MAX_MODULES> m_mod_map;
    fixed_vector<Port *, MAX_PORTS> m_port_map;

};

#endif /* !RESOLVER_included */
