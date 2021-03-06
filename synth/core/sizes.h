#ifndef CORE_SIZES_included
#define CORE_SIZES_included

#ifdef PLATFORM_SIZES_H
#include PLATFORM_SIZES_H       /* get platform limits and defaults */
#endif

#ifdef TARGET_SIZES_H
#include TARGET_SIZES_H         /* target sizes override platform */
#endif

#ifdef PRODUCT_SIZES_H
#include PRODUCT_SIZES_H        /* product sizes override both */
#endif


// -- Sizes -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// The default maxima below are just good enough for the unit tests.
// Most applications will need to override most of them.

#ifndef MAX_POLYPHONY
#define MAX_POLYPHONY 4
#endif

#define MAX_VOICES MAX_POLYPHONY

#ifndef MAX_TIMBRALITY
#define MAX_TIMBRALITY 4
#endif

#define MAX_TIMBRES MAX_TIMBRALITY

#ifndef MAX_FRAMES
#define MAX_FRAMES 4
#endif

#ifndef MAX_SUBSYSTEMS
#define MAX_SUBSYSTEMS 4
#endif

#ifndef MAX_TIMBRE_CONTROLS
#define MAX_TIMBRE_CONTROLS 4
#endif

#ifndef MAX_VOICE_CONTROLS
#define MAX_VOICE_CONTROLS 4
#endif

#ifndef MAX_TIMBRE_MODULES
#define MAX_TIMBRE_MODULES 4
#endif

#ifndef MAX_VOICE_MODULES
#define MAX_VOICE_MODULES 4
#endif

#ifndef MAX_VOICE_LIFE_CONTROLS
#define MAX_VOICE_LIFE_CONTROLS 4
#endif

#ifndef MAX_VOICE_LIFE_MODULES
#define MAX_VOICE_LIFE_MODULES 4
#endif

#ifndef MAX_OUTPUT_MODULES
#define MAX_OUTPUT_MODULES 4
#endif

#ifndef MODULE_MAX_PORTS
#define MODULE_MAX_PORTS 4
#endif

#ifndef MAX_CONTROLS
#define MAX_CONTROLS (MAX_TIMBRE_CONTROLS + MAX_VOICE_CONTROLS)
#endif

#ifndef MAX_MODULES
#define MAX_MODULES (MAX_TIMBRE_MODULES + MAX_VOICE_MODULES)
#endif

#ifndef MAX_PORTS
// pessimistic
#define MAX_PORTS (MAX_CONTROLS + MODULE_MAX_PORTS * MAX_MODULES)
#endif

#ifndef MAX_LINKS
// pessimistic
#define MAX_LINKS (MAX_CONTROLS + MAX_PORTS)
#endif

#ifndef MAX_PREP_STEPS
#define MAX_PREP_STEPS (MAX_CONTROLS + MAX_LINKS)
#endif

#ifndef MAX_RENDER_STEPS
#define MAX_RENDER_STEPS (MAX_CONTROLS + MAX_LINKS + MAX_MODULES)
#endif

#ifndef MAX_RENDER_ACTIONS
#define MAX_RENDER_ACTIONS MAX_RENDER_STEPS
#endif

#endif /* CORE_SIZES_included */
