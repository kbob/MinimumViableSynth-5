#ifndef PLANNER_included
#define PLANNER_included

#include "synth/core/config.h"
#include "synth/core/links.h"
#include "synth/core/modules.h"
#include "synth/core/mod-vector.h"
#include "synth/core/plan.h"
#include "synth/core/ported.h"
#include "synth/core/voice.h"
#include "synth/util/bijection.h"
#include "synth/util/noalloc.h"
#include "synth/util/relation.h"
#include "synth/util/universe.h"

// A Planner creates a Plan from a set of controls, modules, links,
// and control values.

// XXX should use control values.

class Planner {

public:

    typedef fixed_vector<Control *, MAX_TIMBRE_CONTROLS> tc_vec;
    typedef fixed_vector<Module *, MAX_TIMBRE_MODULES>   tm_vec;
    typedef fixed_vector<Control *, MAX_VOICE_CONTROLS>  vc_vec;
    typedef fixed_vector<Module *, MAX_VOICE_MODULES>    vm_vec;
    typedef fixed_vector<Link *, MAX_LINKS>            link_vec;
    typedef fixed_vector<Module *, MAX_OUTPUT_MODULES>   om_vec;

    // Just get it over with: pass everything in through the constructor.
    Planner(const tc_vec& tcontrols,
            const tm_vec& tmodules,
            const vc_vec& vcontrols,
            const vm_vec& vmodules,
            link_vec& links,
            // const cv_vec& cvalues,
            const om_vec& omodules
           )
    : m_tcontrols{tcontrols},
      m_tmodules{tmodules},
      m_vcontrols{vcontrols},
      m_vmodules{vmodules},
      m_links{links},
      // m_cvalues{cvalues},
      m_omodules{omodules}
    {
        // N.B., must add timbre objects before voice objects.
        m_resolver.add_controls(tcontrols.begin(), tcontrols.end())
                  .add_modules(tmodules.begin(), tmodules.end())
                  .add_controls(vcontrols.begin(), vcontrols.end())
                  .add_modules(vmodules.begin(), vmodules.end())
                  .finalize();
    }

    Plan make_plan();

private:

    typedef Resolver::controls_type control_verse;      // 'verse is short
    typedef Resolver::modules_type module_verse;        // for Universe.
    typedef Resolver::ports_type port_verse;
    typedef Universe<link_vec, MAX_LINKS> link_verse;

    typedef control_verse::subset_type control_subset;
    typedef module_verse::subset_type module_subset;
    typedef port_verse::subset_type port_subset;
    typedef link_verse::subset_type link_subset;

    typedef Plan::prep_step_sequence prep_steps;
    typedef Plan::render_step_sequence render_steps;

    typedef Relation<module_verse, module_verse> mod_pred_type;
    typedef Relation<port_verse, link_verse> link_rel_type;
    typedef std::back_insert_iterator<prep_steps> prep_appender;
    typedef std::back_insert_iterator<render_steps> render_appender;

    // How can I ever stop giggling when I have types called
    // prep_steps and render_appender?

    struct mod_partition {
        module_subset pre;
        module_subset voice;
        module_subset post;
    };

    struct fcu_result {
        control_subset timbre;
        control_subset voice;
    };

    // Private methods.  These are all used by `make_plan()`.
    mod_partition
    partition_modules_used();

    fcu_result
    find_controls_used(const module_subset&);

    void
    calc_mod_predecessors();

    void
    calc_links_to();

    module_subset
    collect_pred(module_subset succ, module_subset candidates);

    void
    assemble_prep_steps(const module_subset&, prep_appender&);

    void
    assemble_render_steps(const control_subset&,
                          module_subset section,
                          module_subset done,
                          render_appender&);

    const tc_vec& m_tcontrols;
    const tm_vec& m_tmodules;
    const vc_vec& m_vcontrols;
    const vm_vec& m_vmodules;
    const Universe<link_vec, MAX_LINKS> m_links;
    const om_vec& m_omodules;
    Resolver m_resolver;
    std::unique_ptr<mod_pred_type> m_mod_predecessors;
    std::unique_ptr<link_rel_type> m_links_to;

};

#endif /* !PLANNER_included */
