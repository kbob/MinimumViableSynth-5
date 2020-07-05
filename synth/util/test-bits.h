#include "bits.h"

#include <cxxtest/TestSuite.h>

class bits_unit_test : public CxxTest::TestSuite {

public:

    unsigned naive_count_bits(std::uintmax_t word)
    {
        unsigned n = 0;
        while (word) {
            word &= word - 1;
            n++;
        }
        return n;
    }

    void test_byte()
    {
        for (unsigned i = 0; i <= 0xFF; i++) {
            std::uint8_t w = i;
            auto expected = naive_count_bits(w);
            auto actual = count_bits(w);
            TS_ASSERT_EQUALS(actual, expected);
        }
    }

    void test_short()
    {
        for (unsigned i = 0; i <= 0xFFFF; i++) {
            std::uint16_t w = i;
            auto expected = naive_count_bits(w);
            auto actual = count_bits(w);
            TS_ASSERT_EQUALS(actual, expected);
        }
    }

    void test_int()
    {
        std::uint32_t w = 3490;
        do {
            auto expected = naive_count_bits(w);
            auto actual = count_bits(w);
            TS_ASSERT_EQUALS(actual, expected);
            w += 8589;
        } while (w);
    }

    void test_64()
    {
        std::uint64_t w = 51616;
        do {
            auto expected = naive_count_bits(w);
            auto actual = count_bits(w);
            TS_ASSERT_EQUALS(actual, expected);
            w += 36893488147419;
        } while (w);
    }

};
