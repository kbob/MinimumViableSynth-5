#ifndef SYNTH_CORE_included
#define SYNTH_CORE_included

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct synth_config {
        unsigned voice_count;
        uint8_t  global_lfo_count;
        uint8_t  osc_per_voice;
        uint8_t  lfo_per_voice;
        uint8_t  env_per_voice;
    } synth_config;

    typedef struct Synth Synth;

    Synth *create_synth(const synth_config *);
    void destroy_synth(Synth *);

#ifdef __cplusplus
};
#endif

#endif /* !SYNTH_CORE_included */
