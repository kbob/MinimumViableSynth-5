#include "cfg-output.h"

#include <cxxtest/TestSuite.h>

class output_config_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)OutputConfig();
    }

    void test_defaults()
    {
        OutputConfig oc;

        TS_ASSERT_EQUALS(oc.sample_rate, SR_44100);
        TS_ASSERT_EQUALS(oc.sample_format, sample_format::F32);
        TS_ASSERT_EQUALS(oc.channel_config, channel_config::MONO);
    }

    void test_non_defaults()
    {
        OutputConfig oc(SR_96000, sample_format::I24, channel_config::QUAD);

        TS_ASSERT_EQUALS(oc.sample_rate, SR_96000);
        TS_ASSERT_EQUALS(oc.sample_format, sample_format::I24);
        TS_ASSERT_EQUALS(oc.channel_config, channel_config::QUAD);
    }

};
