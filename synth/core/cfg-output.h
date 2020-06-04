#ifndef CFG_OUTPUT_included
#define CFG_OUTPUT_included

#include "synth/core/config.h"

// These are the standard sample rates, but some platforms will also use
// nonstandard rates.
enum sample_rate {
    SR_44100 = 44100,
    SR_48000 = 48000,
    SR_96000 = 96000,
    SR_192000 = 192000,
};

// Only the standard sample formats are allowed.
enum class sample_format {
    I16,
    I24,
    I32,
    F32,
};

// Only the standard channel configs are allowed.
enum class channel_config {
    MONO = 1,
    STEREO = 2,
    QUAD = 4,
};

struct OutputConfig : public Config::Subsystem {

public:

    OutputConfig(sample_rate sr = SR_44100,
                 sample_format sf = sample_format::F32,
                 channel_config cc = channel_config::MONO)
    : sample_rate{sr},
      sample_format{sf},
      channel_config{cc}
    {}

    const sample_rate sample_rate;
    const sample_format sample_format;
    const channel_config channel_config;

};

#endif /* !CFG_OUTPUT_included */
