#include "synth/core/planner.h"

#include <array>

#include "synth/core/steps.h"


// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>
#include <string>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
static std::string demangle(const std::string& mangled)
{
    int status;
    char *s = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
    std::string demangled(s);
    std::free(s);
    return demangled;
}

template <typename T>
static std::string type_name(T& obj)
{
    const std::string& mangled = typeid(*&obj).name();
    return demangle(mangled);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Plan
Planner::make_plan()
{
    // Partition reachable modules into pre, voice, and post;
    // define other useful module subsets.
    auto mod_parts = partition_modules_used();
    auto timbre_mods = mod_parts.pre | mod_parts.post;
    auto voice_mods = mod_parts.voice;
    auto mods_used = timbre_mods | voice_mods;
    auto no_mods = m_resolver.modules().none;

    // Find reachable controls.
    auto controls_used = find_controls_used(mods_used);
    auto no_controls = m_resolver.controls().none;

    // Build a plan.
    Plan plan;

    // Assemble timbre prep steps.
    auto t_prep_appender = std::back_inserter(plan.t_prep());
    assemble_prep_steps(timbre_mods, t_prep_appender);

    // Assemble voice prep steps.
    auto v_prep_appender = std::back_inserter(plan.v_prep());
    assemble_prep_steps(voice_mods, v_prep_appender);

    // Assemble pre-voice render steps.
    auto pre_render_appender = std::back_inserter(plan.pre_render());
    assemble_render_steps(controls_used.timbre,
                          mod_parts.pre,
                          no_mods,
                          pre_render_appender);

    // Assemble voice render steps.
    auto v_render_appender = std::back_inserter(plan.v_render());
    assemble_render_steps(controls_used.voice,
                          voice_mods,
                          mod_parts.pre,
                          v_render_appender);

    // Assemble post-voice render steps.
    auto post_render_appender = std::back_inserter(plan.post_render());
    assemble_render_steps(no_controls,
                          mod_parts.post,
                          mod_parts.pre | mod_parts.voice,
                          post_render_appender);

    return plan;
}

// partition_modules_used - partition reachable (used) modules into
//    pre_mods  - timbre modules used before voice mods
//    v_mods    - voice modules that are used
//    post_mods - timbre moduls used after voice mods
//
// Unreachable modules are discarded.
Planner::mod_partition
Planner::partition_modules_used()
{
    auto& mod_u = m_resolver.modules();
    auto outputs_used = mod_u.subset(m_omodules.begin(), m_omodules.end());
    auto all_tmods = mod_u.subset(m_tmodules.begin(), m_tmodules.end());
    auto all_vmods = mod_u.subset(m_vmodules.begin(), m_vmodules.end());

    auto post_mods = outputs_used | collect_pred(outputs_used, all_tmods);
    auto voice_mods = collect_pred(post_mods, all_vmods);
    auto pre_mods = collect_pred(voice_mods, all_tmods);
    assert(voice_mods <= all_vmods);
    assert((pre_mods & post_mods) == 0);
    assert((pre_mods | post_mods) <= all_tmods);
    return mod_partition{pre_mods, voice_mods, post_mods};
}

Planner::fcu_result
Planner::find_controls_used(const module_subset& modules)
{
    auto tcontrols_used = m_resolver.controls().none;
    auto vcontrols_used = m_resolver.controls().none;
    for (auto link: m_links.all.members()) {
        Module *m = static_cast<Module *>(link->dest()->owner());
        if (!modules.contains(m))
            continue;
        if (link->ctl()) {
            auto owner = link->ctl()->owner();
            if (auto ctl = dynamic_cast<Control *>(owner)) {
                if (std::find(m_tcontrols.begin(),
                              m_tcontrols.end(),
                              ctl) != m_tcontrols.end()) {
                    tcontrols_used.add(ctl);
                }
                if (std::find(m_vcontrols.begin(),
                              m_vcontrols.end(),
                              ctl) != m_vcontrols.end()) {
                    vcontrols_used.add(ctl);
                }
            }
        }
    }
    return fcu_result{tcontrols_used, vcontrols_used};
}

void
Planner::assemble_prep_steps(const module_subset& modules,
                             prep_appender& add_step)
{
    auto port_u = m_resolver.ports();

    for (auto m: modules.members()) {
        for (auto p: m->ports()) {
            if (!dynamic_cast<InputPort *>(p))
                continue;
            int link_count = 0;
            Link *s_link = nullptr;
            std::cout << "links_to type = "
                      << type_name(m_links_to)
                      << std::endl;
            m_links_to->get(p);
            for (auto link: m_links_to->get(p).members()) {
                link_count++;
                if (link->is_simple()) {
                    Module *m = static_cast<Module *>(link->src()->owner());
                    if (modules.contains(m))
                        s_link = link;
                }
            }
            auto di = port_u.index(p);
            if (link_count == 0) {
                // unconnected input: clear buffer.
                add_step = ClearStep(di, 0);
            }
            else if (link_count == 1 && s_link) {
                // simply connected input: alias it to its source.
                auto si = port_u.index(s_link->src());
                add_step = AliasStep(di, si);
            }
            else {
                // complex connection: remove any existing alias.
                add_step = AliasStep(di, -1);
            }
        }
    }
}

void
Planner::assemble_render_steps(const control_subset& controls,
                               module_subset section,
                               module_subset done,
                               render_appender& add_step)
{
    auto mod_u = m_resolver.modules();
    auto port_u = m_resolver.ports();

    for (auto c: controls.indices())
        add_step = ControlRenderStep(c);
    while ((section - done) != 0) {
        auto ready = mod_u.none;
        for (auto mi: section.indices()) {
            Module *m = mod_u[mi];
            if (done.contains(m))
                continue;
            auto mod_preds = m_mod_predecessors->at(mi);
            if (mod_preds <= done)
                ready.set(mi);
        }
        if (ready == 0)
            throw std::runtime_error("can't compute graph");
        for (auto mi: ready.indices()) {
            Module *m = mod_u[mi];
            for (auto dest: m->ports()) {
                if (!dynamic_cast<InputPort *>(dest))
                    continue;
                auto di = port_u.index(dest);
                bool copied = false;
                auto links_to_dest = m_links_to->at(di);
                for (auto link: links_to_dest.members()) {
                    auto si = port_u.find(link->src());
                    if (link->is_simple() && links_to_dest.size() == 1) {
                        // skip single simple links
                        break;
                    }
                    auto ci = port_u.find(link->ctl());
                    if (!copied) {
                        add_step = CopyStep(di, si, ci, link);
                        copied = true;
                    } else
                        add_step = AddStep(di, si, ci, link);
                }
            }
            add_step = ModuleRenderStep(mi);
        }
        done |= ready;
    }
}


Planner::module_subset
Planner::collect_pred(module_subset succ, module_subset candidates)
{
    auto& mod_u = m_resolver.modules();
    auto pred = mod_u.none;
    auto cur = succ;
    while (true) {
        auto prev = mod_u.none;
        for (auto mi: cur.indices())
            prev |= m_mod_predecessors->at(mi);
        prev &= candidates;
        if (prev == 0)
            break;
        pred |= prev;
        cur = prev;
    }
    return pred;
}

void
Planner::calc_mod_predecessors()
{
    for (auto link: m_links.all.members()) {
        Module *dest_mod = static_cast<Module *>(link->dest()->owner());
        if (link->src()) {
            Module *src_mod = static_cast<Module *>(link->src()->owner());
            m_mod_predecessors->add(dest_mod, src_mod);
        }
        if (link->ctl()) {
            Module *ctl_mod = dynamic_cast<Module *>(link->ctl()->owner());
            if (ctl_mod)
            m_mod_predecessors->add(dest_mod, ctl_mod);
        }
    }
}

void
Planner::calc_links_to()
{
    for (auto link: m_links.all.members())
        m_links_to->add(link->dest(), link);
}
