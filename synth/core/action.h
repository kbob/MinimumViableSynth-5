#ifndef ACTION_included
#define ACTION_included

#include <functional>

#include "synth/core/config.h"
#include "synth/util/noalloc.h"

typedef std::function<void(size_t)> render_action;
typedef fixed_vector<render_action, MAX_RENDER_ACTIONS> render_action_sequence;

#endif /* !ACTION_included */
