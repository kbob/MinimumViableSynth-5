#ifndef TIMBRE_included
#define TIMBRE_included

#include "synth/util/noalloc.h"

class Control;
class Module;
class Patch;
class Plan;

// A Timbre has a patch reference, a "factory default" patch,
// a plan, per-timbre controls, per-timbre modules, and pre-
// and post-voice render actions.

class Timbre {
public:

    static const size_t MAX_TIMBRE_CONTROLS = 4;
    static const size_t MAX_TIMBRE_MODULES = 4;

    Timbre() = default;

// private:
    Patch *m_current_patch;
    Patch *m_default_patch;
    Plan *m_plan;
    fixed_vector<Control *, MAX_TIMBRE_CONTROLS> m_controls;
    fixed_vector<Module *, MAX_TIMBRE_MODULES> m_modules;
    // fixed_vector<RenderAction, MAX_RENDER_ACTION> m_pre_actions;
    // fixed_vector<RenderAction, MAX_RENDER_ACTION> m_post_actions;

};

#endif /* !TIMBRE_included */
