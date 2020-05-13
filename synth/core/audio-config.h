#ifndef AUDIO_CONFIG_included
#define AUDIO_CONFIG_included

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

class AudioConfig {

public:

    AudioConfig(sample_rate sr = SR_44100,
                sample_format sf = sample_format::F32,
                channel_config cc = channel_config::MONO)
    : sample_rate{sr},
      output_sample_format{sf},
      output_channel_config{cc}
    {}

    enum sample_rate sample_rate;
    enum sample_format output_sample_format;
    enum channel_config output_channel_config;

};

#endif /* !AUDIO_CONFIG_include */
