#ifndef CORE_DEFS_included
#define CORE_DEFS_included

#ifdef PLATFORM_DEFS_H
#include PLATFORM_DEFS_H        /* get platform definitions */
#endif

#ifdef TARGET_DEFS_H
#include TARGET_DEFS_H          /* target defs override platform */
#endif

#ifdef PRODUCT_DEFS_H
#include PRODUCT_DEFS_H         /* product definitions override both */
#endif


// -- Defs  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifndef DEFAULT_SAMPLE_TYPE
#define DEFAULT_SAMPLE_TYPE float
#endif

#ifndef SCALE_TYPE
#define SCALE_TYPE float
#endif

#ifndef DEFAULT_SCALE
#define DEFAULT_SCALE 1.0f
#endif

#ifndef NOTE_SHUTDOWN_TIME
#define NOTE_SHUTDOWN_TIME 0.010 // seconds
#endif

#endif /* CORE_DEFS_included */
