#include "param.h"

#include <cxxtest/TestSuite.h>

using midi::ParameterNumber;
using midi::RPN;
using midi::NRPN;

class param_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::ParameterNumber();
        (void)midi::ParameterValue();
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
