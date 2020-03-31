#include "synth/core/mod-network.h"

#include "synth/core/actions.h"

class ModNetwork::port_map {

public:

    Port *at(size_t index) const
    {
        return m_ports[index];
    }

    ssize_t at(Port *port) const
    {
        for (size_t i = 0; i < m_ports.size(); i++)
            if (m_ports.at(i) == port)
                return i;
        return -1;
    }

    void push_back(Port *p)
    {
        m_ports.push_back(p);
    }

private:

    fixed_vector<Port *, ModNetwork::MAX_PORTS> m_ports;

};

Plan ModNetwork::make_plan() const
{
    // map port index to port pointer
    port_map ports;
    init_ports(ports);

    module_adjacency_matrix mod_predecessors;
    init_mod_predecessors(mod_predecessors);

    port_adjacency_matrix port_sources;
    init_port_sources(ports, port_sources);

    Plan plan;

    // Load necessary prep work into `plan.prep`.
    // Unconnected ports are cleared; simply-connected
    // ports are aliased so there is no data copying.
    //
    //     for mod in modules:
    //         for dest in mod inputs:
    //             if no links to dest: gen Clear action
    //             if one simple link to dest: gen Alias action
    //
    for (const auto *mod: m_modules) {
        for (auto *port: mod->ports()) {
            InputPort *dest = dynamic_cast<InputPort *>(port);
            if (!dest)
                continue;
            size_t link_count = 0;
            const Link *a_link = nullptr;
            const SimpleLink *s_link = nullptr;
            for (const auto& link : m_simple_links) {
                if (link.key().dest() == dest) {
                    link_count++;
                    a_link = &link;
                }
            }
            for (const auto& link : m_simple_links) {
                if (link.key().dest() == dest) {
                    link_count++;
                    a_link = &link;
                    s_link = &link;
                }
            }
            for (const auto *link : m_control_links) {
                if (link->key().dest() == dest) {
                    link_count++;
                    a_link = link;
                }
            }
            if (link_count == 0) {
                plan.push_back_prep(ClearAction(ports.at(dest)));
            } else if (link_count == 1 && s_link) {
                size_t src_index = ports.at(a_link->key().src());
                size_t dest_index = ports.at(a_link->key().dest());
                plan.push_back_prep(AliasAction(src_index, dest_index));
            }
        }
    }

    return plan;
}

void ModNetwork::init_ports(port_map& ports) const
{
    for (const auto *m: m_modules) {
        for (auto *p: m->ports()) {
            ports.push_back(p);
        }
    }
}

void
ModNetwork::init_mod_predecessors(module_adjacency_matrix& mod_predecessors)
const
{
    mod_predecessors.fill(0);
    for (const auto& link: m_simple_links) {
        auto key = link.key();
        auto src = key.src();
        auto dest = key.dest();
        auto pred_index = module_index(&src->module());
        auto succ_index = module_index(&dest->module());
        // std::cout << "\n\n";
        // std::cout << "src = " << port_name(*src) << "\n";
        // std::cout << "src module = " << module_name(src->module()) << "\n";
        // std::cout << "\n\n" << std::endl;
        assert(pred_index != -1 && succ_index != -1);
        mod_predecessors[succ_index] |= 1 << pred_index;
    }
    for (const auto *link: m_control_links) {
        auto key = link->key();
        auto src = key.src();
        auto dest = key.dest();
        auto pred_index = module_index(&src->module());
        auto succ_index = module_index(&dest->module());
        assert(pred_index != -1 && succ_index != -1);
        mod_predecessors[succ_index] |= 1 << pred_index;
    }
}

void ModNetwork::init_port_sources(const port_map& ports,
                                   port_adjacency_matrix& port_sources) const
{
    port_sources.fill(0);
    for (const auto& link: m_simple_links) {
        auto key = link.key();
        auto src = key.src();
        auto dest = key.dest();
        auto src_index = ports.at(src);
        auto dest_index = ports.at(dest);
        assert(src_index != -1 && dest_index != -1);
        port_sources[dest_index] |= 1 << src_index;
    }
}

ssize_t ModNetwork::module_index(const Module *mod) const
{
    for (size_t i = 0; i < m_modules.size(); i++)
        if (m_modules.at(i) == mod)
            return i;
    return -1;
}
