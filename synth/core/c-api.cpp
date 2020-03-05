#include "synth/core/c-api.h"

extern "C"
Synth *create_synth(synth_config *cfg)
{
    return new Synth(cfg);
}

extern "C"
void destroy_synth(Synth *s)
{
    delete s;
}
