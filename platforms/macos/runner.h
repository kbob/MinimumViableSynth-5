#ifndef RUNNER_included
#define RUNNER_included

#include <sys/types.h>
#include <sys/sysctl.h>

#include "platforms/macos/soundscope.h"
#include "synth/core/audio-config.h"

template <class Runnable>
class Runner {

public:

    Runner()
    : m_parallel{false},
      m_duration{1.0}
    {}
    Runner(const Runner&) = delete;
    Runner& operator = (const Runner&) = delete;

    Runner& default_duration(float dur) { m_duration = dur; return *this; }

    Runner& invocation(int /*argc*/, char **/*argv*/)
    {
        // XXX insert godawful getopt stuff here.
        // usage:
        //    --duration=SECONDS
        //    --sample-rate=...
        //    --sample-format=...
        //    --channel-config=...
        //    --parallel / --serial
        //    --write-file=WAVFILE
        //    --output='name of audio output'
        //    --list-outputs
        //    --list-all
        return *this;
    }

    int run()
    {
        return m_parallel ? run_parallel() : run_serial();
    }

private:

    int run_serial();

    int run_parallel();

    unsigned calc_thread_count();

    bool m_parallel;
    float m_duration;
    AudioConfig m_config;

};

template <class Runnable>
int
Runner<Runnable>::run_serial()
{
    Soundscope out;
    Runnable runnable(m_config, out);

    int nframes = int(m_duration * m_config.sample_rate);
    for (int chunk_size, i = 0; i < nframes; i += chunk_size) {
        chunk_size = MAX_FRAMES;
        if (chunk_size > nframes - i)
            chunk_size = nframes - i;
        for (auto& t: runnable.synth().timbres())
            t.pre_render(chunk_size);
        for (auto& v: runnable.synth().voices())
            v.render(chunk_size);
        for (auto& t: runnable.synth().timbres())
            t.post_render(chunk_size);
    }
    return 0;
}

template <class Runnable>
int
Runner<Runnable>::run_parallel()
{
    // XXX write me
    return 0;
}

template <class Runnable>
unsigned
Runner<Runnable>::calc_thread_count()
{
    std::int32_t cpu_count = -1;
    size_t cpu_count_size = sizeof cpu_count;
    int x = sysctlbyname("hw.physicalcpu",
                         &cpu_count, &cpu_count_size,
                         NULL, 0);
    if (x) {
        int e = errno;          // save errno before doing anything else.
        std::string msg = "hw.physicalcpu: ";
        msg += std::strerror(e);
        throw std::runtime_error(msg);
    }
    return cpu_count;
}


#endif /* !RUNNER_included */
