#ifndef SOUNDSCOPE_included
#define SOUNDSCOPE_included

#include <stdio.h>

#include "synth/core/config.h"
#include "synth/core/modules.h"

class Soundscope : public ModuleType<Soundscope> {

public:

    Soundscope()
    : m_f{NULL}
    {
        in.name("in");
        ports(in);
        m_f = fopen("/tmp/foo", "w");
    }

    ~Soundscope()
    {
        if (m_f) {
            fputs("end\n", m_f);
            fclose(m_f);
        }
    }

    Input<> in;

    void render(size_t frame_count)
    {
        for (size_t i = 0; i < frame_count; i++)
            fprintf(m_f, "%g\n", in[i]);
    }

private:

    FILE *m_f;

};

#endif /* !SOUNDSCOPE_included */
