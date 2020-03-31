#include "synth/core/mod-network.h"

#include "synth/core/actions.h"

Plan ModNetwork::make_plan() const
{
    size_t n_modules = m_modules.size();

    auto ports = m_modules.ports();

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
            const SimpleLink *s_link = nullptr;
            for (const auto& link : m_simple_links) {
                if (link.key().dest() == dest) {
                    link_count++;
                    s_link = &link;
                }
            }
            for (const auto *link : m_control_links) {
                if (link->key().dest() == dest) {
                    link_count++;
                }
            }
            if (link_count == 0) {
                plan.push_back_prep(ClearAction(ports.index(dest)));
            } else if (link_count == 1 && s_link) {
                size_t src_index = ports.index(s_link->key().src());
                size_t dest_index = ports.index(s_link->key().dest());
                plan.push_back_prep(AliasAction(src_index, dest_index));
            }
        }
    }


    // Generate the rendering actions.
    // Modules are rendered after the modules they depend on,
    // and after their inputs are computed.
    //
    //     done = {}
    //     while done != {all}:
    //         ready = {modules whose predecessors are done}
    //         create actions for ready modules
    //         done |= ready
    //
    module_mask done_mask = 0;
    const module_mask all_done_mask = (1 << n_modules) - 1;
    while (done_mask != all_done_mask) {

        // Collect all modules ready to process.
        module_mask ready_mask = 0;
        for (size_t i = 0; i < n_modules; i++) {
            if ((mod_predecessors[i] & ~done_mask) == 0) {
                ready_mask |= 1 << i;
            }
        }
        ready_mask &= ~done_mask;
        assert(ready_mask && "cycle in signal graph");

        // for mod in ready modules:
        //     for dest in mod inputs:
        //         for link in links to dest:
        //             copy first link; add other links
        //     render mod
        for (size_t i = 0; i < n_modules; i++) {
            if (!(ready_mask & 1 << i))
                continue;
            const Module *mod = m_modules.at(i);
            for (auto *port: mod->ports()) {
                InputPort *dest = dynamic_cast<InputPort *>(port);
                if (!dest)
                    continue;   // not an input port

                // The first source is copied to the destination;
                // the rest are added to it.
                bool copied = false;
                auto copy_or_add = [&] (size_t si, size_t di, ControlLink *lk) {
                    RunAction act;
                    if (!copied) {
                        act = CopyAction(si, di, lk);
                        copied = true;
                    } else {
                        act = AddAction(si, di, lk);
                    }
                    plan.push_back_run(act);
                };

                for (const auto& link: m_simple_links) {
                    if (link.key().dest() != dest)
                        continue;
                    ssize_t src_index = ports.index(link.key().src());
                    ssize_t dest_index = ports.index(link.key().dest());
                    if (port_sources[dest_index] == 1 << src_index) {
                        // This port is simply-connected.  Do not
                        // emit actions for it.
                        break;
                    }
                    copy_or_add(src_index, dest_index, nullptr);
                }
                for (auto *link: m_control_links) {
                    if (link->key().dest() != dest)
                        continue;
                    ssize_t src_index = ports.index(link->key().src());
                    ssize_t dest_index = ports.index(link->key().dest());
                    copy_or_add(src_index, dest_index, link);
                }
            }
            plan.push_back_run(RenderAction(i));
        }

        done_mask |= ready_mask;
    }

    return plan;
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
        auto pred_index = m_modules.index(&src->module());
        auto succ_index = m_modules.index(&dest->module());
        mod_predecessors[succ_index] |= 1 << pred_index;
    }
    for (const auto *link: m_control_links) {
        auto key = link->key();
        auto src = key.src();
        auto dest = key.dest();
        auto pred_index = m_modules.index(&src->module());
        auto succ_index = m_modules.index(&dest->module());
        mod_predecessors[succ_index] |= 1 << pred_index;
    }
}

void ModNetwork::init_port_sources(const port_vector& ports,
                                   port_adjacency_matrix& port_sources) const
{
    port_sources.fill(0);
    for (const auto& link: m_simple_links) {
        auto key = link.key();
        auto src = key.src();
        auto dest = key.dest();
        auto src_index = ports.index(src);
        auto dest_index = ports.index(dest);
        port_sources[dest_index] |= 1 << src_index;
    }
}
