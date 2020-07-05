#include "layering.h"

#include <cxxtest/TestSuite.h>

using midi::CHANNEL_COUNT;
using midi::Layering;

class layering_unit_test : public CxxTest::TestSuite {

public:

    static const size_t TIMB = MAX_TIMBRALITY - 1;

    void check_inverse(const Layering& l)
    {
        std::array<Layering::channel_mask, MAX_TIMBRES> t2c = {0};
        for (size_t ci = 0, bit = 1; ci < CHANNEL_COUNT; ci++, bit <<= 1) {
            auto c2t = l.channel_timbres(ci);
            for (size_t ti = 0; ti < MAX_TIMBRES; ti++)
                if (c2t & (1 << ti))
                    t2c[ti] |= bit;
                else
                    t2c[ti] &= ~bit;
        }

        for (size_t ti = 0; ti < MAX_TIMBRES; ti++) {
            if (l.timbre_channels(ti) != t2c[ti]) {
                TS_TRACE("fail: ti = " + std::to_string(ti));
                std::cout << std::hex;
                std::cout << "c2t[0] = 0x" << int(l.channel_timbres(0)) << '\n';
                std::cout << "c2t[1] = 0x" << int(l.channel_timbres(1)) << '\n';
                std::cout << "c2t[2] = 0x" << int(l.channel_timbres(2)) << '\n';
                std::cout << "c2t[3] = 0x" << int(l.channel_timbres(3)) << '\n';
                std::cout << "c2t[4] = 0x" << int(l.channel_timbres(4)) << '\n';
                std::cout << "c2t[5] = 0x" << int(l.channel_timbres(5)) << '\n';
                std::cout << "c2t[6] = 0x" << int(l.channel_timbres(6)) << '\n';
                std::cout << "c2t[7] = 0x" << int(l.channel_timbres(7)) << '\n';
                std::cout << '\n';
                std::cout << "t2c[0] = 0x" << int(l.timbre_channels(0)) << '\n';
                std::cout << "t2c[1] = 0x" << int(l.timbre_channels(1)) << '\n';
                std::cout << "t2c[2] = 0x" << int(l.timbre_channels(2)) << '\n';
                std::cout << "t2c[3] = 0x" << int(l.timbre_channels(3)) << '\n';
                std::cout << std::dec;
            }
            TS_ASSERT_EQUALS(l.timbre_channels(ti), t2c[ti]);
            if (ti >= l.timbrality)
                TS_ASSERT_EQUALS(t2c[ti], 0);
        }
    }


    void test_instantiate()
    {
        size_t TIMB = 2;
        (void)midi::Layering(TIMB);
    }

    void test_defaults()
    {
        Layering l(TIMB);
        TS_ASSERT_EQUALS(l.timbrality, TIMB);
        TS_ASSERT_EQUALS(l.all_timbres, (1 << TIMB) - 1);
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            TS_ASSERT_EQUALS(l.channel_timbres(ci), 0x1);
        check_inverse(l);
    }

    void test_omni_mode()
    {
        Layering l(TIMB);
        l.omni_mode();
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            TS_ASSERT_EQUALS(l.channel_timbres(ci), 0x1);
        check_inverse(l);
    }

    void test_poly_modes()
    {
        Layering l(TIMB);
        for (size_t bc = 0; bc < CHANNEL_COUNT; bc++) {
            l.poly_mode(bc);
            for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                TS_ASSERT_EQUALS(l.channel_timbres(ci), ci == bc);
        }
        check_inverse(l);
    }

    void test_mono_modes()
    {
        Layering l(TIMB);
        for (size_t bc = 0; bc < CHANNEL_COUNT; bc++) {
            for (size_t n = 1; n <= CHANNEL_COUNT; n++) {
                std::uint32_t m32 = ((1 << n) - 1) << bc;
                std::uint32_t m = m32 | m32 >> CHANNEL_COUNT;
                l.mono_mode(m);
                for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
                    TS_ASSERT_EQUALS(l.channel_timbres(ci),
                                     (m & 1 << ci) ? 1 : 0);
                check_inverse(l);
            }
        }
    }

    void test_multi_mode()
    {
        Layering l(TIMB);
        l.multi_mode();
        for (size_t ci = 0; ci < CHANNEL_COUNT; ci++)
            TS_ASSERT_EQUALS(l.channel_timbres(ci), ci < TIMB ? 1 << ci : 0);
        check_inverse(l);
    }

    void test_channel_timbres()
    {
        Layering l(TIMB);
        l.multi_mode();
        l.channel_timbres(0, 0x6);
        l.channel_timbres(1, 0x5);
        l.channel_timbres(2, 0x4);
        TS_ASSERT_EQUALS(l.channel_timbres(0), 0x6);
        TS_ASSERT_EQUALS(l.channel_timbres(1), 0x5);
        TS_ASSERT_EQUALS(l.channel_timbres(2), 0x4);
        for (size_t ci = 3; ci < CHANNEL_COUNT; ci++)
            TS_ASSERT_EQUALS(l.channel_timbres(ci), 0x0);
        TS_ASSERT_EQUALS(l.timbre_channels(0), 0x0002);
        TS_ASSERT_EQUALS(l.timbre_channels(1), 0x0001);
        TS_ASSERT_EQUALS(l.timbre_channels(2), 0x0007);
        TS_ASSERT_EQUALS(l.timbre_channels(3), 0x0000);
        check_inverse(l);
    }

    void test_timbre_channels()
    {
        Layering l(TIMB);
        l.multi_mode();
        l.timbre_channels(0, 0xFFFF);
        l.timbre_channels(1, 0x1111);
        l.timbre_channels(2, 0x2222);
        TS_ASSERT_EQUALS(l.timbre_channels(0), 0xFFFF);
        TS_ASSERT_EQUALS(l.timbre_channels(1), 0x1111);
        TS_ASSERT_EQUALS(l.timbre_channels(2), 0x2222);
        TS_ASSERT_EQUALS(l.timbre_channels(3), 0x0000);
        TS_ASSERT_EQUALS(l.channel_timbres(0), 0x3);
        TS_ASSERT_EQUALS(l.channel_timbres(1), 0x5);
        TS_ASSERT_EQUALS(l.channel_timbres(2), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(3), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(4), 0x3);
        TS_ASSERT_EQUALS(l.channel_timbres(5), 0x5);
        TS_ASSERT_EQUALS(l.channel_timbres(6), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(7), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(8), 0x3);
        TS_ASSERT_EQUALS(l.channel_timbres(9), 0x5);
        TS_ASSERT_EQUALS(l.channel_timbres(10), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(11), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(12), 0x3);
        TS_ASSERT_EQUALS(l.channel_timbres(13), 0x5);
        TS_ASSERT_EQUALS(l.channel_timbres(14), 0x1);
        TS_ASSERT_EQUALS(l.channel_timbres(15), 0x1);
        check_inverse(l);
    }

};
