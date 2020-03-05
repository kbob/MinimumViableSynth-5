#include "synth/core/c-api.h"
#include "signalgraph.h"

int main()
{
    synth_config cfg = { 2, 1, 1, 1, 1 };
    Synth *s = create_synth(&cfg);
    SignalGraph *g = foo();
    delete g;
    destroy_synth(s);
    return 0;
}
