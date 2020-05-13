#include "audio-config.h"

#include <cxxtest/TestSuite.h>

class audio_config_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)AudioConfig();
    }

    void test_sample_rate()
    {
        AudioConfig ac;
        TS_ASSERT_EQUALS(ac.sample_rate, 44100);

        ac.sample_rate = SR_44100;
        TS_ASSERT_EQUALS(ac.sample_rate, 44100);

        ac.sample_rate = SR_48000;
        TS_ASSERT_EQUALS(ac.sample_rate, 48000);

        ac.sample_rate = SR_96000;
        TS_ASSERT_EQUALS(ac.sample_rate, 96000);

        ac.sample_rate = SR_192000;
        TS_ASSERT_EQUALS(ac.sample_rate, 192000);

        ac.sample_rate = static_cast<sample_rate>(46875);
        TS_ASSERT_EQUALS(ac.sample_rate, 46875);
    }

};
