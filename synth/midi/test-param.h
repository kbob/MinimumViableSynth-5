#include "param.h"

#include <cxxtest/TestSuite.h>

using midi::ParameterNumber;
using midi::ParameterValue;
using midi::RPN;
using midi::NRPN;

class param_number_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::ParameterNumber();
    }

    void test_default_constructor()
    {
        ParameterNumber n;
        TS_ASSERT(!n.is_valid());
    }

    void test_number_constructor()
    {
        ParameterNumber n(0x1234);
        TS_ASSERT(n.is_valid());
        TS_ASSERT_EQUALS(n.number(), 0x1234);
    }

    void test_RPN_constructor()
    {
        ParameterNumber n(RPN::COARSE_TUNING);
        TS_ASSERT(n.is_valid());
        TS_ASSERT_EQUALS(n.number(),
                         ParameterNumber::number_type(RPN::COARSE_TUNING));
    }

    void test_NRPN_constructor()
    {
        ParameterNumber n(NRPN(0x1234));
        TS_ASSERT(n.is_valid());
        TS_ASSERT_EQUALS(n.number(), 0x1234);
    }

    void test_bytes_constructor()
    {
        ParameterNumber n(0x12, 0x34);
        TS_ASSERT(n.is_valid());
        TS_ASSERT_EQUALS(n.number(), 0x1234);
    }

    void test_bytes()
    {
        ParameterNumber n(0x12, 0x34);
        TS_ASSERT_EQUALS(n.msb(), 0x12);
        TS_ASSERT_EQUALS(n.lsb(), 0x34);
    }

    void test_equality()
    {
        ParameterNumber n0;
        ParameterNumber n1 = 0x1234;
        ParameterNumber n2 = 0x3412;

        TS_ASSERT( (n0 == n0));
        TS_ASSERT(!(n0 == n1));
        TS_ASSERT(!(n0 == n2));

        TS_ASSERT(!(n1 == n0));
        TS_ASSERT( (n1 == n1));
        TS_ASSERT(!(n1 == n2));

        TS_ASSERT(!(n2 == n0));
        TS_ASSERT(!(n2 == n1));
        TS_ASSERT( (n2 == n2));
    }

    void test_less_than()
    {
        ParameterNumber n0;
        ParameterNumber n1 = 0x1234;
        ParameterNumber n2 = 0x3412;

        TS_ASSERT(!(n0 < n0));
        TS_ASSERT(!(n0 < n1));
        TS_ASSERT(!(n0 < n2));

        TS_ASSERT( (n1 < n0));
        TS_ASSERT(!(n1 < n1));
        TS_ASSERT( (n1 < n2));

        TS_ASSERT( (n2 < n0));
        TS_ASSERT(!(n2 < n1));
        TS_ASSERT(!(n2 < n2));
    }

};

