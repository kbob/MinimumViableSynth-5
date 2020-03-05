#include "synth/core/c-api.h"

SignalGraph *foo()
{
    g = new SignalGraph()
        ;

    return g;
}

int main()
{
    synth_config cfg = { 2, 1, 1, 1, 1 };
    Synth *s = create_synth(&cfg);
    destroy_synth(s);
    return 0;
}
