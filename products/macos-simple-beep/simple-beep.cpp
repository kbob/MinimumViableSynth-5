#include "targets/simple-beep/simple-beep.h"
#include "platforms/macos/runner.h"

int main(int argc, char *argv[])
{
    Runner<SimpleBeep> r;
    return r.default_duration(0.1)
            .invocation(argc, argv)
            .run();
}