class param_value_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::ParameterValue();
    }

    void test_default_constructor()
    {
        ParameterValue v;
        TS_ASSERT(!v.is_valid());
    }

    void test_value_constructor()
    {
        ParameterValue v(1234);
        TS_ASSERT(v.is_valid());
        TS_ASSERT_EQUALS(v.value(), 1234);
    }

    void test_bytes_constructor()
    {
        ParameterValue v(0b1010101, 0b1100110);
        TS_ASSERT(v.is_valid());
        TS_ASSERT_EQUALS(v.value(), 0b10101011100110);
    }

    void test_increment_value()
    {
        const int start = ParameterValue::MAX - 400;
        const int end = ParameterValue::MAX + 10;
        ParameterValue v(start);
        for (int i = start; i < end; i++) {
            int expected = i;
            if (expected > ParameterValue::MAX)
                expected = ParameterValue::MAX;
            TS_ASSERT_EQUALS(v.value(), expected);
            v.increment_value();
        }
    }

    void test_decrement_value()
    {
        const int start = 400;
        const int end   = -10;
        ParameterValue v(start);
        for (int i = start; i > end; --i) {
            int expected = i;
            if (expected < 0)
                expected = 0;
            TS_ASSERT_EQUALS(v.value(), expected);
            v.decrement_value();
        }
    }

    void test_increment_msb()
    {
        const int start = 100;
        const int end   = 140;
        const int lsb   = 123;
        ParameterValue v(start, lsb);
        for (int i = start; i < end; i++) {
            int expected_msb = i;
            int expected_lsb = lsb;
            if (expected_msb > 127) {
                expected_msb = 127;
                expected_lsb = 127;
            }
            TS_ASSERT_EQUALS(v.msb(), expected_msb);
            TS_ASSERT_EQUALS(v.lsb(), expected_lsb);
            v.increment_msb();
        }
    }

    void test_decrement_msb()
    {
        const int start =  20;
        const int end   = -10;
        const int lsb   = 123;
        ParameterValue v(start, lsb);
        for (int i = start; i > end; --i) {
            int expected_msb = i;
            int expected_lsb = lsb;
            if (expected_msb < 0) {
                expected_msb = 0;
                expected_lsb = 0;
            }
            TS_ASSERT_EQUALS(v.msb(), expected_msb);
            TS_ASSERT_EQUALS(v.lsb(), expected_lsb);
            v.decrement_msb();
        }
    }

    void test_increment_centesimally()
    {
        const int start = 12490;
        const int end   = 12820;
        ParameterValue v(start / 100, start % 100);
        for (int i = start; i < end; i++) {
            int expected = i;
            if (expected > 12799)
                expected = 12799;
            // TS_TRACE("expect " + std::to_string(expected));
            TS_ASSERT_EQUALS(v.msb(), expected / 100);
            TS_ASSERT_EQUALS(v.lsb(), expected % 100);
            v.increment_centesimally();
        }
    }

    void test_decrement_centesimally()
    {
        const int start = 200;
        const int end   = -10;
        ParameterValue v(start / 100, start % 100);
        for (int i = start; i > end; --i) {
            int expected = i;
            if (expected < 0)
                expected = 0;
            // TS_TRACE("expect " + std::to_string(expected));
            TS_ASSERT_EQUALS(v.msb(), expected / 100);
            TS_ASSERT_EQUALS(v.lsb(), expected % 100);
            v.decrement_centesimally();
        }
    }

    void test_set_msb()
    {
        ParameterValue v(0x12, 0x34);
        v.set_msb(0x21);
        TS_ASSERT_EQUALS(v.msb(), 0x21);
        TS_ASSERT_EQUALS(v.lsb(), 0x34);

        // setting MSB makes value valid.
        ParameterValue v1;
        TS_ASSERT(!v1.is_valid());
        v1.set_msb(123);
        TS_ASSERT(v1.is_valid());
        TS_ASSERT_EQUALS(v1.value(), 123 << 7);

        // setting MSB does not change LSB.
        ParameterValue v2(45);
        v2.set_msb(67);
        TS_ASSERT_EQUALS(v2.value(), 67 << 7 | 45);
    }

    void test_set_lsb()
    {
        ParameterValue v(0x12, 0x34);
        v.set_lsb(0x56);
        TS_ASSERT_EQUALS(v.msb(), 0x12);
        TS_ASSERT_EQUALS(v.lsb(), 0x56);

        // setting LSB does not make value valid.
        ParameterValue v1;
        TS_ASSERT(!v1.is_valid());
        v1.set_lsb(0x34);
        TS_ASSERT(!v1.is_valid());
        v1.set_msb(0x12);
        TS_ASSERT(v1.is_valid());
        TS_ASSERT_EQUALS(v1.value(), 0x12 << 7 | 0x34);

        // setting LSB does not change MSB.
        ParameterValue v2(12 << 7);
        v2.set_lsb(34);
        TS_ASSERT_EQUALS(v2.value(), 12 << 7 | 34);
    }

};
